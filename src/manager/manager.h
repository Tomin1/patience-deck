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

#ifndef MANAGER_H
#define MANAGER_H

#include <QList>
#include <QObject>
#include <QPair>
#include "engine.h"
#include "enginedata.h"
#include "queue.h"

class Engine;
class Table;
class Slot;
class Card;
class Manager : public QObject
{
    Q_OBJECT
public:
    explicit Manager(Table *table);

    bool preparing() const;
    void store(const QList<Card *> &cards);

private slots:
    void handleNewSlot(int id, const CardList &cards, int type, double x, double y,
                       int expansionDepth, bool expandedDown, bool expandedRight);
    void handleAction(Engine::ActionTypeFlags action, int slotId, int index, const CardData &card);
    void handleImmediately(Engine::ActionType action, int slotId, int index, const CardData &card);
    void handleClearData();
    void handleGameStarted();
    void handleMoveEnded();

private:
    bool handleQueued(const Action &action);

    Engine *m_engine;
    Table *m_table;
    bool m_preparing;
    Queue<Card *> m_queue;
};

#endif // MANAGER_H
