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
            for (const CardData &data: dataList) {
                auto card = new Card(data, m_table, slot);
                cards.append(card);
            }
            slot->put(cards);
        }
        m_table->addSlot(slot);
    }
}

void Manager::handleInsertCard(int slotId, int index, const CardData &data)
{
    if (m_preparing) {
        Slot *slot = m_table->slot(slotId);
        Card *card = new Card(data, m_table, slot);
        slot->insert(index, card);
    } else {
        queue(slotId, index, data);
        dequeue();
    }
}

void Manager::handleAppendCard(int slotId, const CardData &data)
{
    if (m_preparing) {
        Slot *slot = m_table->slot(slotId);
        Card *card = new Card(data, m_table, slot);
        slot->append(card);
    } else {
        queue(slotId, -1, data);
        dequeue();
    }
}

void Manager::handleRemoveCard(int slotId, int index)
{
    if (m_preparing) {
        Slot *slot = m_table->slot(slotId);
        Card *card = slot->takeAt(index);
        card->setParentItem(nullptr);
        card->deleteLater();
    } else {
        qCDebug(lcManager) << "Storing from slot" << slotId << "at" << index;
        Slot *slot = m_table->slot(slotId);
        if (slot->count() > index) {
            Card *card = slot->takeAt(index);
            store(card);
        }
        dequeue();
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
        qCDebug(lcManager) << "Clearing slot" << slotId;
        Slot *slot = m_table->slot(slotId);
        store(slot->takeAll()); // this calls dequeue()
    }
}

void Manager::handleClearData()
{
    m_preparing = true;
    m_table->clear();
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
    if (!m_cards.empty() || !m_insertions.empty())
        qCWarning(lcManager) << "There were still" << m_cards.count() << "cards and"
                           << m_insertions.count() << "insertions to be relayed when move ended!";
}

void Manager::store(Card *card)
{
    qCDebug(lcManager) << "Storing" << *card;
    card->setParentItem(nullptr);
    card->setParent(this);
    m_cards.insertMulti(SuitAndRank(card->suit(), card->rank()), card);
}

void Manager::store(QList<Card *> cards)
{
    for (auto card : cards)
        store(card);
    dequeue();
}

void Manager::queue(int slotId, int index, const CardData &data)
{
    qCDebug(lcManager) << "Queueing insertion of" << data << "for slot" << slotId
                     << "to index" << index;
    m_insertions.append(Insertion(slotId, index, data));
}

const Manager::Insertion *Manager::nextAction() const
{
    return m_insertions.isEmpty() ? nullptr : &m_insertions.first();
}

void Manager::dequeue()
{
    const Insertion *insertion;
    while ((insertion = nextAction()) && m_cards.contains(insertion->suitAndRank())) {
        qCDebug(lcManager) << "Dequeueing insertion of" << insertion->data
                         << "for slot" << insertion->slot << "to index" << insertion->index;
        Slot *slot = m_table->slot(insertion->slot);
        Card *card = m_cards.take(insertion->suitAndRank());
        card->setShow(insertion->data.show);
        if (insertion->index == -1)
            slot->append(card);
        else
            slot->insert(insertion->index, card);
        card->update();
        m_insertions.removeFirst();
    }
}

Manager::operator QString() const
{
    return QStringLiteral("storing %1 cards for %2 insertions")
        .arg(m_cards.count())
        .arg(m_insertions.count());
};
