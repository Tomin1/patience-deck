/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2020-2021 Tomi Leppänen
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
#include "enginedata.h"

class QMouseEvent;
class Table;
class Card;
class Slot;
class Drag : public QQuickItem
{
    Q_OBJECT

public:
    Drag(QMouseEvent *event, Table *table, Slot *slot, Card *card);
    ~Drag();

    Card *card() const;
    Slot *source() const;

    void update(QMouseEvent *event);
    void finish(QMouseEvent *event);
    void drop(Slot *slot);
    void cancel();

signals:
    void doDrag(quint32 id, int slotId, const CardList &cards);
    void doCancelDrag(quint32 id, int slotId, const CardList &cards);
    void doCheckDrop(quint32 id, int startSlotId, int endSlotId, const CardList &cards);
    void doDrop(quint32 id, int startSlotId, int endSlotId, const CardList &cards);
    void doClick(quint32 id, int slotId);
    void doDoubleClick(quint32 id, int slotId);

private slots:
    void handleCouldDrag(quint32 id, int slotId, bool could);
    void handleCouldDrop(quint32 id, int slotId, bool could);
    void handleDropped(quint32 id, int slotId, bool could);

private:
    enum DragState {
        NoDrag,
        AboutToDrag,
        StartingDrag,
        Dragging,
        Dropping,
        Dropped,
        Canceled,
        Clicked,
    };

    enum Droppability {
        Unknown = 0,
        Checking,
        CanDrop,
        CantDrop,
    };

    bool mayBeAClick(QMouseEvent *event);
    void checkTargets(bool force = false);
    void highlightOrDrop();
    static bool couldBeDoubleClick(const Card *card);

    static quint32 s_count;
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
    int m_target;
    QList<Slot *> m_targets;
    QList<Card *> m_cards;
    QHash<int, Droppability> m_couldDrop;
};

#endif // DRAG_H
