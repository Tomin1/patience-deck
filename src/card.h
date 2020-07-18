#ifndef CARD_H
#define CARD_H

#include <QObject>

class Card : public QObject
{
    Q_OBJECT

public:
    enum Rank : int {
        RankJoker = 0,
        RankAce = 1,
        RankTwo = 2,
        RankThree = 3,
        RankFour = 4,
        RankFive = 5,
        RankSix = 6,
        RankSeven = 7,
        RankEight = 8,
        RankNine = 9,
        RankTen = 10,
        RankJack = 11,
        RankQueen = 12,
        RankKing = 13,
        RankAceHigh = 14,

        CardBack = 54,
        CardSlot = 55,
        CardsTotal = 56,
    };

    enum Joker {
        BlackJoker = 52,
        RedJoker = 53,
    };

    enum Suit : int {
        SuitClubs = 0,
        SuitDiamonds = 1,
        SuitHeart = 2,
        SuitSpade = 3
    };

    Card(bool faceDown, Suit suit, Rank rank);

    bool faceDown() { return m_faceDown; };
    Suit suit() { return m_suit; };
    Rank rank() { return m_rank; };

    friend inline bool operator==(const Card &a, const Card &b);

private:
    bool m_faceDown;
    Suit m_suit;
    Rank m_rank;
};

inline bool operator==(const Card &a, const Card &b)
{
    return a.m_faceDown == b.m_faceDown && a.m_suit == b.m_suit && a.m_rank == b.m_rank;
}

#endif // CARD_H
