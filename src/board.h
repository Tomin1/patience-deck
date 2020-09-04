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

    void doCheckDrag(int slotId, const CardList &cards);
    void doCheckDrop(int startSlotId, int endSlotId, const CardList &cards);
    void doDrop(int startSlotId, int endSlotId, const CardList &cards);

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
