#include "card.h"

Card::Card(Suit suit, Rank rank, bool faceDown, QObject *parent)
    : QObject(parent)
    , m_suit(suit)
    , m_rank(rank)
    , m_faceDown(faceDown)
{
}
