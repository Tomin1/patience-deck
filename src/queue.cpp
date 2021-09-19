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

#include "logging.h"
#include "queue.h"

#ifndef ENGINE_EXERCISER
#include "card.h"
template class Queue<Card *>;
#else
#include <QSharedPointer>
#include "enginedata.h"
template class Queue<QSharedPointer<CardData>>;
#endif

template<class C>
void Queue<C>::addSlot(int slot)
{
    m_laterActions.insert(slot, std::list<Action>());
}

template<class C>
void Queue<C>::clear()
{
    m_actions.clear();
    m_laterActions.clear();
}

template<class C>
int Queue<C>::actionCount()
{
    int count = m_actions.size();
    for (const auto &list : m_laterActions)
        count += list.size();
    return count;
}

template<class C>
int Queue<C>::cardCount()
{
    return m_cards.count();
}

template<class C>
void Queue<C>::queue(Engine::ActionType type, int slot, int index, const CardData &data)
{
    Action action(type, slot, index, data);
    qCDebug(lcQueue) << "Queueing" << action;
    m_actions.push_back(action);
}

template<class C>
void Queue<C>::requeue(Action action)
{
    qCDebug(lcQueue) << "Queueing again" << action;
    m_laterActions[action.slot].push_back(action);
}

template<class C>
void Queue<C>::store(C card)
{
    qCDebug(lcQueue) << "Storing" << *card;
    m_cards.insert(card->value(), card);
}

template<class C>
C Queue<C>::take(const Action &action)
{
    return m_cards.take(action.value());
}

template<class C>
QList<C> Queue<C>::takeAll()
{
    QList<C> cards;
    for (auto card : m_cards)
        cards.append(card);
    m_cards.clear();
    return cards;
}

template<class C>
typename Queue<C>::iterator Queue<C>::begin()
{
    return Queue<C>::iterator(this);
}

template<class C>
typename Queue<C>::iterator Queue<C>::end()
{
    return Queue<C>::iterator(this, true);
}

template<class C>
void Queue<C>::incrementQueued(int slot, int index)
{
    for (auto &action : m_laterActions[slot]) {
        if (action.index >= index)
            action.index++;
    }
}

template<class C>
void Queue<C>::decrementQueued(int slot, int index)
{
    for (auto it = m_laterActions[slot].begin(); it != m_laterActions[slot].end(); ++it) {
        if (it->index == index)
            it = m_laterActions[slot].erase(it);
        else if (it->index > index)
            it->index--;
    }
}

template<class C>
void Queue<C>::flipQueued(int slot, int index, const CardData &data)
{
    for (auto &action : m_laterActions[slot]) {
        if (action.index == index) {
            action.data.show = data.show;
            if (action.data.rank != data.rank || action.data.suit != data.suit)
                qCCritical(lcQueue) << "Rank or suit doesn't match to" << data
                                    << "for queued" << action.data << "in slot" << slot
                                    << "at index" << index;
            break;
        }
    }
}

template<class C>
void Queue<C>::clearQueued(int slot)
{
    m_laterActions[slot].clear();
}

template<class C>
Queue<C>::iterator::iterator(Queue<C> *queue, bool atEnd)
    : queue(queue)
    , state(atEnd ? EndState : BeginState)
{
    ++(*this); // Move forward from BeginState
}

template<class C>
void Queue<C>::iterator::operator=(const iterator& other)
{
    queue = other.queue;
    state = other.state;
    iter = other.iter;
    slotIter = other.slotIter;
}

template<class C>
bool Queue<C>::iterator::operator!=(const iterator& other) const
{
    if (queue != other.queue)
        return true;

    if (state != other.state)
        return true;

    switch (state) {
    case BeginState:
    case EndState:
        return false;
    case IterLaterState:
        if (slotIter != other.slotIter)
            return false;
        [[fallthrough]];
    case IterActionsState:
        return iter != other.iter; 
    default:
        Q_UNREACHABLE();
        break;
    }
}

template<class C>
typename Queue<C>::iterator &Queue<C>::iterator::operator++()
{
    bool first = false;
    if (state == BeginState) {
        iter = queue->m_actions.begin();
        state = IterActionsState;
        first = true;
    }
    if (state != EndState) {
        if (state == IterActionsState) {
            if (!first)
                ++iter;
            // Iterating m_actions
            if (iter == queue->m_actions.end()) {
                slotIter = queue->m_laterActions.begin();
                state = slotIter == queue->m_laterActions.end() ? EndState : IterLaterState;
                first = true;
            }
        }
        if (state == IterLaterState) {
            // Iterating m_laterActions
            if (first)
                iter = slotIter->begin();
            else
                ++iter;

            while (iter == slotIter->end()) {
                ++slotIter;
                if (slotIter == queue->m_laterActions.end()) {
                    qCDebug(lcQueue) << "Iterator reached end";
                    state = EndState;
                    break;
                }
                iter = slotIter->begin();
            }
        }
    }
    return *this;
}

template<class C>
Action &Queue<C>::iterator::operator*() const
{
    if (state == BeginState || state == EndState)
        qCCritical(lcQueue) << "Deref queue iterator at" << (state == BeginState ? "beginning" : "end");
    return *iter;
}

template<class C>
bool Queue<C>::iterator::requeue()
{
    if (state != IterActionsState) {
        qCWarning(lcQueue) << "Trying to queue an action while in state" << state;
        qCCritical(lcQueue) << "Discarding" << *iter;
        return false;
    }

    if (iter->type != Engine::InsertionAction) {
        qCWarning(lcQueue) << "Trying to queue non-insertion action";
        qCCritical(lcQueue) << "Discarding" << *iter;
        return false;
    }

    iter->replaces = true;
    queue->requeue(*iter);
    return true;
}

Action::Action(Engine::ActionType type, int slot, int index, const CardData &data)
    : type(type)
    , slot(slot)
    , index(index)
    , data(data)
    , replaces(false)
{
}

SuitAndRank Action::value() const
{
    return SuitAndRank(data.suit, data.rank);
}

QDebug operator<<(QDebug debug, const Action &action)
{
    switch (action.type) {
    case Engine::InsertionAction:
        debug.nospace() << "insertion of " << action.data << "to slot" << action.slot << "at" << action.index;
        break;
    case Engine::RemovalAction:
        debug.nospace() << "removal of " << action.data << "from slot" << action.slot << "at" << action.index;
        break;
    case Engine::FlippingAction:
        debug.nospace() << "flipping of " << action.data << "in slot" << action.slot << "at" << action.index;
        break;
    case Engine::ClearingAction:
        debug.nospace() << "clearing slot " << action.slot;
    }
    return debug.space();
}
