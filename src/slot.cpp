#include "board.h"
#include "card.h"
#include "slot.h"
#include "logging.h"

Slot::Slot(int id, const CardList &cards, SlotType type, double x, double y,
           int expansionDepth, bool expandedDown, bool expandedRight, Board *parent)
    : QQuickPaintedItem(parent)
    , m_board(parent)
    , m_id(id)
    , m_type(type)
    , m_exposed(false)
    , m_position(x, y)
    , m_expansionDelta(0.0)
    , m_expansion(expandedDown ? ExpandsInY : DoesNotExpand
                | expandedRight ? ExpandsInX : DoesNotExpand)
    , m_expansionDepth(expansionDepth)
{
    for (const CardData &card : cards)
        m_cards.append(new Card(card, this));
}

void Slot::paint(QPainter *painter)
{
    Q_UNUSED(painter) // TODO
}

int Slot::id() const
{
    return m_id;
}

QPointF Slot::position() const
{
    return m_position;
}

int Slot::count() const
{
    return m_cards.count();
}

bool Slot::empty() const
{
    return m_cards.empty();
}

void Slot::appendCard(const CardData &card)
{
    m_cards.append(new Card(card, this));
}

void Slot::insertCard(int index, const CardData &card)
{
    m_cards.insert(index, new Card(card, this));
}

void Slot::removeCard(int index)
{
    m_cards.removeAt(index);
}

bool Slot::expanded() const
{
    return m_expansion != DoesNotExpand;
}

bool Slot::expandedRight() const
{
    return m_expansion & ExpandsInX;
}

bool Slot::expandedDown() const
{
    return m_expansion & ExpandsInY;
}

bool Slot::explicitDelta() const
{
    return m_expansion & DeltaSet;
}

qreal Slot::delta() const
{
    return m_expansionDelta;
}

void Slot::setDelta(double delta)
{
    m_expansion |= DeltaSet;
    m_expansionDelta = delta;
}

int Slot::expansionDepth() const
{
    return m_expansionDepth;
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
