/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2020-2022 Tomi Leppänen
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

#ifndef SLOT_H
#define SLOT_H

#include <QElapsedTimer>
#include <QPointF>
#include <QtQuick/QQuickItem>
#include "card.h"
#include "enginedata.h"
#include "engine.h"

class Table;
class Slot : public QQuickItem
{
    Q_OBJECT

public:
    enum ExpansionType {
        DoesNotExpand = 0x00,
        ExpandsInX = 0x01,
        ExpandsInY = 0x02,
        DeltaSet = 0x04
    };
    Q_ENUM(ExpansionType)
    Q_DECLARE_FLAGS(ExpansionTypes, ExpansionType)

    typedef QList<Card *>::const_iterator const_iterator;
    const_iterator constBegin() const;
    const_iterator constEnd() const;
    const_iterator constFind(const Card *card) const;

    typedef QList<Card *>::iterator iterator;
    iterator begin();
    iterator end();
    iterator find(const Card *card);
    const_iterator firstExpanded();

    Slot(int id, SlotType type, double x, double y, int expansionDepth,
         bool expandedDown, bool expandedRight, Table *table);

    void updateDimensions();
    void updateLocations();
    void updateLocations(iterator iter);

    int id() const;
    QPointF position() const;
    int count() const;
    bool isEmpty() const;
    bool highlighted(const Card *card = nullptr) const;

    void insert(int index, Card *card);
    void set(int index, Card *card);
    Card *takeAt(int index);
    QList<Card *> takeAll();
    void highlight(Card *card = nullptr);
    void removeHighlight();

    CardList asCardData(Card *first) const;
    QList<Card *> take(Card *first);
    void put(const QList<Card *> &cards);
    Card *top() const;
    bool contains(Card *card) const;
    using QQuickItem::contains;

    QRectF box() const;

    bool expanded() const;
    bool expandedRight() const;
    bool expandedDown() const;
    qreal delta(const const_iterator &iter);
    void setDelta(double delta);
    int expansionDepth() const;

signals:
    void slotEmptied();

private:
    bool reevaluateDelta();

    Table *m_table;
    int m_id;
    SlotType m_type;
    QList<Card *> m_cards;
    bool m_exposed;
    QPointF m_position;
    qreal m_expansionDelta;
    qreal m_calculatedDelta;
    ExpansionTypes m_expansion;
    int m_expansionDepth;
    const_iterator m_firstExpanded;
    bool m_firstExpandedValid;
    bool m_highlighted;
    Card *m_firstHighlightedCard;
};

QDebug operator<<(QDebug debug, const Slot *slot);

#endif // SLOT_H
