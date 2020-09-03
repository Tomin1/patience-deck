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

class QPainter;
class Board : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(qreal horizontalMargin READ horizontalMargin WRITE setHorizontalMargin NOTIFY horizontalMarginChanged);
    Q_PROPERTY(qreal verticalMargin READ verticalMargin WRITE setVerticalMargin NOTIFY verticalMarginChanged);

public:
    explicit Board(QQuickItem *parent = nullptr);

    void paint(QPainter *painter);

    qreal horizontalMargin() const;
    void setHorizontalMargin(qreal horizontalMargin);
    qreal verticalMargin() const;
    void setVerticalMargin(qreal verticalMargin);

signals:
    void horizontalMarginChanged();
    void verticalMarginChanged();

private slots:
    void handleNewSlot(int id, int type, double x, double y,
                       int expansionDepth, bool expandedDown, bool expandedRight);
    void handleSetExpansionToDown(int id, double expansion);
    void handleSetExpansionToRight(int id, double expansion);
    void handleClearSlot(int id);
    void handleNewCard(int slotId, int suit, int rank, bool show);
    void handleClearData();
    void handleWidthChanged(double width);
    void handleHeightChanged(double height);
    void updateCardSize();

private:
    bool readyToPaint();
    QPointF getPoint(const QPointF &position) const;
    qreal getExpansion(Slot *slot) const;

    QMap<int, Slot *> m_slots;
    QSizeF m_margin;
    QSizeF m_boardSize;
    QSizeF m_cardSize;
    QSizeF m_cardSpace;
    QSizeF m_cardMargin;
    QSvgRenderer m_cardRenderer;
};

#endif // BOARD_H
