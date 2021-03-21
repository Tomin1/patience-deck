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

#ifndef ENGINERELAY_H
#define ENGINERELAY_H

#include <QLinkedList>
#include <QList>
#include <QObject>
#include <QPair>
#include "engine.h"
#include "enginedata.h"

class Engine;
class Table;
class Slot;
class Card;
class Manager : public QObject
{
    Q_OBJECT
public:
    explicit Manager(Table *table);

    operator QString() const;

    bool preparing() const;
    void store(const QList<Card *> &cards);

private slots:
    void handleNewSlot(int id, const CardList &cards, int type, double x, double y,
                       int expansionDepth, bool expandedDown, bool expandedRight);
    void handleInsertCard(int slotId, int index, const CardData &card);
    void handleAppendCard(int slotId, const CardData &card);
    void handleRemoveCard(int slotId, int index, const CardData &card);
    void handleFlipCard(int slotId, int index, const CardData &card);
    void handleClearSlot(int slotId);
    void handleClearData();
    void handleGameStarted();
    void handleMoveEnded();

private:
    typedef QPair<Suit, Rank> SuitAndRank;

    struct Action {
        enum ActionType {
            InsertionAction,
            RemovalAction,
            FlipAction,
            ClearAction,
        };

        ActionType type;
        int slot;
        int index;
        CardData data;

        Action(ActionType type, int slot, int index, const CardData &data);

        SuitAndRank suitAndRank() const;

        operator QString() const;
    };

    void store(Card *card);
    void queue(Action::ActionType type, int slotId, int index, const CardData &data);
    const Action *nextAction(int slot) const;
    void discardAction(int slot);
    void dequeue();
    bool handle(Slot *slot, const Action *action);

    Engine *m_engine;
    Table *m_table;
    bool m_preparing;
    QHash<SuitAndRank, Card *> m_cards;
    QHash<int, QLinkedList<Action>> m_actions;
};

#endif // ENGINERELAY_H
