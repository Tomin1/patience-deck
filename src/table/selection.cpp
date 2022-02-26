/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2022 Tomi Leppänen
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

#include "card.h"
#include "engine.h"
#include "feedbackevent.h"
#include "logging.h"
#include "selection.h"
#include "slot.h"
#include "table.h"

Selection::Selection(Table *table, Slot *slot, Card *card)
    : QObject(table)
    , m_state(NoSelection)
    , m_table(table)
    , m_source(slot)
    , m_card(card)
    , m_target(nullptr)
    , m_vibrationBlocked(false)
{
    auto engine = Engine::instance();
    connect(this, &Selection::doDrag, engine, &Engine::drag);
    connect(this, &Selection::doCancelDrag, engine, &Engine::cancelDrag);
    connect(this, &Selection::doCheckDrop, engine, &Engine::checkDrop);
    connect(this, &Selection::doDrop, engine, &Engine::drop);
    connect(engine, &Engine::couldDrag, this, &Selection::handleCouldDrag);
    connect(engine, &Engine::couldDrop, this, &Selection::handleCouldDrop);
    connect(engine, &Engine::dropped, this, &Selection::handleDropped);

    emit doDrag(id(), slot->id(), slot->asCardData(card));

    qCDebug(lcSelection) << "Checking for selection of" << card << "for" << slot;
}

Selection::Selection(const Selection *other)
    : QObject(other->parent())
    , CountableId(other)
    , m_state(Dead)
    , m_table(other->m_table)
    , m_source(other->m_source)
    , m_card(other->m_card)
    , m_target(other->m_target)
    , m_cards(other->m_cards)
    , m_vibrationBlocked(other->m_vibrationBlocked)
{
    qCDebug(lcSelection) << "Created a dead selection";
}

Selection::~Selection()
{
    if (m_state < Finished)
        qCWarning(lcDrag) << "Selection was not finished or canceled when it was destroyed";
}

Selection *Selection::clone() const
{
    return new Selection(this);
}

Selection::SelectionState Selection::state() const
{
    return m_state;
}

void Selection::handleCouldDrag(quint32 id, int slotId, bool could)
{
    Q_UNUSED(slotId)

    if (id != CountableId::id())
        return;

    if (m_state == NoSelection && could) {
        m_state = Selected;
        m_table->highlight(m_source, m_card);
        qCDebug(lcSelection) << "Selected cards in" << m_source << "beginning with" << m_card;
        if (m_target)
            checkTarget(m_target);
    } else if (m_state == Canceled) {
        if (could)
            emit doCancelDrag(CountableId::id(), m_source->id(), m_source->asCardData(m_card));
        qCDebug(lcSelection) << this << "was canceled early";
        done();
    } else {
        qCDebug(lcSelection) << "Could not select cards from" << m_source << "beginning with" << m_card;
        cancel();
    }
}

Slot *Selection::slot() const
{
    return m_source;
}

Card *Selection::card() const
{
    return m_card;
}

bool Selection::contains(const Card *card) const
{
    for (auto it = m_source->constFind(m_card); it != m_source->constEnd(); ++it) {
        if (*it == card)
            return true;
    }
    return false;
}

void Selection::blockVibration()
{
    m_vibrationBlocked = true;
}

void Selection::finish(Slot *slot)
{
    switch (m_state) {
    case NoSelection:
        m_target = slot;
        break;
    case Selected:
        checkTarget(slot);
        break;
    default:
        break;
    }
}

void Selection::checkTarget(Slot *slot)
{
    qCDebug(lcSelection) << "Checking move from" << m_source << "to" << slot;
    m_state = Dropping;
    m_target = slot;
    emit doCheckDrop(id(), m_source->id(), slot->id(), m_source->asCardData(m_card));
}

void Selection::handleCouldDrop(quint32 id, int slotId, bool could)
{
    Q_UNUSED(slotId)

    if (id != CountableId::id())
        return;

    if (m_state == Dropping && could) {
        m_cards = m_source->take(m_card);
        drop(m_target);
        qCDebug(lcSelection) << this << "dropping" << m_cards.count() << "cards to" << m_target;
    } else {
        if (!m_vibrationBlocked)
            emit m_table->feedback()->selectionChanged();
        qCDebug(lcSelection) << this << "can not drop to" << m_target;
        cancel();
    }
}

void Selection::drop(Slot *slot)
{
    qCDebug(lcSelection) << "Moving from" << m_source << "to" << slot;
    m_state = Dropped;
    emit doDrop(id(), m_source->id(), slot->id(), toCardData(m_cards, m_state));
}

void Selection::handleDropped(quint32 id, int slotId, bool could)
{
    Q_UNUSED(slotId)

    if (id != CountableId::id())
        return;

    if (m_state >= Finished) {
        qCWarning(lcSelection) << this << "has already handled dropping";
        return;
    }

    if (could) {
        qCDebug(lcSelection) << this << "finished dropping" << m_cards.count() << "cards to" << m_target;
        // Success vibration is not blocked
        emit m_table->feedback()->dropSucceeded();
        m_state = Finished;
        m_table->store(m_cards);
    } else {
        // Unlikely
        qCCritical(lcSelection) << this << "failed dropping of" << m_cards.count() << "cards to" << m_target;
        if (!m_vibrationBlocked)
            emit m_table->feedback()->selectionChanged();
        m_state = Canceled;
        m_source->put(m_cards);
    }
    m_cards.clear();
    done();
}

void Selection::cancel()
{
    qCDebug(lcSelection) << "Canceling selection of" << m_card << "at state" << m_state;
    if (m_state < Dropped) {
        SelectionState previousState = m_state;
        m_state = Canceled;
        if (previousState >= Selected)
            emit doCancelDrag(id(), m_source->id(), m_source->asCardData(m_card));
        done();
    }
}

void Selection::done()
{
    m_table->highlight(nullptr);
    emit finished();
    deleteLater();
}

CardList Selection::toCardData(const QList<Card *> &cards, SelectionState state)
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

QDebug operator<<(QDebug debug, const Selection *selection)
{
    if (selection) {
        debug.nospace() << "Selection(";
        switch (selection->m_state) {
        case Selection::NoSelection:
            debug.nospace() << "NoSelection";
            break;
        case Selection::Selected:
            debug.nospace() << "Selected";
            break;
        case Selection::Dropping:
            debug.nospace() << "Dropping";
            break;
        case Selection::Dropped:
            debug.nospace() << "Dropped";
            break;
        case Selection::Finished:
            debug.nospace() << "Finished";
            break;
        case Selection::Canceled:
            debug.nospace() << "Canceled";
            break;
        case Selection::Dead:
            debug.nospace() << "Dead";
            break;
        }
        debug.nospace() << ")";
    } else {
        debug.nospace() << "invalid selection";
    }
    return debug.space();
}
