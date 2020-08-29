#ifndef SLOT_H
#define SLOT_H

#include <QObject>
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
        ExpandedAtX = 0x04,
        ExpandedAtY = 0x08,
    };
    Q_ENUM(ExpansionType)
    Q_DECLARE_FLAGS(ExpansionTypes, ExpansionType)

    Slot(int id, SlotType type, double x, double y, int expansionDepth,
         bool expandedDown, bool expandedRight, QObject *parent = nullptr);

    void addCard(Suit suit, Rank rank, bool faceDown);
    void clear();

    bool expandsRight();
    bool expandedRight();
    void setExpansionToRight(double expansion);
    bool expandsDown();
    bool expandedDown();
    void setExpansionToDown(double expansion);

    typedef QList<Card *>::const_iterator const_iterator;
    const_iterator constBegin();
    const_iterator constEnd();

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
    double m_x;
    double m_y;
    double m_expansionDelta;
    ExpansionTypes m_expansion;
    int m_expansionDepth;
};

#endif // SLOT_H
