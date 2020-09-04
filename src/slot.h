#ifndef SLOT_H
#define SLOT_H

#include <QObject>
#include <QPointF>
#include "card.h"
#include "enginedata.h"

class Slot : public QObject
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

    Slot(int id, SlotType type, double x, double y, int expansionDepth,
         bool expandedDown, bool expandedRight, QObject *parent = nullptr);

    int id() const;
    QPointF position() const;
    int count() const;
    bool empty() const;

    void addCard(const CardData &card);
    void clear();

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
