/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2020-2024 Tomi Leppänen
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

#ifndef DRAG_H
#define DRAG_H

#include <QElapsedTimer>
#include <QPointF>
#include <QQuickItem>
#include "countableid.h"
#include "enginedata.h"

class Card;
class QMouseEvent;
class Selection;
class Slot;
class Table;
class Drag : public QQuickItem, public CountableId
{
    Q_OBJECT

public:
    Drag(QMouseEvent *event, Table *table, Card *card, Selection *selection = nullptr);
    ~Drag();

    Card *card() const;

    void abort();
    void update(QMouseEvent *event);
    void finish(QMouseEvent *event);

signals:
    void doDrag(quint32 id, int slotId, const CardList &cards);
    void doCancelDrag(quint32 id, int slotId, const CardList &cards);
    void doCheckDrop(quint32 id, int startSlotId, int endSlotId, const CardList &cards);
    void doDrop(quint32 id, int startSlotId, int endSlotId, const CardList &cards);
    void doClick(quint32 id, int slotId);
    void doDoubleClick(quint32 id, int slotId);
    void finished();

private slots:
    void handleCouldDrag(quint32 id, int slotId, bool could);
    void handleCouldDrop(quint32 id, int slotId, bool could);
    void handleDropped(quint32 id, int slotId, bool could);
    void handleClicked(quint32 id, int slotId, bool could);
    void handleDoubleClicked(quint32 id, int slotId, bool could);

private:
    enum DragState {
        WaitingForSelection = -1,
        NoDrag = 0,
        AboutToDrag,
        StartingDrag,
        Dragging,
        Dropping,
        Dropped,
        AnimatingBack,
        Finished,
        Canceled,
        Clicked,
    };

    enum Droppability {
        Unknown = 0,
        Checking,
        CanDrop,
        CantDrop,
    };

    friend QDebug operator<<(QDebug debug, const Drag *drag);

    bool mayBeAClick(QMouseEvent *event);
    void checkTargets(bool force = false);
    void highlightOrDrop();
    void drop(Slot *slot);
    void move(const QPointF point);
    void cancel();
    void done();
    Card *firstCard() const;
    static bool couldBeDoubleClick(const Card *card);
    static CardList toCardData(const QList<Card *> &cards, DragState state);

    static QElapsedTimer s_doubleClickTimer;
    static const Card *s_lastCard;

    DragState m_state;
    quint32 m_id;
    QElapsedTimer m_timer;
    QPointF m_startPoint;
    QPointF m_lastPoint;
    bool m_mayBeADoubleClick;
    Table *m_table;
    Card *m_card;
    Slot *m_source;
    Selection *m_selection;
    int m_target;
    QList<Slot *> m_targets;
    QList<Card *> m_cards;
    QHash<int, Droppability> m_couldDrop;
};

#endif // DRAG_H
