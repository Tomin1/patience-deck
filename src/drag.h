#ifndef DRAG_H
#define DRAG_H

#include <QObject>
#include <QPointF>
#include "enginedata.h"

class QMouseEvent;
class Board;
class Card;
class Slot;
class Drag : public QObject
{
    Q_OBJECT

public:
    Drag(QMouseEvent *event, Slot *slot, Card *card, Board *board);

    Card *card() const;
    Slot *source() const;

    void update(QMouseEvent *event);
    void finish(Slot *target);

signals:
    void doDrag(int slotId, const CardList &cards);
    void doCancelDrag(int slotId, const CardList &cards);
    void doCheckDrop(int startSlotId, int endSlotId, const CardList &cards);
    void doDrop(int startSlotId, int endSlotId, const CardList &cards);

private slots:
    void handleCouldDrag(bool could);
    void handleCouldDrop(bool could);
    void handleDropped(bool could);

private:
    void end();

    QPointF m_lastPoint;
    Board *m_board;
    Card *m_card;
    Slot *m_source;
    Slot *m_target;
    QList<Card *> m_cards;
};

#endif // DRAG_H
