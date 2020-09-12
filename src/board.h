#ifndef BOARD_H
#define BOARD_H

#include <QMap>
#include <QPointF>
#include <QSizeF>
#include <QSvgRenderer>
#include <QtQuick/QQuickPaintedItem>
#include "engine.h"
#include "enginedata.h"
#include "slot.h"
#include "drag.h"

class QPainter;
class Board : public QQuickPaintedItem
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

public:
    explicit Board(QQuickItem *parent = nullptr);

    void paint(QPainter *painter);

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

    qreal sideMargin() const;
    QSizeF margin() const;
    QSizeF maximumMargin() const;
    QSizeF boardSize() const;
    QSizeF cardSize() const;
    QSizeF cardSpace() const;
    QSizeF cardMargin() const;
    bool preparing() const;

    QSvgRenderer *cardRenderer();

    void cardGrabbed(QMouseEvent *event, Slot *slot, Card *card);
    void cardMoved(QMouseEvent *event, Card *card);
    void cardReleased(QMouseEvent *event, Card *card);

signals:
    void minimumSideMarginChanged();
    void horizontalMarginChanged();
    void maximumHorizontalMarginChanged();
    void verticalMarginChanged();
    void maximumVerticalMarginChanged();

private slots:
    void handleNewSlot(int id, const CardList &cards, int type,
                       double x, double y, int expansionDepth,
                       bool expandedDown, bool expandedRight);
    void handleSetExpansionToDown(int id, double expansion);
    void handleSetExpansionToRight(int id, double expansion);
    void handleInsertCard(int slotId, int index, const CardData &card);
    void handleAppendCard(int slotId, const CardData &card);
    void handleRemoveCard(int slotId, int index);
    void handleClearData();
    void handleGameStarted();
    void handleWidthChanged(double width);
    void handleHeightChanged(double height);
    void updateCardSize();

private:
    Slot *getSlotAt(const QPointF &point, Slot *source);

    QMap<int, Slot *> m_slots;
    qreal m_minimumSideMargin;
    qreal m_sideMargin;
    QSizeF m_margin;
    QSizeF m_maximumMargin;
    QSizeF m_boardSize;
    QSizeF m_cardSize;
    QSizeF m_cardSpace;
    QSizeF m_cardMargin;
    QSvgRenderer m_cardRenderer;
    Drag *m_drag;
    bool m_preparing;
};

#endif // BOARD_H
