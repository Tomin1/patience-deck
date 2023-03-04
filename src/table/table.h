/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2020-2023 Tomi Leppänen
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
#include <QPointF>
#include <QSizeF>
#include <QThread>
#include <QtQuick/QQuickItem>
#include <QVector>
#include "countableid.h"
#include "engine.h"
#include "enginedata.h"
#include "manager.h"
#include "slot.h"

class FeedbackEventAttachedType;
class QAnimationGroup;
class QCommandLineParser;
class QSGTexture;
class QQuickWindow;
class SlotNode;
class Table : public QQuickItem, public CountableId
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
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor
               RESET resetBackgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(QColor highlightColor READ highlightColor WRITE setHighlightColor
               RESET resetHighlightColor NOTIFY highlightColorChanged)
    Q_PROPERTY(bool doubleResolution READ doubleResolution
               WRITE setDoubleResolution NOTIFY doubleResolutionChanged)
    Q_PROPERTY(bool animationPlaying READ animationPlaying NOTIFY animationPlayingChanged)
    Q_PROPERTY(bool animationPaused READ animationPaused
               WRITE setAnimationPaused NOTIFY animationPausedChanged)

public:
    explicit Table(QQuickItem *parent = nullptr);
    ~Table();

    static void addArguments(QCommandLineParser *parser);
    static void setArguments(QCommandLineParser *parser);

    Q_INVOKABLE void playWinAnimation();
    void stopAnimation();

    void updatePolish();
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *);
    QSGTexture *cardTexture();
    bool textureIsDoubleSize() const;

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
    QColor backgroundColor() const;
    void setBackgroundColor(QColor color);
    void resetBackgroundColor();
    bool transparentBackground() const;
    bool doubleResolution() const;
    void setDoubleResolution(bool doubleResolution);
    bool animationPlaying() const;
    bool animationPaused() const;
    void setAnimationPaused(bool paused);

    qreal sideMargin() const;
    QSizeF margin() const;
    QSizeF maximumMargin() const;
    QSizeF tableSize() const;
    QSizeF cardSize() const;
    QSizeF cardSizeInTexture() const;
    QSizeF cardSpace() const;
    QSizeF cardMargin() const;
    bool preparing() const;

    QList<Slot *> getSlotsFor(const Card *card, const QList<Card *> cards, Slot *source);
    void highlight(Slot *slot, Card *card = nullptr);
    FeedbackEventAttachedType *feedback();

    void addSlot(Slot *slot);
    Slot *slot(int id) const;
    void clear();
    void store(const QList<Card *> &cards);
    Drag *drag(QMouseEvent *event, Card *card);
    void select(Card *card);
    Q_INVOKABLE void unselect();

    void setDirtyCardSize();
    void disableActions(bool disabled);

    typedef QVector<Slot *>::iterator iterator;
    iterator begin();
    iterator end();

    enum Dirty {
        Clean = 0x00,
        BackgroundType = 0x01,
        BackgroundColor = 0x02,
        BackgroundSize = 0x04,
        HighlightedSlot = 0x08,
        HighlightColor = 0x10,
        SlotSize = 0x20,
        SlotCount = 0x40,
        HiddenSlots = 0x80,
        Filthy = 0xff
    };
    Q_DECLARE_FLAGS(DirtyFlags, Dirty)

signals:
    void minimumSideMarginChanged();
    void horizontalMarginChanged();
    void maximumHorizontalMarginChanged();
    void verticalMarginChanged();
    void maximumVerticalMarginChanged();
    void backgroundColorChanged();
    void doubleResolutionChanged();
    void animationPlayingChanged();
    void animationPausedChanged();
    void highlightColorChanged();
    void cardTextureUpdated();
    void actionsDisabled(bool disabled);

    void doClick(quint32 id, int slotId);
    void doRenderCardTexture(const QSize &size, bool drawDoubleSize);

private slots:
    void connectWindowSignals(QQuickWindow *window);
    void createCardTexture();
    void swapCardTexture();
    void handleCardTextureRendered(QImage image, const QSize &size);
    void handleDoubleSizeTextureRendered(QImage image, const QSize &size);
    void handleSizeChanged();
    void handleSceneGraphInvalidated();
    void handleSetExpansionToDown(int id, double expansion);
    void handleSetExpansionToRight(int id, double expansion);
    void handleSlotEmptied();
    void handleWidthChanged(double width);
    void handleHeightChanged(double height);
    void handleEngineFailure();
    void handleGameContinued();

private:
    void updateCardSize();
    void updateIfNotPreparing();
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void setCardTexture(QSGTexture *texture);
    void setPendingCardTexture(QSGTexture *texture);
    void createWinAnimation();

    static QRectF getSlotOutline(Slot *slot);
    static void setMaterialForSlotNode(SlotNode *node);
    static void setGeometryForSlotNode(SlotNode *node, Slot *slot);
    void setHighlightForSlotNode(SlotNode *node, Slot *slot);
    QRectF getBoundingRect(const QList<Card *> &cards);
    QList<Slot *> getSlotsFor(const QRectF &rect, Slot *source);
    Slot *findSlotAtPoint(const QPointF point) const;
    QSize size() const;

    QVector<Slot *> m_slots;
    qreal m_minimumSideMargin;
    qreal m_sideMargin;
    QSizeF m_margin;
    QSizeF m_maximumMargin;
    QSizeF m_tableSize;
    QSizeF m_cardSize;
    QSizeF m_cardSizeInTexture;
    QSizeF m_cardSpace;
    QSizeF m_cardMargin;
    DirtyFlags m_dirty;
    bool m_dirtyCardSize;
    QColor m_backgroundColor;
    bool m_doubleResolution;

    Slot *m_highlightedSlot;
    QColor m_highlightColor;

    QElapsedTimer m_timer;
    QPointF m_startPoint;

    Manager m_manager;
    QObject *m_interaction;

    QThread m_textureThread;
    QSGTexture *m_cardTexture;
    QSGTexture *m_pendingCardTexture;
    QImage m_cardImage;
    QImage m_doubleSizeImage;
    QQuickWindow *m_previousWindow;
    QAnimationGroup *m_animation;
    bool m_animate;
};

QDebug operator<<(QDebug debug, const Table *table);

Q_DECLARE_OPERATORS_FOR_FLAGS(Table::DirtyFlags);

#endif // TABLE_H
