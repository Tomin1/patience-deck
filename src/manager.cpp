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
        m_insertions.insert(id, QLinkedList<Insertion>());
    }
}

void Manager::handleAction(Engine::ActionType action, int slotId, int index, const CardData &data)
{
    Slot *slot = m_table->slot(slotId);
    switch (action) {
    case Engine::InsertionAction:
        {
            Card *card = nullptr;
            if (m_preparing) {
                card = new Card(data, m_table, slot, this);
            } else {
                for (auto &insertion : m_insertions[slotId]) {
                    if (insertion.index >= index)
                        insertion.index++;
                }
                card = m_cards.take(SuitAndRank(data.suit, data.rank));
                if (card) {
                    card->setShow(data.show);
                    card->update();
                    qCDebug(lcManager) << "Inserted" << *card << "to" << *slot << "at" << index;
                } else {
                    queue(slotId, index, data);
                }
            }
            slot->insert(index, card);
            break;
        }
    case Engine::RemovalAction:
        {
            Card *card = slot->takeAt(index);
            if (m_preparing) {
                card->setParentItem(nullptr);
                card->deleteLater();
            } else {
                for (auto it = m_insertions[slotId].begin(); it != m_insertions[slotId].end(); it++) {
                    if (it->index == index)
                        it = m_insertions[slotId].erase(it);
                    else if (it->index > index)
                        it->index--;
                }
                if (card) {
                    if (card->rank() != data.rank || card->suit() != data.suit)
                        qCCritical(lcManager) << "Wrong card taken! Was" << *card << "should be" << data;
                    store(card);
                } else {
                    qCDebug(lcManager) << "Removed null card from" << *slot << "at" << index;
                }
            }
            break;
        }
    case Engine::FlippingAction:
        {
            auto it = slot->begin() + index;
            Card *card = *it;
            if (card) {
                card->setShow(data.show);
                if (card->rank() != data.rank || card->suit() != data.suit)
                    qCCritical(lcManager) << "Rank or suit doesn't match to" << data
                                          << "for card" << *card << "in slot" << slot
                                          << "at index" << index;
                card->update();
            } else {
                for (auto &insertion : m_insertions[slotId]) {
                    if (insertion.index == index) {
                        insertion.data.show = data.show;
                        if (insertion.data.rank != data.rank || insertion.data.suit != data.suit)
                                qCCritical(lcManager) << "Rank or suit doesn't match to" << data
                                                      << "for insertion" << insertion << "in slot" << slot
                                                      << "at index" << index;
                        break;
                    }
                }
            }
            break;
        }
    case Engine::ClearingAction:
        {
            auto cards = slot->takeAll();
            if (m_preparing) {
                for (Card *card : cards) {
                    if (card) {
                        card->setParentItem(nullptr);
                        card->deleteLater();
                    }
                }
            } else {
                m_insertions[slotId].clear();
                store(cards);
            }
            break;
        }
    }
}

void Manager::handleClearData()
{
    m_preparing = true;
    for (int slotId : *m_table) {
        Slot *slot = m_table->slot(slotId);
        auto cards = slot->takeAll();
        for (Card *card : cards) {
            if (card) {
                card->setParentItem(nullptr);
                card->deleteLater();
            }
        }
        slot->setParentItem(nullptr);
        slot->deleteLater();
    }
    m_table->clear();
    for (Card *card : m_cards)
        card->deleteLater();
    m_cards.clear();
    m_insertions.clear();
    qCDebug(lcManager) << "Started preparing while storing" << m_cards.count()
                       << "for" << insertionCount() << "insertions";
}

void Manager::handleGameStarted()
{
    m_preparing = false;
    m_table->setDirtyCardSize();
    qCDebug(lcManager) << "Stopped preparing while storing" << m_cards.count()
                       << "for" << insertionCount() << "insertions";
}

void Manager::handleMoveEnded()
{
    dequeue();

    int count = insertionCount();

    if (count > 0) {
        qCWarning(lcManager) << "There were still" << count << "insertions to be done"
                             << "and" << m_cards.count() << "cards stored when move ended!";
        if (lcManager().isDebugEnabled()) {
            for (const auto list : m_insertions) {
                for (const auto action : list)
                    qCDebug(lcManager) << action;
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
        if (card)
            store(card);
}

void Manager::queue(int slotId, int index, const CardData &data)
{
    Insertion insertion(slotId, index, data);
    qCDebug(lcManager) << "Queueing" << insertion;
    m_insertions[slotId].append(insertion);
}

void Manager::dequeue()
{
    for (auto it = m_insertions.begin(); it != m_insertions.end(); it++) {
        Slot *slot = m_table->slot(it.key());
        for (Insertion insertion : it.value()) {
            Card *card = m_cards.take(insertion.suitAndRank());
            if (!card) {
                qCCritical(lcManager) << "No such card to insert" << insertion.suitAndRank();
            } else {
                card->setShow(insertion.data.show);
                slot->set(insertion.index, card);
                card->update();
            }
        }
        it.value().clear();
    }
}

int Manager::insertionCount() const
{
    int count = 0;
    for (const auto list : m_insertions)
        count += list.count();
    return count;
}

Manager::Insertion::Insertion(int slot, int index, const CardData &data)
    : slot(slot)
    , index(index)
    , data(data)
{
}

Manager::SuitAndRank Manager::Insertion::suitAndRank() const
{
    return SuitAndRank(data.suit, data.rank);
}

QDebug operator<<(QDebug debug, const Manager::Insertion &insertion)
{
    debug.nospace() << "insertion of " << insertion.data << " to " << insertion.slot
                    << " at " << insertion.index;
    return debug.space();
}
