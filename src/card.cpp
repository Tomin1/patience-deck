#include "card.h"

Card::Card(bool faceDown, Suit suit, Rank rank)
    : QObject(nullptr)
    , m_faceDown(faceDown)
    , m_suit(suit)
    , m_rank(rank)
{
}
