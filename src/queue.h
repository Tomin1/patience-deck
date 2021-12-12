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

#ifndef QUEUE_H
#define QUEUE_H

#include <list>
#include <QDebug>
#include "engine.h"
#include "enginedata.h"

struct Action {
    Engine::ActionType type;
    int slot;
    int index;
    CardData data;
    bool replaces;

    Action(Engine::ActionType type, int slot, int index, const CardData &data);

    SuitAndRank value() const;

    friend QDebug operator<<(QDebug debug, const Action &action);
};

template<class C>
class Queue
{
public:
    void addSlot(int slot);
    void clear();
    int actionCount();
    int cardCount();

    class iterator {
        Queue *queue;
        typename std::list<Action>::iterator iter;
        typename QMap<int, std::list<Action>>::iterator slotIter;

        enum QueueIteratorState {
            BeginState,
            IterActionsState,
            IterLaterState,
            EndState
        } state;

    public:
        iterator(Queue *queue, bool atEnd = false);

        void operator=(const iterator& other);
        bool operator!=(const iterator& other) const;
        iterator &operator++();
        Action &operator*() const;

        bool requeue();
    };

    typename Queue<C>::iterator begin();
    typename Queue<C>::iterator end();

    void queue(Engine::ActionType type, int slot, int index, const CardData &data);
    void requeue(Action action);

    void store(C card);
    C take(const Action &action);
    QList<C> takeAll();

    void incrementQueued(int slot, int index);
    void decrementQueued(int slot, int index, const CardData &data);
    void flipQueued(int slot, int index, const CardData &data);
    void clearQueued(int slot);

    typename QSet<C>::iterator beginRecent();
    typename QSet<C>::iterator endRecent();

private:
    std::list<Action> m_actions;
    QMap<int, std::list<Action>> m_laterActions;
    QMultiHash<SuitAndRank, C> m_cards;
    QSet<C> m_recentlyAdded;
};

#endif // QUEUE_H
