#include "card.h"
#include "slot.h"
#include "logging.h"

Slot::Slot(int id, SlotType type, double x, double y,
           int expansionDepth, bool expandedDown, bool expandedRight)
    : QObject(nullptr)
    , m_id(id)
    , m_type(type)
    , m_exposed(false)
    , m_x(x)
    , m_y(y)
    , m_expansionDelta(0.0)
    , m_expansion(expandedDown ? ExpandsInY : DoesNotExpand
                | expandedRight ? ExpandsInX : DoesNotExpand)
    , m_expansionDepth(expansionDepth)
    , m_needsUpdate(true)
{
}

void Slot::setCards(QList<QSharedPointer<Card>> cards)
{
    if (!cards.empty()) {
        // TODO: Could this be done more efficiently?
        if (cards.count() == m_cards.count()) {
            bool same = true;
            for (int i = 0; i < m_cards.count(); ++i) {
                if (cards[i] != m_cards[i]) {
                    same = false;
                    break;
                }
            }
            if (same)
                return;
        }
    }
    m_cards.swap(cards);
    qCDebug(lcEngine) << "Set" << m_cards.count() << "cards to slot";
    emit cardsChanged();
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
