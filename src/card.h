#ifndef CARD_H
#define CARD_H

#include <libguile.h>
#include <QObject>

class Aisleriot;
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

    Card(SCM data, QObject *parent = nullptr);

    SCM toSCM() const;

    friend inline bool operator==(const Card &a, const Card &b);

private:
    bool faceDown;
    Suit suit;
    Rank rank;
};

inline bool operator==(const Card &a, const Card &b)
{
    return a.faceDown == b.faceDown && a.suit == b.suit && a.rank == b.rank;
}

#endif // CARD_H
