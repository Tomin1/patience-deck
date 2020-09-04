#ifndef ENGINEDATA_H
#define ENGINEDATA_H

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

enum SlotType : int {
    UnknownSlot,
    ChooserSlot,
    FoundationSlot,
    ReserveSlot,
    StockSlot,
    TableauSlot,
    WasteSlot
};

enum Expansion : int {
    Full = -1,
    None = 0
};

struct CardData {
    Suit suit;
    Rank rank;
    bool show;
};

#endif // ENGINEDATA_H
