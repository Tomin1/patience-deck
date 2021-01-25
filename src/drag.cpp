/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2020  Tomi Leppänen
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
#include "drag.h"
#include "table.h"
#include "card.h"
#include "slot.h"
#include "constants.h"
#include "logging.h"

namespace {

CardList toCardData(const QList<Card *> &cards)
{
    CardList list;
    for (const Card *card : cards)
        list << card->data();
    if (list.isEmpty()) {
        qCCritical(lcPatience) << "Returning an empty list of CardData";
        abort();
    }
    return list;
}

} // namespace

quint32 Drag::s_count = 0;

Drag::Drag(QMouseEvent *event, Table *table, Slot *slot, Card *card)
    : QObject(card)
    , m_id(s_count++)
    , m_mayBeAClick(true)
    , m_completed(false)
    , m_table(table)
    , m_card(card)
    , m_source(slot)
    , m_target(nullptr)
{
    auto engine = Engine::instance();
    connect(this, &Drag::doDrag, engine, &Engine::drag);
    connect(this, &Drag::doCancelDrag, engine, &Engine::cancelDrag);
    connect(this, &Drag::doCheckDrop, engine, &Engine::checkDrop);
    connect(this, &Drag::doDrop, engine, &Engine::drop);
    connect(this, &Drag::doClick, engine, &Engine::click);
    connect(engine, &Engine::couldDrag, this, &Drag::handleCouldDrag);
    connect(engine, &Engine::couldDrop, this, &Drag::handleCouldDrop);
    connect(engine, &Engine::dropped, this, &Drag::handleDropped);

    m_startPoint = m_lastPoint = card->mapToItem(m_table, event->pos());
    m_timer.start();
}

Drag::~Drag()
{
    if (!m_cards.isEmpty()) {
        if (m_completed) {
            for (Card *card : m_cards) {
                card->setParentItem(nullptr);
                card->deleteLater();
            }
        } else {
            emit doCancelDrag(m_id, m_source->id(), toCardData(m_cards));
            m_source->put(m_cards);
        }
        m_cards.clear();
    }
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
    if (!m_mayBeAClick)
        ; // continue
    else if (!testClick(event))
        emit doDrag(m_id, m_source->id(), m_source->asCardData(m_card));
    else
        return;

    if (m_cards.isEmpty())
        return;

    QPointF point = m_card->mapToItem(m_table, event->pos());
    QPointF move = point - m_lastPoint;
    for (Card *card : m_cards) {
        card->setX(card->x() + move.x());
        card->setY(card->y() + move.y());
    }
    m_lastPoint = point;
}

void Drag::finish(QMouseEvent *event)
{
    if (testClick(event)) {
        qCDebug(lcPatience) << "Detected click on" << m_card;
        emit doClick(m_id, m_source->id());
        deleteLater();
        return;
    }

    if (m_cards.isEmpty())
        return;

    m_targets = m_table->getSlotsFor(m_card, m_source);

    handleCouldDrop(m_id, false);
}

void Drag::cancel()
{
    if (!m_cards.isEmpty()) {
        emit doCancelDrag(m_id, m_source->id(), toCardData(m_cards));
        m_source->put(m_cards);
        m_cards.clear();
    }

    deleteLater();
}

void Drag::handleCouldDrag(quint32 id, bool could)
{
    if (id != m_id)
        return;

    if (could) {
        m_cards = m_source->take(m_card);
        for (Card *card : m_cards) {
            card->setParentItem(m_table);
            QPointF position = m_table->mapFromItem(m_source, QPointF(card->x(), card->y()));
            card->setX(position.x());
            card->setY(position.y());
        }
    }
}

void Drag::handleCouldDrop(quint32 id, bool could)
{
    if (id != m_id)
        return;

    if (could) {
        qCDebug(lcPatience) << "Moving from" << m_source->id() << "to" << m_target->id();
        emit doDrop(m_id, m_source->id(), m_target->id(), toCardData(m_cards));
    } else if (!m_targets.isEmpty()) {
        m_target = m_targets.takeFirst();
        qCDebug(lcPatience) << "Trying to moving from" << m_source->id() << "to" << m_target->id();
        emit doCheckDrop(m_id, m_source->id(), m_target->id(), toCardData(m_cards));
    } else {
        cancel();
        deleteLater();
    }
}

void Drag::handleDropped(quint32 id, bool could)
{
    if (id != m_id)
        return;

    if (could)
        m_completed = true;
    else
        cancel();

    deleteLater();
}

bool Drag::testClick(QMouseEvent *event)
{
    if (!m_mayBeAClick)
        return false;

    auto styleHints = QGuiApplication::styleHints();
    if (m_timer.hasExpired(styleHints->startDragTime()))
        m_mayBeAClick = false;

    qreal distance = (m_startPoint - m_card->mapToItem(m_table, event->pos())).manhattanLength();
    if (distance >= styleHints->startDragDistance())
        m_mayBeAClick = false;

    return m_mayBeAClick;
}
