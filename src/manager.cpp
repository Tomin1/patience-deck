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
    connect(engine, &Engine::insertCard, this, &Manager::handleInsertCard);
    connect(engine, &Engine::appendCard, this, &Manager::handleAppendCard);
    connect(engine, &Engine::removeCard, this, &Manager::handleRemoveCard);
    connect(engine, &Engine::flipCard, this, &Manager::handleFlipCard);
    connect(engine, &Engine::clearSlot, this, &Manager::handleClearSlot);
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

void Manager::handleInsertCard(int slotId, int index, const CardData &data)
{
    if (m_preparing) {
        Slot *slot = m_table->slot(slotId);
        Card *card = new Card(data, m_table, slot, this);
        slot->insert(index, card);
    } else {
        queue(Action::InsertionAction, slotId, index, data);
    }
}

void Manager::handleAppendCard(int slotId, const CardData &data)
{
    if (m_preparing) {
        Slot *slot = m_table->slot(slotId);
        Card *card = new Card(data, m_table, slot, this);
        slot->append(card);
    } else {
        queue(Action::InsertionAction, slotId, -1, data);
    }
}

void Manager::handleRemoveCard(int slotId, int index, const CardData &card)
{
    if (m_preparing) {
        Slot *slot = m_table->slot(slotId);
        Card *card = slot->takeAt(index);
        card->setParentItem(nullptr);
        card->deleteLater();
    } else {
        queue(Action::RemovalAction, slotId, index, card);
    }
}

void Manager::handleFlipCard(int slotId, int index, const CardData &card)
{
    if (m_preparing) {
        Slot *slot = m_table->slot(slotId);
        auto it = slot->begin() + index;
        (*it)->setShow(card.show);
    } else {
        queue(Action::FlipAction, slotId, index, card);
    }
}

void Manager::handleClearSlot(int slotId)
{
    if (m_preparing) {
        auto cards = m_table->slot(slotId)->takeAll();
        for (Card *card : cards) {
            card->setParentItem(nullptr);
            card->deleteLater();
        }
    } else {
        CardData none;
        queue(Action::ClearAction, slotId, -1, none);
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
    qCDebug(lcManager) << "Started preparing while" << *this;
}

void Manager::handleGameStarted()
{
    m_preparing = false;
    m_table->updateCardSize();
    qCDebug(lcManager) << "Stopped preparing while" << *this;
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

void Manager::queue(Action::ActionType type, int slotId, int index, const CardData &data)
{
    if (lcManager().isDebugEnabled()) {
        switch (type) {
        case Action::InsertionAction:
            qCDebug(lcManager) << "Queueing insertion of" << data << "for slot" << slotId << "to index" << index;
            break;
        case Action::RemovalAction:
            qCDebug(lcManager) << "Queueing removal of" << data << "for slot" << slotId << "from index" << index;
            break;
        case Action::FlipAction:
            qCDebug(lcManager) << "Queueing flipping of" << data << "for slot" << slotId << "at index" << index;
            break;
        case Action::ClearAction:
            qCDebug(lcManager) << "Queueing clearing of slot" << slotId;
            break;
        }
    }
    m_actions[slotId].append(Action(type, slotId, index, data));
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

    switch (action->type) {
    case Action::InsertionAction:
        if (m_cards.contains(action->suitAndRank())) {
            qCDebug(lcManager) << "Dequeueing insertion of" << action->data
                               << "for slot" << action->slot << "to index" << action->index;
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
    case Action::RemovalAction:
        {
            qCDebug(lcManager) << "Dequeueing removal of" << action->data
                               << "for slot" << action->slot << "from index" << action->index;
            Card *card = slot->takeAt(action->index);
            if (card->rank() != action->data.rank || card->suit() != action->data.suit)
                qCCritical(lcManager) << "Wrong card taken! Was" << *card << "should be" << action->data;
            store(card);
            handled = true;
            break;
        }
    case Action::FlipAction:
        {
            qCDebug(lcManager) << "Dequeueing flipping of" << action->data
                               << "for slot" << action->slot << "at index" << action->index;
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
    case Action::ClearAction:
        {
            qCDebug(lcManager) << "Dequeueing clearing of slot" << action->slot;
            Slot *slot = m_table->slot(action->slot);
            store(slot->takeAll());
            handled = true;
            break;
        }
    }

    return handled;
}

Manager::operator QString() const
{
    int count = 0;
    for (const auto list : m_actions)
        count += list.count();
    return QStringLiteral("storing %1 cards for %2 actions")
        .arg(m_cards.count())
        .arg(count);
}

Manager::Action::Action(ActionType type, int slot, int index, const CardData &data)
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

Manager::Action::operator QString() const
{
    return (type == RemovalAction
            ? QStringLiteral("Removal of %1 from %2 at %3")
            : QStringLiteral("Insertion of %1 to %2 at %3"))
        .arg(data)
        .arg(slot)
        .arg(index);
}
