#ifndef DRAG_H
#define DRAG_H

#include <QElapsedTimer>
#include <QObject>
#include <QPointF>
#include "enginedata.h"

class QMouseEvent;
class Table;
class Card;
class Slot;
class Drag : public QObject
{
    Q_OBJECT

public:
    Drag(QMouseEvent *event, Table *table, Slot *slot, Card *card);
    ~Drag();

    Card *card() const;
    Slot *source() const;

    void update(QMouseEvent *event);
    void finish(QMouseEvent *event);
    void cancel();

signals:
    void doDrag(quint32 id, int slotId, const CardList &cards);
    void doCancelDrag(quint32 id, int slotId, const CardList &cards);
    void doCheckDrop(quint32 id, int startSlotId, int endSlotId, const CardList &cards);
    void doDrop(quint32 id, int startSlotId, int endSlotId, const CardList &cards);
    void doClick(quint32 id, int slotId);

private slots:
    void handleCouldDrag(quint32 id, bool could);
    void handleCouldDrop(quint32 id, bool could);
    void handleDropped(quint32 id, bool could);

private:
    bool testClick(QMouseEvent *event);

    static quint32 s_count;
    quint32 m_id;
    QElapsedTimer m_timer;
    QPointF m_startPoint;
    QPointF m_lastPoint;
    bool m_mayBeAClick;
    bool m_completed;
    Table *m_table;
    Card *m_card;
    Slot *m_source;
    Slot *m_target;
    QList<Card *> m_cards;
};

#endif // DRAG_H
