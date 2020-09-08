#ifndef CARD_H
#define CARD_H

#include <QtQuick/QQuickPaintedItem>
#include <QPen>
#include "enginedata.h"

class QPainter;
class Board;
class Slot;
class Card : public QQuickPaintedItem
{
    Q_OBJECT

public:
    Card(const CardData &card, Board *board, Slot *slot);

    void paint(QPainter *painter);

    Suit suit() const;
    Rank rank() const;
    bool show() const;
    bool isBlack() const;
    const QString elementName() const;

    bool operator==(const Card &other) const;

private:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    Board *m_board;
    Slot *m_slot;
    CardData m_data;

    // Temporary
    bool m_dragged;
    QPointF m_dragLast;
};

QDebug operator<<(QDebug debug, const Card &card);

#endif // CARD_H
