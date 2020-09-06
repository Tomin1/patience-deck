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

    friend inline bool operator==(const Card &a, const Card &b);

private:
    Board *m_board;
    Slot *m_slot;
    CardData m_data;
};

inline bool operator==(const Card &a, const Card &b)
{
    return a.m_data == b.m_data;
}

#endif // CARD_H
