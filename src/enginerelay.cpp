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

#include "enginerelay.h"
#include "logging.h"
#include "card.h"
#include "table.h"

EngineRelay::EngineRelay(Table *table)
    : QObject(table)
    , m_table(table)
    , m_preparing(true)
{
    auto engine = Engine::instance();
    connect(engine, &Engine::insertCard, this, &EngineRelay::handleInsertCard);
    connect(engine, &Engine::appendCard, this, &EngineRelay::handleAppendCard);
    connect(engine, &Engine::removeCard, this, &EngineRelay::handleRemoveCard);
    connect(engine, &Engine::clearSlot, this, &EngineRelay::handleClearSlot);
    connect(engine, &Engine::clearData, this, &EngineRelay::handleClearData);
    connect(engine, &Engine::gameStarted, this, &EngineRelay::handleGameStarted);
    connect(engine, &Engine::moveEnded, this, &EngineRelay::handleMoveEnded);
}

void EngineRelay::handleInsertCard(int slotId, int index, const CardData &card)
{
    if (m_preparing) {
        emit insertNewCard(slotId, index, card);
    } else {
        queue(slotId, index, card);
        dequeue();
    }
}

void EngineRelay::handleAppendCard(int slotId, const CardData &card)
{
    if (m_preparing) {
        emit appendNewCard(slotId, card);
    } else {
        queue(slotId, -1, card);
        dequeue();
    }
}

void EngineRelay::handleRemoveCard(int slotId, int index)
{
    if (m_preparing) {
        emit removeNewCard(slotId, index);
    } else {
        qCDebug(lcRelay) << "Storing from slot" << slotId << "at" << index;
        Slot *slot = m_table->slot(slotId);
        if (slot->count() > index) {
            Card *card = slot->takeAt(index);
            store(card);
        }
        dequeue();
    }
}

void EngineRelay::handleClearSlot(int slotId)
{
    if (m_preparing) {
        emit clearSlot(slotId);
    } else {
        qCDebug(lcRelay) << "Clearing slot" << slotId;
        Slot *slot = m_table->slot(slotId);
        store(slot->takeAll()); // this calls dequeue()
    }
}

void EngineRelay::handleClearData()
{
    m_preparing = true;
    qCDebug(lcRelay) << "Started preparing while" << *this;
}

void EngineRelay::handleGameStarted()
{
    m_preparing = false;
    qCDebug(lcRelay) << "Stopped preparing while" << *this;
}

void EngineRelay::handleMoveEnded()
{
    if (!m_cards.empty() || !m_insertions.empty())
        qCWarning(lcRelay) << "There were still" << m_cards.count() << "cards and"
                           << m_insertions.count() << "insertions to be relayed when move ended!";
}

void EngineRelay::store(Card *card)
{
    qCDebug(lcRelay) << "Storing" << *card;
    card->setParentItem(nullptr);
    card->setParent(this);
    m_cards.insertMulti(SuitAndRank(card->suit(), card->rank()), card);
}

void EngineRelay::store(QList<Card *> cards)
{
    for (auto card : cards)
        store(card);
    dequeue();
}

void EngineRelay::queue(int slotId, int index, const CardData &data)
{
    qCDebug(lcRelay) << "Queueing insertion of" << data << "for slot" << slotId
                     << "to index" << index;
    m_insertions.append(Insertion(slotId, index, data));
}

const EngineRelay::Insertion *EngineRelay::nextAction() const
{
    return m_insertions.isEmpty() ? nullptr : &m_insertions.first();
}

void EngineRelay::dequeue()
{
    const Insertion *insertion;
    while ((insertion = nextAction()) && m_cards.contains(insertion->suitAndRank())) {
        qCDebug(lcRelay) << "Dequeueing insertion of" << insertion->data
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

EngineRelay::operator QString() const
{
    return QStringLiteral("storing %1 cards for %2 insertions")
        .arg(m_cards.count())
        .arg(m_insertions.count());
};
