#include <QGuiApplication>
#include <QPainter>
#include <QScreen>
#include "board.h"
#include "card.h"
#include "logging.h"
#include "slot.h"

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

namespace {

QPointF mapPos(const QPointF &pos)
{
    switch (QGuiApplication::primaryScreen()->orientation()) {
    case Qt::LandscapeOrientation:
        return QPointF(pos.y(), -pos.x());
    case Qt::InvertedLandscapeOrientation:
        return QPointF(-pos.y(), pos.x());
    default:
        return QPointF(pos.x(), pos.y());
    };
}

}; // namespace

Card::Card(const CardData &card, Board *board, Slot *slot)
    : QQuickPaintedItem(slot)
    , m_board(board)
    , m_slot(slot)
    , m_data(card)
    , m_dragged(false)
{
    setAcceptedMouseButtons(Qt::LeftButton);
}

void Card::paint(QPainter *painter)
{
    m_board->cardRenderer()->render(painter, elementName());
}

Suit Card::suit() const
{
    return m_data.suit;
}

Rank Card::rank() const
{
    return m_data.rank;
}

bool Card::show() const
{
    return m_data.show;
}

bool Card::isBlack() const
{
    return m_data.suit == SuitClubs || m_data.suit == SuitSpade;
}

const QString Card::elementName() const
{
    if (!m_data.show) {
        return Id::Back;
    } else {
        int rank = m_data.rank;
        switch (m_data.rank) {
        case RankJoker:
            return isBlack() ? Id::JokerBlack : Id::JokerRed;
        case RankJack:
            return Id::getSuitTemplate(m_data.suit).arg(Id::Jack);
        case RankQueen:
            return Id::getSuitTemplate(m_data.suit).arg(Id::Queen);
        case RankKing:
            return Id::getSuitTemplate(m_data.suit).arg(Id::King);
        case RankAceHigh:
            rank = 1;
            // fallthrough
        default:
            return Id::getSuitTemplate(m_data.suit).arg(rank);
        }
    }
}

bool Card::operator==(const Card &other) const
{
    return m_data == other.m_data;
}

void Card::mousePressEvent(QMouseEvent *event)
{
    qCDebug(lcMouse) << event << "for" << *this;
    setKeepMouseGrab(true);
    m_dragLast = mapPos(event->screenPos());
    m_board->doCheckDrag(m_slot->id(), CardList() << m_data);
    // TODO: connect to couldDrag and allow/deny dragging
}

void Card::mouseReleaseEvent(QMouseEvent *event)
{
    qCDebug(lcMouse) << event << "for" << *this;
    // TODO: finish drag, check if drag was acceptable
    // TODO: apply final position (in old or new slot)
    setKeepMouseGrab(false);
}

void Card::mouseMoveEvent(QMouseEvent *event)
{
    qCDebug(lcMouse) << event << "for" << *this;
    QPointF newPosition = mapPos(event->screenPos()) - m_dragLast;
    setX(x() + newPosition.x());
    setY(y() + newPosition.y());
    m_dragLast = mapPos(event->screenPos());
}

QDebug operator<<(QDebug debug, const Card &card)
{
    debug.nospace() << "Card(rank=";
    debug.nospace() << card.rank();
    debug.nospace() << ", suit=";
    debug.nospace() << card.suit();
    debug.nospace() << ", show=";
    debug.nospace() << card.show();
    debug.nospace() << ")";
    return debug.maybeSpace();
}
