#include "card.h"

namespace Id {

const auto Back = QStringLiteral("back");
const auto JokerBlack = QStringLiteral("joker_black");
const auto JokerRed = QStringLiteral("joker_red");
const auto Club = QStringLiteral("club_%1");
const auto Diamond = QStringLiteral("diamond_%1");
const auto Heart = QStringLiteral("heart_%1");
const auto Spade = QStringLiteral("spade_%1");
const auto Jack = QStringLiteral("jack");
const auto Queen = QStringLiteral("queen");
const auto King = QStringLiteral("king");

const QString &getSuitTemplate(Suit suit)
{
    switch (suit) {
    case SuitClubs:
        return Club;
    case SuitDiamonds:
        return Diamond;
    case SuitHeart:
        return Heart;
    case SuitSpade:
        return Spade;
    default:
        Q_UNREACHABLE();
        break;
    }
}

}; // Id

Card::Card(Suit suit, Rank rank, bool show, QObject *parent)
    : QObject(parent)
    , m_suit(suit)
    , m_rank(rank)
    , m_show(show)
{
}

Suit Card::suit() const
{
    return m_suit;
}

Rank Card::rank() const
{
    return m_rank;
}

bool Card::show() const
{
    return m_show;
}

bool Card::isBlack() const
{
    return m_suit == SuitClubs || m_suit == SuitSpade;
}

const QString Card::elementName() const
{
    if (!m_show) {
        return Id::Back;
    } else {
        int rank = m_rank;
        switch (m_rank) {
        case RankJoker:
            return isBlack() ? Id::JokerBlack : Id::JokerRed;
        case RankJack:
            return Id::getSuitTemplate(m_suit).arg(Id::Jack);
        case RankQueen:
            return Id::getSuitTemplate(m_suit).arg(Id::Queen);
        case RankKing:
            return Id::getSuitTemplate(m_suit).arg(Id::King);
        case RankAceHigh:
            rank = 1;
            // fallthrough
        default:
            return Id::getSuitTemplate(m_suit).arg(rank);
        }
    }
}
