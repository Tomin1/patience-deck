#include <QPainter>
#include "table.h"
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

Card::Card(const CardData &card, Table *table, Slot *slot)
    : QQuickPaintedItem(slot)
    , m_table(table)
    , m_slot(slot)
    , m_data(card)
    , m_drag(nullptr)
{
    setAcceptedMouseButtons(Qt::LeftButton);
    setZ(1);
}

void Card::paint(QPainter *painter)
{
    m_table->cardRenderer()->render(painter, elementName());
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

bool Card::dragged() const
{
    return m_drag != nullptr;
}

CardData Card::data() const
{
    return m_data;
}

bool Card::operator==(const Card &other) const
{
    return m_data == other.m_data;
}

void Card::mousePressEvent(QMouseEvent *event)
{
    qCDebug(lcMouse) << event << "for" << *this;

    if (m_drag)
        m_drag->cancel();

    setKeepMouseGrab(true);

    m_drag = new Drag(event, m_table, m_slot, this);
    Drag *drag = m_drag;
    connect(m_drag, &Drag::destroyed, this, [this, drag] {
        if (m_drag == drag)
            m_drag = nullptr;
    });
}

void Card::mouseReleaseEvent(QMouseEvent *event)
{
    qCDebug(lcMouse) << event << "for" << *this;

    if (!m_drag) {
        qCCritical(lcPatience) << "Can not handle mouse release! There is no drag ongoing!";
        return;
    }

    m_drag->finish(event);

    setKeepMouseGrab(false);
}

void Card::mouseMoveEvent(QMouseEvent *event)
{
    qCDebug(lcMouse) << event << "for" << *this;

    if (!m_drag) {
        qCCritical(lcPatience) << "Can not handle mouse move! There is no drag ongoing!";
        return;
    }

    m_drag->update(event);
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
