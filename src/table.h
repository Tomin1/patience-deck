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

#ifndef TABLE_H
#define TABLE_H

#include <QColor>
#include <QImage>
#include <QMap>
#include <QPointF>
#include <QSizeF>
#include <QThread>
#include <QtQuick/QQuickItem>
#include "engine.h"
#include "enginedata.h"
#include "manager.h"
#include "slot.h"

class QSGTexture;
class QQuickWindow;
class Table : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(qreal minimumSideMargin READ minimumSideMargin
               WRITE setMinimumSideMargin NOTIFY minimumSideMarginChanged);
    Q_PROPERTY(qreal horizontalMargin READ horizontalMargin
               WRITE setHorizontalMargin NOTIFY horizontalMarginChanged);
    Q_PROPERTY(qreal maximumHorizontalMargin READ maximumHorizontalMargin
               WRITE setMaximumHorizontalMargin NOTIFY maximumHorizontalMarginChanged);
    Q_PROPERTY(qreal verticalMargin READ verticalMargin
               WRITE setVerticalMargin NOTIFY verticalMarginChanged);
    Q_PROPERTY(qreal maximumVerticalMargin READ maximumVerticalMargin
               WRITE setMaximumVerticalMargin NOTIFY maximumVerticalMarginChanged);
    Q_PROPERTY(QColor highlightColor READ highlightColor WRITE setHighlightColor
               RESET resetHighlightColor NOTIFY highlightColorChanged)

public:
    explicit Table(QQuickItem *parent = nullptr);
    ~Table();

    void updatePolish();
    QSGNode *getPaintNodeForSlot(Slot *slot);
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *);
    QSGTexture *cardTexture();

    qreal minimumSideMargin() const;
    void setMinimumSideMargin(qreal minimumSideMargin);
    qreal horizontalMargin() const;
    void setHorizontalMargin(qreal horizontalMargin);
    qreal maximumHorizontalMargin() const;
    void setMaximumHorizontalMargin(qreal maximumHorizontalMargin);
    qreal verticalMargin() const;
    void setVerticalMargin(qreal verticalMargin);
    qreal maximumVerticalMargin() const;
    void setMaximumVerticalMargin(qreal maximumVerticalMargin);
    QColor highlightColor() const;
    void setHighlightColor(QColor color);
    void resetHighlightColor();

    qreal sideMargin() const;
    QSizeF margin() const;
    QSizeF maximumMargin() const;
    QSizeF tableSize() const;
    QSizeF cardSize() const;
    QSizeF cardSizeInTexture() const;
    QSizeF cardSpace() const;
    QSizeF cardMargin() const;
    bool preparing() const;

    QList<Slot *> getSlotsFor(const Card *card, Slot *source);
    void highlight(Slot *slot);

    void addSlot(Slot *slot);
    Slot *slot(int id) const;
    void clear();
    void store(const QList<Card *> &cards);
    Drag *drag(QMouseEvent *event, Card *card);

    typedef QMap<int, Slot *>::key_iterator iterator;
    iterator begin();
    iterator end();

public slots:
    void setDirtyCardSize();

signals:
    void minimumSideMarginChanged();
    void horizontalMarginChanged();
    void maximumHorizontalMarginChanged();
    void verticalMarginChanged();
    void maximumVerticalMarginChanged();
    void highlightColorChanged();
    void highlightOpacityChanged();
    void cardTextureUpdated();

    void doClick(quint32 id, int slotId);
    void doRenderCardTexture(const QSize &size);

private slots:
    void handleCardTextureRendered(QImage image, const QSize &size);
    void handleSetExpansionToDown(int id, double expansion);
    void handleSetExpansionToRight(int id, double expansion);
    void handleSlotEmptied();
    void handleWidthChanged(double width);
    void handleHeightChanged(double height);
    void handleEngineFailure();

private:
    void updateCardSize();
    void updateIfNotPreparing();
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void setCardTexture(QSGTexture *texture);

    QMap<int, Slot *> m_slots;
    qreal m_minimumSideMargin;
    qreal m_sideMargin;
    QSizeF m_margin;
    QSizeF m_maximumMargin;
    QSizeF m_tableSize;
    QSizeF m_cardSize;
    QSizeF m_cardSizeInTexture;
    QSizeF m_cardSpace;
    QSizeF m_cardMargin;
    bool m_dirty;
    bool m_dirtyCardSize;

    Slot *m_highlightedSlot;
    QColor m_highlightColor;

    QElapsedTimer m_timer;
    QPointF m_startPoint;

    Manager m_manager;
    Drag *m_drag;

    QThread m_textureThread;
    QSGTexture *m_cardTexture;
    QImage m_cardImage;
};

QDebug operator<<(QDebug debug, const Table *table);

#endif // TABLE_H
