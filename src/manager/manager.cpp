/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2021 Tomi Leppänen
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

#include "manager.h"
#include "logging.h"
#include "card.h"
#include "slot.h"
#include "table.h"

Manager::Manager(Table *table)
    : QObject(table)
    , m_table(table)
    , m_preparing(true)
{
    auto engine = Engine::instance();
    connect(engine, &Engine::newSlot, this, &Manager::handleNewSlot);
    connect(engine, &Engine::action, this, &Manager::handleAction);
    connect(engine, &Engine::clearData, this, &Manager::handleClearData);
    connect(engine, &Engine::gameStarted, this, &Manager::handleGameStarted);
    connect(engine, &Engine::moveEnded, this, &Manager::handleMoveEnded);
}

bool Manager::preparing() const
{
    return m_preparing;
}

void Manager::handleNewSlot(int id, const CardList &dataList, int type,
                            double x, double y, int expansionDepth,
                            bool expandedDown, bool expandedRight)
{
    if (!m_preparing) {
        qCWarning(lcManager) << "Trying to insert slot although game is not being prepared";
    } else {
        Slot *slot = new Slot(id, SlotType(type), x, y, expansionDepth,
                              expandedDown, expandedRight, m_table);
        if (!dataList.isEmpty()) {
            QList<Card *> cards;
            for (const CardData &data : dataList) {
                auto card = new Card(data, m_table, slot, this);
                cards.append(card);
            }
            slot->put(cards);
        }
        m_table->addSlot(slot);
        m_queue.addSlot(id);
    }
}

void Manager::handleAction(Engine::ActionType action, int slotId, int index, const CardData &data)
{
    if (m_preparing) {
        handleImmediately(action, slotId, index, data);
    } else {
        m_queue.queue(action, slotId, index, data);
    }
}

void Manager::handleImmediately(Engine::ActionType action, int slotId, int index, const CardData &data)
{
    Slot *slot = m_table->slot(slotId);
    switch (action) {
    case Engine::InsertionAction:
        {
            Card *card = new Card(data, m_table, slot, this);
            slot->insert(index, card);
            break;
        }
    case Engine::RemovalAction:
        {
            Card *card = slot->takeAt(index);
            card->shred();
            break;
        }
    case Engine::FlippingAction:
        {
            auto it = slot->begin() + index;
            (*it)->setShow(data.show);
            break;
        }
    case Engine::ClearingAction:
        {
            auto cards = slot->takeAll();
            for (Card *card : cards) {
                if (card)
                    card->shred();
            }
            break;
        }
    }
}

bool Manager::handleQueued(const Action &action) {
    bool handled = false;

    Slot *slot = m_table->slot(action.slot);
    switch (action.type) {
    case Engine::InsertionAction:
        {
            if (!action.replaces)
                m_queue.incrementQueued(slot->id(), action.index);
            Card *card = m_queue.take(action);
            if (card) {
                handled = true;
                card->setShow(action.data.show);
                card->update();
                qCDebug(lcManager) << "Inserted" << card << "to" << slot << "at" << action.index;
            } else {
                qCDebug(lcManager) << "Inserted placeholder for" << action.data << "to" << slot << "at" << action.index;
            }
            if (action.replaces)
                slot->set(action.index, card);
            else
                slot->insert(action.index, card);
            break;
        }
    case Engine::RemovalAction:
        {
            Card *card = slot->takeAt(action.index);
            m_queue.decrementQueued(slot->id(), action.index, action.data);
            if (card) {
                if (card->rank() != action.data.rank || card->suit() != action.data.suit)
                    qCCritical(lcManager) << "Wrong card taken! Was" << card << "should be" << action.data;
                card->moveTo(m_table);
                m_queue.store(card);
            } else {
                qCDebug(lcManager) << "Removed placeholder from" << slot << "at" << action.index;
            }
            handled = true;
            break;
        }
    case Engine::FlippingAction:
        {
            auto it = slot->begin() + action.index;
            Card *card = *it;
            if (card) {
                card->setShow(action.data.show);
                if (card->rank() != action.data.rank || card->suit() != action.data.suit)
                    qCCritical(lcManager) << "Rank or suit doesn't match to" << action.data
                                          << "for" << card << "in" << slot << "at index" << action.index;
                card->update();
            } else {
                m_queue.flipQueued(slot->id(), action.index, action.data);
            }
            handled = true;
            break;
        }
    case Engine::ClearingAction:
        {
            auto cards = slot->takeAll();
            m_queue.clearQueued(slot->id());
            store(cards);
            handled = true;
            break;
        }
    }
    return handled;
}

void Manager::handleClearData()
{
    m_preparing = true;
    for (Slot *slot : *m_table) {
        for (Card *card : slot->takeAll()) {
            if (card) {
                card->shred();
            }
        }
        slot->setParentItem(nullptr);
        slot->deleteLater();
    }
    m_table->clear();
    m_queue.clear();
    for (Card *card : m_queue.takeAll())
        card->deleteLater();
    qCDebug(lcManager) << "Started preparing while storing" << m_queue.cardCount()
                       << "for" << m_queue.actionCount() << "actions";
}

void Manager::handleGameStarted()
{
    m_preparing = false;
    m_table->setDirtyCardSize();
    qCDebug(lcManager) << "Stopped preparing while storing" << m_queue.cardCount()
                       << "for" << m_queue.actionCount() << "actions";
}

void Manager::handleMoveEnded()
{
    for (auto it = m_queue.begin(); it != m_queue.end(); ++it) {
        if (!handleQueued(*it))
            it.requeue();
    }

    // Hide all unused cards
    for (auto it = m_queue.beginRecent(); it != m_queue.endRecent(); ++it)
        (*it)->setParentItem(nullptr);

    m_queue.clear();
}

void Manager::store(const QList<Card *> &cards)
{
    for (Card *card : cards) {
        if (card) {
            card->moveTo(m_table);
            m_queue.store(card);
        } else {
            qCWarning(lcManager) << "Skipped placeholder card";
        }
    }
}
