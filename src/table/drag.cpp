/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2020-2021 Tomi Leppänen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QGuiApplication>
#include <QScreen>
#include <QStyleHints>
#include "card.h"
#include "constants.h"
#include "drag.h"
#include "feedbackevent.h"
#include "logging.h"
#include "slot.h"
#include "table.h"

quint32 Drag::s_count = 0;

QElapsedTimer Drag::s_doubleClickTimer;

const Card *Drag::s_lastCard = nullptr;

Drag::Drag(QMouseEvent *event, Table *table, Slot *slot, Card *card)
    : QQuickItem(card)
    , m_state(NoDrag)
    , m_id(nextId())
    , m_mayBeADoubleClick(false)
    , m_table(table)
    , m_card(card)
    , m_source(slot)
    , m_target(-1)
{
    setParentItem(table);
    setX(slot->x());
    setY(slot->y());

    auto engine = Engine::instance();
    connect(this, &Drag::doDrag, engine, &Engine::drag);
    connect(this, &Drag::doCancelDrag, engine, &Engine::cancelDrag);
    connect(this, &Drag::doCheckDrop, engine, &Engine::checkDrop);
    connect(this, &Drag::doDrop, engine, &Engine::drop);
    connect(this, &Drag::doClick, engine, &Engine::click);
    connect(this, &Drag::doDoubleClick, engine, &Engine::doubleClick);
    connect(engine, &Engine::couldDrag, this, &Drag::handleCouldDrag);
    connect(engine, &Engine::couldDrop, this, &Drag::handleCouldDrop);
    connect(engine, &Engine::dropped, this, &Drag::handleDropped);
    connect(engine, &Engine::clicked, this, &Drag::handleClicked);
    connect(engine, &Engine::doubleClicked, this, &Drag::handleDoubleClicked);

    m_mayBeADoubleClick = couldBeDoubleClick(card);
    m_startPoint = m_lastPoint = card->mapToItem(m_table, event->pos());
    m_timer.start();

    qCDebug(lcDrag) << "Started drag of" << card << "for" << slot;
}

Drag::~Drag()
{
    if (m_state < Finished)
        qCWarning(lcDrag) << "Drag was not finished or canceled when it was destroyed";
}

Card *Drag::card() const
{
    return m_card;
}

Slot *Drag::source() const
{
    return m_source;
}

void Drag::update(QMouseEvent *event)
{
    if (mayBeAClick(event))
        return;

    switch (m_state) {
    case AboutToDrag:
        m_state = StartingDrag;
        emit doDrag(m_id, m_source->id(), m_source->asCardData(m_card));
        break;
    case Dragging: {
        QPointF point = m_card->mapToItem(m_table, event->pos());
        QPointF move = point - m_lastPoint;
        setX(x() + move.x());
        setY(y() + move.y());
        m_lastPoint = point;
        checkTargets();
        break;
    }
    default:
        qCCritical(lcDrag) << "Invalid state in update:" << m_state;
        [[fallthrough]];
    case StartingDrag:
    case Canceled:
        break;
    }
}

void Drag::finish(QMouseEvent *event)
{
    if (mayBeAClick(event)) {
        m_state = Clicked;
        if (m_mayBeADoubleClick) {
            qCDebug(lcDrag) << "Detected double click on" << m_card;
            emit doDoubleClick(m_id, m_source->id());
        } else {
            qCDebug(lcDrag) << "Detected click on" << m_card;
            emit doClick(m_id, m_source->id());
        }
        return;
    }

    switch (m_state) {
    case Dragging:
        m_state = Dropping;
        checkTargets(true);
        break;
    case Canceled:
        done();
        break;
    default:
        cancel();
        break;
    }
}

void Drag::checkTargets(bool force)
{
    auto targets = m_table->getSlotsFor(m_card, m_source);
    if (force || m_targets != targets) {
        m_target = -1;
        m_targets = targets;
        highlightOrDrop();
    }
}

void Drag::highlightOrDrop()
{
    if (m_state > Dropping)
        return;

    m_target++;
    if (m_target < m_targets.count()) {
        Slot *target = m_targets.at(m_target);
        switch (m_couldDrop.value(target->id(), Unknown)) {
        case Unknown:
            // Cache miss, check
            m_couldDrop.insert(target->id(), Checking);
            qCDebug(lcDrag) << "Testing move from" << m_source << "to" << target;
            emit doCheckDrop(m_id, m_source->id(), target->id(), toCardData(m_cards, m_state));
            [[fallthrough]];
        case Checking: // Signal for the same slot received twice, the correct signal should still arrive
            m_target--; // Check the slot again later
            break;
        case CanDrop:
            // Found target to highlight or drop to
            if (m_state < Dropping) {
                qCDebug(lcDrag) << "Highlighting" << target;
                m_table->highlight(target);
            } else {
                m_table->highlight(nullptr);
                drop(target);
            }
            break;
        case CantDrop:
            // Recurse to next iteration
            highlightOrDrop();
            break;
        }
    } else {
        // No suitable target, remove highlights or cancel
        if (m_state < Dropping) {
            m_table->highlight(nullptr);
        } else {
            emit m_table->feedback()->dropFailed();
            cancel();
        }
    }
}

void Drag::drop(Slot *slot)
{
    qCDebug(lcDrag) << "Moving from" << m_source << "to" << slot;
    m_state = Dropped;
    emit doDrop(m_id, m_source->id(), slot->id(), toCardData(m_cards, m_state));
}

