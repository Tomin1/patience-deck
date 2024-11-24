/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2020-2021 Tomi Leppänen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ENGINEDATA_H
#define ENGINEDATA_H

#include <QList>
#include <QMetaEnum>
#include <QMetaType>
#include <random>

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
    BlackJoker = 52,
    RedJoker = 53,
    CardBack = 54,
    CardSlot = 55,
    CardsTotal = 56,
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

typedef QPair<Suit, Rank> SuitAndRank;

struct CardData {
    Suit suit;
    Rank rank;
    bool show;

    CardData();
    CardData(Suit suit, Rank rank, bool show);
    CardData(const CardData &other) = default;

    bool equalValue(const CardData &other) const;
    bool operator==(const CardData &other) const;
    bool operator!=(const CardData &other) const;
    operator bool() const;

    SuitAndRank value() const;

    friend QDebug operator<<(QDebug debug, const CardData &data);
    friend QDebug operator<<(QDebug debug, const CardData *data);
};

typedef QList<CardData> CardList;

Q_DECLARE_METATYPE(struct CardData)

Q_DECLARE_METATYPE(CardList)

#define NoOptionGroup 0

struct GameOption {
    QString displayName;
    uint group;
    uint index;
    bool set;

    bool isCheckOption() const {
        return group == NoOptionGroup;
    }

    bool isRadioOption() const {
        return group != NoOptionGroup;
    }
};

typedef QList<GameOption> GameOptionList;

Q_DECLARE_METATYPE(struct GameOption)

Q_DECLARE_METATYPE(GameOptionList)

class Seed {
public:
    static Seed randomSeed();
    static Seed fromString(const QString &str);

    Seed();
    Seed(const Seed &other);
    ~Seed();
    Seed &operator=(const Seed &other);
    Seed &operator=(Seed &&other);

    bool isValid() const;
    std::mt19937 getRng() const;
    QString toString() const;

private:
    typedef unsigned char tag_t;
    Seed(uint_fast32_t seed);
    Seed(quint32 *seed, int count);
    inline bool isScalar() const { return m_tag == 0 || !isValid(); }
    inline bool isVector() const { return !isScalar(); }

    friend QDebug operator<<(QDebug debug, const Seed &seed);

    union {
        uint_fast32_t m_scalar;
        quint32 *m_vector;
    };
    tag_t m_tag = 0;
};

Q_DECLARE_METATYPE(Seed)

#endif // ENGINEDATA_H
