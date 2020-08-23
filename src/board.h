#ifndef BOARD_H
#define BOARD_H

#include <QtQuick/QQuickPaintedItem>

class QPainter;
class Board : public QQuickPaintedItem
{
    Q_OBJECT

public:
    explicit Board(QQuickItem *parent = nullptr);

    void paint(QPainter *painter);
};

#endif // BOARD_H