void Drag::cancel()
{
    qCDebug(lcDrag) << "Canceling drag of" << m_card << "at state" << m_state;

    if (m_state < Dropped) {
        if (m_state >= Dragging) { 
            emit doCancelDrag(m_id, m_source->id(), toCardData(m_cards, Canceled));
            setParentItem(nullptr);
            m_source->put(m_cards);
            m_cards.clear();
        }

        if (m_state == Dropping || m_state == AboutToDrag)
            done();
        m_state = Canceled;
    }
}

void Drag::handleCouldDrag(quint32 id, int slotId, bool could)
{
    if (id != m_id && slotId != m_source->id())
        return;

    if (m_state == StartingDrag && could) {
        m_state = Dragging;
        m_cards = m_source->take(m_card);
        if (m_cards.empty())
            qCCritical(lcDrag) << "Tried to drag empty stack of cards from slot" << m_source
                               << "starting from" << m_card;
        for (Card *card : m_cards) {
            if (card)
                card->moveTo(this);
            else
                qCCritical(lcDrag) << "Tried to drag non-existing card";
        }

        checkTargets();
    } else if (m_state == Canceled) {
        emit doCancelDrag(m_id, m_source->id(), m_source->asCardData(m_card));
        done();
    } else {
        cancel();
    }
}

void Drag::handleCouldDrop(quint32 id, int slotId, bool could)
{
    if (id != m_id)
        return;

    m_couldDrop.insert(slotId, could ? CanDrop : CantDrop);
    highlightOrDrop();
}

void Drag::handleDropped(quint32 id, int slotId, bool could)
{
    Q_UNUSED(slotId)

    if (id != m_id)
        return;

    if (m_state >= Finished) {
        qCWarning(lcDrag) << this << "has already handled dropping";
        return;
    }

    if (could) {
        emit m_table->feedback()->dropSucceeded();
        m_state = Finished;
        m_table->store(m_cards);
        m_cards.clear();
        done();
    } else {
        emit m_table->feedback()->dropFailed();
        cancel();
    }
}

void Drag::handleClicked(quint32 id, int slotId, bool could)
{
    Q_UNUSED(slotId)

    if (id != m_id)
        return;

    if (could) {
        emit m_table->feedback()->clicked();
        s_doubleClickTimer.invalidate();
    }

    done();
}

void Drag::handleDoubleClicked(quint32 id, int slotId, bool could)
{
    Q_UNUSED(slotId)

    if (id != m_id)
        return;

    if (could)
        emit m_table->feedback()->clicked();

    done();
}

void Drag::done()
{
    emit finished();
    deleteLater();
}

bool Drag::mayBeAClick(QMouseEvent *event)
{
    if (m_state > NoDrag)
        return false;

    auto styleHints = QGuiApplication::styleHints();
    if (m_timer.hasExpired(styleHints->startDragTime()))
        m_state = AboutToDrag;

    qreal distance = (m_startPoint - m_card->mapToItem(m_table, event->pos())).manhattanLength();
    if (distance >= styleHints->startDragDistance())
        m_state = AboutToDrag;

    return m_state == NoDrag;
}

bool Drag::couldBeDoubleClick(const Card *card)
{
    qint64 time = (card == s_lastCard && s_doubleClickTimer.isValid()) ?
        s_doubleClickTimer.elapsed() : std::numeric_limits<qint64>::max();
    s_lastCard = card;

    if (time <= QGuiApplication::styleHints()->mouseDoubleClickInterval()) {
        s_doubleClickTimer.invalidate();
        return true;
    } else {
        s_doubleClickTimer.start();
        return false;
    }
}

quint32 Drag::nextId()
{
    quint32 id = ++s_count;
    if (id == 0) // We wrapped around, 0 is not an acceptable value
        id = ++s_count;
    return id;
}

CardList Drag::toCardData(const QList<Card *> &cards, DragState state)
{
    CardList list;
    for (const Card *card : cards) {
        if (card)
            list << card->data();
        else
            qCCritical(lcDrag) << "Tried to convert non-existing card to data";
    }
    if (list.isEmpty())
        qCCritical(lcDrag) << "Returning an empty list of CardData in state" << state;
    return list;
}

QDebug operator<<(QDebug debug, const Drag *drag)
{
    if (drag) {
        debug.nospace() << "Drag(";
        switch (drag->m_state) {
        case Drag::NoDrag:
            debug.nospace() << "NoDrag";
            break;
        case Drag::AboutToDrag:
            debug.nospace() << "AboutToDrag";
            break;
        case Drag::StartingDrag:
            debug.nospace() << "StartingDrag";
            break;
        case Drag::Dragging:
            debug.nospace() << "Dragging";
            break;
        case Drag::Dropping:
            debug.nospace() << "Dropping";
            break;
        case Drag::Dropped:
            debug.nospace() << "Dropped";
            break;
        case Drag::Finished:
            debug.nospace() << "Finished";
            break;
        case Drag::Canceled:
            debug.nospace() << "Canceled";
            break;
        case Drag::Clicked:
            debug.nospace() << "Clicked";
            break;
        }
        debug.nospace() << ")";
    } else {
        debug.nospace() << "invalid drag";
    }
    return debug.space();
}
