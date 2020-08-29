#include "card.h"
#include "slot.h"
#include "logging.h"

Slot::Slot(int id, SlotType type, double x, double y, int expansionDepth,
           bool expandedDown, bool expandedRight, QObject *parent)
    : QObject(parent)
    , m_id(id)
    , m_type(type)
    , m_exposed(false)
    , m_x(x)
    , m_y(y)
    , m_expansionDelta(0.0)
    , m_expansion(expandedDown ? ExpandsInY : DoesNotExpand
                | expandedRight ? ExpandsInX : DoesNotExpand)
    , m_expansionDepth(expansionDepth)
{
}

void Slot::addCard(Suit suit, Rank rank, bool faceDown)
{
    m_cards.append(new Card(suit, rank, faceDown, this));
}

void Slot::clear()
{
    m_cards.clear();
}

bool Slot::expandsRight()
{
    return m_expansion & ExpandsInX;
}

bool Slot::expandedRight()
{
    return m_expansion & ExpandedAtX;
}

void Slot::setExpansionToRight(double expansion)
{
    m_expansion |= ExpandedAtX;
    m_expansionDelta = expansion;
}

bool Slot::expandsDown()
{
    return m_expansion & ExpandsInY;
}

bool Slot::expandedDown()
{
    return m_expansion & ExpandedAtY;
}

void Slot::setExpansionToDown(double expansion)
{
    m_expansion |= ExpandedAtY;
    m_expansionDelta = expansion;
}

Slot::const_iterator Slot::constBegin()
{
    return m_cards.constBegin();
}

Slot::const_iterator Slot::constEnd()
{
    return m_cards.constEnd();
}

Slot::iterator Slot::begin()
{
    return m_cards.begin();
}

Slot::iterator Slot::end()
{
    return m_cards.end();
}
