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

#ifndef CARD_H
#define CARD_H

#include <QtQuick/QQuickItem>
#include "enginedata.h"
#include "drag.h"

class QQuickWindow;
class QSvgRenderer;
class QSGTexture;
class Table;
class Slot;
class Card : public QQuickItem
{
    Q_OBJECT

public:
    Card(const CardData &card, Table *table, Slot *slot);

    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *);

    QSizeF size() const;
    void setSize(const QSizeF &size);

    Suit suit() const;
    Rank rank() const;
    bool show() const;
    void setShow(bool show);
    bool isBlack() const;
    bool dragged() const;

    CardData data() const;

    bool operator==(const Card &other) const;

private:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    Slot *slot() const;
    bool highlighted() const;

    static QSvgRenderer *cardRenderer();
    static QSGTexture *cardTexture(const QSizeF &size, QQuickWindow *window);

    Table *m_table;
    CardData m_data;
    Drag *m_drag;
    bool m_dirty;
};

QDebug operator<<(QDebug debug, const Card &card);

#endif // CARD_H
