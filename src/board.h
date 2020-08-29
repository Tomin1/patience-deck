#ifndef BOARD_H
#define BOARD_H

#include <QHash>
#include <QtQuick/QQuickPaintedItem>
#include "engine.h"
#include "enginedata.h"
#include "slot.h"

class QPainter;
class Board : public QQuickPaintedItem
{
    Q_OBJECT

public:
    explicit Board(QQuickItem *parent = nullptr);

    void paint(QPainter *painter);

private slots:
    void handleNewSlot(int id, int type, double x, double y,
                       int expansionDepth, bool expandedDown, bool expandedRight);
    void handleSetExpansionToDown(int id, double expansion);
    void handleSetExpansionToRight(int id, double expansion);
    void handleClearSlot(int id);
    void handleNewCard(int slotId, int suit, int rank, bool faceDown);

private:
    QHash<int, Slot *> m_slots;
};

#endif // BOARD_H
