#include "card.h"
#include "slot.h"
#include "logging.h"

Slot::Slot(int id, SlotType type, double x, double y, int expansionDepth,
           bool expandedDown, bool expandedRight, QObject *parent)
    : QObject(parent)
    , m_id(id)
    , m_type(type)
    , m_exposed(false)
    , m_position(x, y)
    , m_expansionDelta(0.0)
    , m_expansion(expandedDown ? ExpandsInY : DoesNotExpand
                | expandedRight ? ExpandsInX : DoesNotExpand)
    , m_expansionDepth(expansionDepth)
{
}

int Slot::id() const
{
    return m_id;
}

QPointF Slot::position() const
{
    return m_position;
}

void Slot::addCard(Suit suit, Rank rank, bool faceDown)
{
    m_cards.append(new Card(suit, rank, faceDown, this));
}

void Slot::clear()
{
    m_cards.clear();
}

bool Slot::expandsRight() const
{
    return m_expansion & ExpandsInX;
}

bool Slot::expandedRight() const
{
    return m_expansion & ExpandedAtX;
}

void Slot::setExpansionToRight(double expansion)
{
    m_expansion |= ExpandedAtX;
    m_expansionDelta = expansion;
}

bool Slot::expandsDown() const
{
    return m_expansion & ExpandsInY;
}

bool Slot::expandedDown() const
{
    return m_expansion & ExpandedAtY;
}

void Slot::setExpansionToDown(double expansion)
{
    m_expansion |= ExpandedAtY;
    m_expansionDelta = expansion;
}

Slot::const_iterator Slot::constBegin() const
{
    return m_cards.constBegin();
}

Slot::const_iterator Slot::constEnd() const
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
