/*
 * Exerciser for Patience Deck engine class.
 * Copyright (C) 2021-2022 Tomi Leppänen
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

#include "checker.h"

EngineChecker::EngineChecker(QObject *parent)
    : QObject(parent)
    , m_move(-1)
{
    auto engine = Engine::instance();
    connect(engine, &Engine::clearData, this, &EngineChecker::handleClearData);
    connect(engine, &Engine::newSlot, this, &EngineChecker::handleNewSlot);
    connect(engine, &Engine::gameStarted, this, &EngineChecker::handleGameStarted);
    connect(engine, &Engine::action, this, &EngineChecker::handleAction);
}

void EngineChecker::dump() const
{
    if (m_move < 0)
        qDebug() << "Errors while preparing:";
    else
        qDebug() << QStringLiteral("Errors on move %1:").arg(m_move).toUtf8().constData();
    for (const auto &error : m_errors) {
        qDebug() << error;
    }
}

void EngineChecker::handleClearData()
{
    bool signal = failed();
    m_move = -1;
    m_slots.clear();
    m_queue.clear();
    m_errors.clear();
    if (signal)
        emit failedChanged();
}

void EngineChecker::handleNewSlot(int id, const CardList &cards, int type, double x, double y,
                                  int expansionDepth, bool expandedDown, bool expandedRight)
{
    Q_UNUSED(type)
    Q_UNUSED(x)
    Q_UNUSED(y)
    Q_UNUSED(expansionDepth)
    Q_UNUSED(expandedDown)
    Q_UNUSED(expandedRight)

    m_slots.insert(id, cards);
    m_queue.addSlot(id);
}

void EngineChecker::handleGameStarted()
{
    m_move = 0;
}

void EngineChecker::handleAction(Engine::ActionTypeFlags action, int slot, int index, const CardData &data)
{
    Engine::ActionType type = Engine::actionType(action);
    if (type == Engine::MoveEndedAction)
        handleMoveEnded();
    else if (m_move < 0)
        handleImmediately(type, slot, index, data);
    else if (!(action & Engine::EngineActionFlag))
        m_queue.queue(type, slot, index, data);
}

void EngineChecker::handleImmediately(Engine::ActionType action, int slotId, int index, const CardData &data)
{
    if (!m_slots.contains(slotId)) {
        fail(Error::BadSlot, Action(action, slotId, index, data), CardData());
        return;
    }

    CardList &slot = m_slots[slotId];
    switch (action) {
    case Engine::InsertionAction:
        slot.insert(index, data);
        break;
    case Engine::RemovalAction:
        slot.removeAt(index);
        break;
    case Engine::FlippingAction:
        slot[index].show = data.show;
        break;
    case Engine::ClearingAction:
        slot.clear();
        break;
    default:
        break;
    }
}

void EngineChecker::handleMoveEnded()
{
    ++m_move;

    for (auto it = m_queue.begin(); it != m_queue.end(); ++it) {
        if (!handleQueued(*it)) {
            if ((*it).replaces)
                fail(Error::DoubleRequeue, *it, CardData());
            else
                it.requeue();
        }
    }

    m_queue.clear();

    emit queueFinished();
}

bool EngineChecker::handleQueued(const Action &action)
{
    bool handled = false;

    if (!m_slots.contains(action.slot)) {
        fail(Error::BadSlot, action, CardData());
        return true;
    }

    CardList &slot = m_slots[action.slot];
    switch (action.type) {
    case Engine::InsertionAction:
        {
            if (!action.replaces)
                m_queue.incrementQueued(action.slot, action.index);
            QSharedPointer<CardData> card = m_queue.take(action);
            if (card) {
                handled = true;
                card->show = action.data.show;
            }
            if (slot.count() < action.index) {
                fail(Error::BadIndex, action, card ? *card : CardData());
            } else {
                if (action.replaces)
                    slot.replace(action.index, card ? *card : CardData());
                else
                    slot.insert(action.index, card ? *card : CardData());
            }
            break;
        }
    case Engine::RemovalAction:
        {
            if (slot.count() < action.index) {
                fail(Error::BadIndex, action, CardData());
            } else {
                CardData card = slot.takeAt(action.index);
                m_queue.decrementQueued(action.slot, action.index, card);
                if (card) {
                    if (card.rank != action.data.rank || card.suit != action.data.suit)
                        fail(Error::WrongCard, action, card);
                    m_queue.store(QSharedPointer<CardData>::create(card));
                }
            }
            handled = true;
            break;
        }
    case Engine::FlippingAction:
        {
            if (slot.count() < action.index) {
                fail(Error::BadIndex, action, CardData());
            } else {
                const CardData &card = slot.at(action.index);
                if (card) {
                    if (card.rank != action.data.rank || card.suit != action.data.suit)
                        fail(Error::WrongCard, action, card);
                    slot[action.index].show = action.data.show;
                } else {
                    m_queue.flipQueued(action.slot, action.index, action.data);
                }
            }
            handled = true;
            break;
        }
    case Engine::ClearingAction:
        {
            CardList cards;
            m_queue.clearQueued(action.slot);
            cards.swap(slot);
            for (auto card : cards)
                m_queue.store(QSharedPointer<CardData>::create(card));
            handled = true;
            break;
        }
    default:
        break;
    }
    return handled;
}

void EngineChecker::fail(Error::Reason reason, const Action &action, const CardData &data)
{
    bool signal = !failed();
    m_errors.push_back(Error(reason, action, data));
    if (signal)
        emit failedChanged();
}

bool EngineChecker::failed() const
{
    return !m_errors.empty();
}

void EngineChecker::removeCards(int slot, const CardList &cards)
{
    if (!m_slots.contains(slot))
        fail(Error::BadSlot, Action(Engine::RemovalAction, slot, -1, cards.at(0)), CardData());

    if (m_slots[slot].count() < cards.count())
        fail(Error::MissingCards, Action(Engine::RemovalAction, slot, -1, cards.at(0)), CardData());

    int first = m_slots[slot].count() - cards.count();
    int count = cards.count();
    for (int i = 0; i < count; ++i) {
        CardData card = m_slots[slot].takeAt(first);
        m_queue.store(QSharedPointer<CardData>::create(card));
        if (card != cards.at(i))
            fail(Error::WrongCard, Action(Engine::RemovalAction, slot, first + i, cards.at(i)), card);
    }
}

EngineChecker::Error::Error(Reason reason, const Action &action, const CardData &data)
    : reason(reason)
    , action(action)
    , data(data)
{
}

QDebug operator<<(QDebug debug, const EngineChecker::Error &error)
{
    switch (error.reason) {
    case EngineChecker::Error::NoReason:
        debug.nospace() << "Invalid error for action: " << error.action;
        break;
    case EngineChecker::Error::WrongCard:
        debug.nospace() << "Wrong card taken for action: " << error.action << "," << error.data;
        break;
    case EngineChecker::Error::DoubleRequeue:
        debug.nospace() << "Could not complete action: " << error.action;
        break;
    case EngineChecker::Error::BadIndex:
        debug.nospace() << "Invalid index for action: " << error.action;
        break;
    case EngineChecker::Error::BadSlot:
        debug.nospace() << "Invalid slot index for action: " << error.action;
        break;
    case EngineChecker::Error::MissingCards:
        debug.nospace() << "Not enough cards to take from: " << error.action;
        break;
    }
    return debug.space();
}
