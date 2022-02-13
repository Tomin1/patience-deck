/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2022 Tomi Leppänen
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

#ifndef SELECTION_H
#define SELECTION_H

#include <QObject>
#include "countableid.h"
#include "enginedata.h"

class Card;
class Slot;
class Table;
class Selection : public QObject, public CountableId
{
    Q_OBJECT

public:
    Selection(Table *table, Slot *slot, Card *card);
    ~Selection();

    enum SelectionState {
        NoSelection,
        Selected,
        Dropping,
        Dropped,
        Finished,
        Canceled,
        Dead,
    };
    Q_ENUM(SelectionState)

    Slot *slot() const;
    Card *card() const;
    bool contains(const Card *card) const;
    SelectionState state() const;
    Selection *clone() const;

    void finish(Slot *slot);
    void cancel();

signals:
    void doDrag(quint32 id, int slotId, const CardList &cards);
    void doCancelDrag(quint32 id, int slotId, const CardList &cards);
    void doCheckDrop(quint32 id, int startSlotId, int endSlotId, const CardList &cards);
    void doDrop(quint32 id, int startSlotId, int endSlotId, const CardList &cards);
    void finished();

private slots:
    void handleCouldDrag(quint32 id, int slotId, bool could);
    void handleCouldDrop(quint32 id, int slotId, bool could);
    void handleDropped(quint32 id, int slotId, bool could);

private:
    friend QDebug operator<<(QDebug debug, const Selection *selection);

    explicit Selection(const Selection *other);
    void checkTarget(Slot *slot);
    void drop(Slot *slot);
    void done();
    static CardList toCardData(const QList<Card *> &cards, SelectionState state);

    SelectionState m_state;
    Table *m_table;
    Slot *m_source;
    Card *m_card;
    Slot *m_target;
    QList<Card *> m_cards;
};

#endif // SELECTION_H
