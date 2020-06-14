#include "card.h"
#include "slot.h"

Slot::Slot(int id, SlotType type, double x, double y,
           int expansionDepth, bool expandedDown, bool expandedRight,
           QObject *parent)
    : QObject(parent)
    , m_id(id)
    , m_type(type)
    , m_exposed(false)
    , m_x(x)
    , m_y(y)
    , m_expansion(0.0)
    , m_expansionDepth(expansionDepth)
    , m_expandedDown(expandedDown)
    , m_expandedRight(expandedRight)
    , m_needsUpdate(true)
{
}

void Slot::setCards(SCM cards)
{
    // mimics aisleriot/src/game.c:cscmi_slot_set_cards
    QList <Card*> newCards;
    if (scm_is_true(scm_list_p(cards))) {
        for (SCM it = cards; it != SCM_EOL; it = SCM_CDR(it)) {
            Card *card = new Card(it, this);
            newCards.append(card);
        }
        // TODO: Could this be done more efficiently?
        if (newCards.count() == m_cards.count()) {
            bool same = true;
            for (int i = 0; i < m_cards.count(); ++i) {
                if (newCards[i] != m_cards[i]) {
                    same = false;
                    break;
                }
            }
            if (same)
                return;
        }
    }
    m_cards.swap(newCards);
    for (int i = 0; i < newCards.count(); i++) {
        // TODO: Use smart pointers instead!
        delete newCards[i];
    }
    // TODO: Emit signal
}
