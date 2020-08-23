#include <QBrush>
#include <QColor>
#include <QPainter>
#include "board.h"

Board::Board(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    // Fill the scene with suitable color, nothing behind this is displayed
    setOpaquePainting(true);
    QColor backgroundColor(Qt::darkGreen);
    setFillColor(backgroundColor);
}

void Board::paint(QPainter *painter)
{
    Q_UNUSED(painter) // TODO
}
