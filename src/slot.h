#ifndef SLOT_H
#define SLOT_H

#include <QElapsedTimer>
#include <QPointF>
#include <QtQuick/QQuickPaintedItem>
#include "card.h"
#include "enginedata.h"
#include "engine.h"

class Board;
class QPainter;
class Slot : public QQuickPaintedItem
{
    Q_OBJECT

public:
    enum ExpansionType {
        DoesNotExpand = 0x00,
        ExpandsInX = 0x01,
        ExpandsInY = 0x02,
        DeltaSet = 0x04,
        DeltaCalculated = 0x08
    };
    Q_ENUM(ExpansionType)
    Q_DECLARE_FLAGS(ExpansionTypes, ExpansionType)

    Slot(int id, const CardList &cards, SlotType type, double x, double y,
         int expansionDepth, bool expandedDown, bool expandedRight, Board *board);

    void paint(QPainter *painter);
    void updateDimensions();
    void updateLocations(Card *card = nullptr);

    int id() const;
    QPointF position() const;
    int count() const;
    bool empty() const;

    void appendCard(const CardData &card);
    void insertCard(int index, const CardData &card);
    void removeCard(int index);

    CardList asCardData(Card *first) const;
    QList<Card *> take(Card *first);
    void put(const QList<Card *> &cards);
    bool contains(Card *card) const;

    bool expanded() const;
    bool expandedRight() const;
    bool expandedDown() const;
    qreal delta(int index);
    void setDelta(double delta);
    int expansionDepth() const;
    int firstExpandedIndex() const;

    typedef QList<Card *>::const_iterator const_iterator;
    const_iterator constBegin() const;
    const_iterator constEnd() const;
    const_iterator constFind(Card *card) const;

    typedef QList<Card *>::iterator iterator;
    iterator begin();
    iterator end();
    iterator find(Card *card);

signals:
    void doClick(quint32 id, int slotId);

private:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    Board *m_board;
    int m_id;
    SlotType m_type;
    QList<Card *> m_cards;
    bool m_exposed;
    QPointF m_position;
    qreal m_expansionDelta;
    qreal m_calculatedDelta;
    ExpansionTypes m_expansion;
    int m_expansionDepth;
    QPen m_pen;

    QElapsedTimer m_timer;
    QPointF m_startPoint;
};

QDebug operator<<(QDebug debug, const Slot &slot);

#endif // SLOT_H
