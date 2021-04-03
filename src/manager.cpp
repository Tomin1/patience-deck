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
        m_actions.insert(id, QLinkedList<Action>());
    }
}

void Manager::handleAction(Engine::ActionType action, int slotId, int index, const CardData &data)
{
    if (m_preparing) {
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
                card->setParentItem(nullptr);
                card->deleteLater();
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
                    card->setParentItem(nullptr);
                    card->deleteLater();
                }
                break;
            }
        }
    } else {
        queue(action, slotId, index, data);
    }
}

void Manager::handleClearData()
{
    m_preparing = true;
    for (int slotId : *m_table) {
        Slot *slot = m_table->slot(slotId);
        auto cards = slot->takeAll();
        for (Card *card : cards) {
            card->setParentItem(nullptr);
            card->deleteLater();
        }
        slot->setParentItem(nullptr);
        slot->deleteLater();
    }
    m_table->clear();
    for (Card *card : m_cards)
        card->deleteLater();
    m_cards.clear();
    m_actions.clear();
    qCDebug(lcManager) << "Started preparing while storing" << m_cards.count()
                       << "for" << actionCount() << "actions";
}

void Manager::handleGameStarted()
{
    m_preparing = false;
    m_table->updateCardSize();
    qCDebug(lcManager) << "Stopped preparing while storing" << m_cards.count()
                       << "for" << actionCount() << "actions";
}

void Manager::handleMoveEnded()
{
    dequeue();

    int count = 0;
    for (const auto list : m_actions)
        count += list.count();

    if (count > 0) {
        qCWarning(lcManager) << "There were still" << count << "actions to be done"
                             << "and" << m_cards.count() << "cards stored when move ended!";
        if (lcManager().isDebugEnabled()) {
            for (const auto list : m_actions) {
                for (const auto action : list) {
                    qCDebug(lcManager) << action;
                }
            }
        }
    }
}

void Manager::store(Card *card)
{
    qCDebug(lcManager) << "Storing" << *card;
    card->setParentItem(nullptr);
    m_cards.insertMulti(SuitAndRank(card->suit(), card->rank()), card);
}

void Manager::store(const QList<Card *> &cards)
{
    for (Card *card : cards)
        store(card);
}

void Manager::queue(Engine::ActionType type, int slotId, int index, const CardData &data)
{
    Action action(type, slotId, index, data);
    qCDebug(lcManager) << "Queueing" << action;
    m_actions[slotId].append(action);
}

const Manager::Action *Manager::nextAction(int slot) const
{
    return m_actions[slot].isEmpty() ? nullptr : &m_actions[slot].first();
}

void Manager::discardAction(int slot)
{
    m_actions[slot].removeFirst();
}

void Manager::dequeue()
{
    const Action *action;
    while (true) {
        int changes = 0;
        for (int slotId : *m_table) {
            while ((action = nextAction(slotId))) {
                Slot *slot = m_table->slot(slotId);
                if (handle(slot, action)) {
                    discardAction(slot->id());
                    changes++;
                } else {
                    break;
                }
            }
        }

        qCDebug(lcManager) << changes << "on this round";

        if (!changes)
            break;
    }
}

bool Manager::handle(Slot *slot, const Action *action) {
    bool handled = false;

    qCDebug(lcManager) << "Dequeueing" << *action;
    switch (action->type) {
    case Engine::InsertionAction:
        if (m_cards.contains(action->suitAndRank())) {
            Card *card = m_cards.take(action->suitAndRank());
            card->setShow(action->data.show);
            if (action->index == -1)
                slot->append(card);
            else
                slot->insert(action->index, card);
            card->update();
            handled = true;
        }
        break;
    case Engine::RemovalAction:
        {
            Card *card = slot->takeAt(action->index);
            if (card->rank() != action->data.rank || card->suit() != action->data.suit)
                qCCritical(lcManager) << "Wrong card taken! Was" << *card << "should be" << action->data;
            store(card);
            handled = true;
            break;
        }
    case Engine::FlippingAction:
        {
            Slot *slot = m_table->slot(action->slot);
            auto it = slot->begin() + action->index;
            Card *card = *it;
            card->setShow(action->data.show);
            if (card->rank() != action->data.rank || card->suit() != action->data.suit)
                qCCritical(lcManager) << "Rank or suit doesn't match to" << action->data
                                      << "for card" << *card << "in slot" << action->slot
                                      << "at index" << action->index;
            card->update();
            handled = true;
            break;
        }
    case Engine::ClearingAction:
        {
            Slot *slot = m_table->slot(action->slot);
            store(slot->takeAll());
            handled = true;
            break;
        }
    }

    return handled;
}

int Manager::actionCount() const
{
    int count = 0;
    for (const auto list : m_actions)
        count += list.count();
    return count;
}

Manager::Action::Action(Engine::ActionType type, int slot, int index, const CardData &data)
    : type(type)
    , slot(slot)
    , index(index)
    , data(data)
{
}

Manager::SuitAndRank Manager::Action::suitAndRank() const
{
    return SuitAndRank(data.suit, data.rank);
}

QDebug operator<<(QDebug debug, const Manager::Action &action)
{
    switch (action.type) {
    case Engine::InsertionAction:
        debug.nospace() << "insertion of " << action.data << " to " << action.slot << " at " << action.index;
        break;
    case Engine::RemovalAction:
        debug.nospace() << "removal of " << action.data << " from " << action.slot << " at " << action.index;
        break;
    case Engine::FlippingAction:
        debug.nospace() << "flipping of " << action.data << " in " << action.slot << " at " << action.index;
        break;
    case Engine::ClearingAction:
        debug.nospace() << "clearing slot " << action.slot;
    }
    return debug.space();
}
