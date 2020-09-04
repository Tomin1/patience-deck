#ifndef SLOT_H
#define SLOT_H

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
    };
    Q_ENUM(ExpansionType)
    Q_DECLARE_FLAGS(ExpansionTypes, ExpansionType)

    Slot(int id, const CardList &cards, SlotType type, double x, double y,
         int expansionDepth, bool expandedDown, bool expandedRight, Board *parent = nullptr);

    void paint(QPainter *painter);

    int id() const;
    QPointF position() const;
    int count() const;
    bool empty() const;

    void appendCard(const CardData &card);
    void insertCard(int index, const CardData &card);
    void removeCard(int index);

    bool expanded() const;
    bool expandedRight() const;
    bool expandedDown() const;
    bool explicitDelta() const;
    qreal delta() const;
    void setDelta(double delta);
    int expansionDepth() const;

    typedef QList<Card *>::const_iterator const_iterator;
    const_iterator constBegin() const;
    const_iterator constEnd() const;

    typedef QList<Card *>::iterator iterator;
    iterator begin();
    iterator end();

signals:
    void cardsChanged();

private:
    Board *m_board;
    int m_id;
    SlotType m_type;
    QList<Card *> m_cards;
    bool m_exposed;
    QPointF m_position;
    qreal m_expansionDelta;
    ExpansionTypes m_expansion;
    int m_expansionDepth;
};

#endif // SLOT_H
