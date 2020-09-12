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
    Drag(QMouseEvent *event, Board *board, Slot *slot, Card *card);
    ~Drag();

    Card *card() const;
    Slot *source() const;

    void update(QMouseEvent *event);
    void finish(Slot *target);
    void cancel();

signals:
    void doDrag(quint32 id, int slotId, const CardList &cards);
    void doCancelDrag(quint32 id, int slotId, const CardList &cards);
    void doCheckDrop(quint32 id, int startSlotId, int endSlotId, const CardList &cards);
    void doDrop(quint32 id, int startSlotId, int endSlotId, const CardList &cards);

private slots:
    void handleCouldDrag(quint32 id, bool could);
    void handleCouldDrop(quint32 id, bool could);
    void handleDropped(quint32 id, bool could);

private:
    static quint32 s_count;
    quint32 m_id;
    QPointF m_lastPoint;
    Board *m_board;
    Card *m_card;
    Slot *m_source;
    Slot *m_target;
    QList<Card *> m_cards;
};

#endif // DRAG_H
