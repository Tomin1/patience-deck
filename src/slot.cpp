#include <QPainter>
#include "board.h"
#include "card.h"
#include "slot.h"
#include "constants.h"
#include "logging.h"

namespace {

const qreal CardStep = 0.2;
const qreal MinCardStep = 0.05;
const qreal SlotRounding = 10.0;
const int SlotOutlineWidth = 3;
const QMarginsF SlotMargins(3, 3, 3, 3);

}; // namespace

Slot::Slot(int id, const CardList &cards, SlotType type, double x, double y,
           int expansionDepth, bool expandedDown, bool expandedRight, Board *board)
    : QQuickPaintedItem(board)
    , m_board(board)
    , m_id(id)
    , m_type(type)
    , m_exposed(false)
    , m_position(x, y)
    , m_expansionDelta(0.0)
    , m_calculatedDelta(0.0)
    , m_expansion(expandedDown ? ExpandsInY : DoesNotExpand
                | expandedRight ? ExpandsInX : DoesNotExpand)
    , m_expansionDepth(expansionDepth)
    , m_pen(Qt::gray)
{
    setAcceptedMouseButtons(Qt::LeftButton);
    for (const CardData &card : cards)
        m_cards.append(new Card(card, board, this));
    m_pen.setWidth(SlotOutlineWidth);
    auto engine = Engine::instance();
    connect(this, &Slot::doClick, engine, &Engine::click);
}

void Slot::paint(QPainter *painter)
{
    painter->setPen(m_pen);
    QRectF target = QRectF(0, 0, width(), height()) - SlotMargins;
    painter->drawRoundedRect(target, SlotRounding, SlotRounding, Qt::RelativeSize);
}

void Slot::updateDimensions()
{
    QSizeF margin = m_board->margin();
    QSizeF cardSpace = m_board->cardSpace();
    QSizeF cardMargin = m_board->cardMargin();
    setX(m_board->sideMargin() + (cardSpace.width() + margin.width()) * m_position.x() + cardMargin.width());
    setY(margin.height() + (cardSpace.height() + margin.height()) * m_position.y() + cardMargin.height());

    QSizeF cardSize = m_board->cardSize();
    setWidth(cardSize.width());
    setHeight(cardSize.height());
    for (Card *card : m_cards)
        card->setSize(cardSize);

    m_expansion &= ~DeltaCalculated;

    updateLocations();
}

void Slot::updateLocations(Card *first)
{
    int index = (first) ? m_cards.indexOf(first) : 0;
    for (auto it = find(first); it != end(); it++) {
        Card *card = *it;
        if (expandedRight()) {
            card->setX(delta(index));
            card->setY(0);
        } else if (expandedDown()) {
            card->setX(0);
            card->setY(delta(index));
        } else {
            card->setX(0);
            card->setY(0);
        }
        index++;
    }
}

int Slot::id() const
{
    return m_id;
}

QPointF Slot::position() const
{
    return m_position;
}

int Slot::count() const
{
    return m_cards.count();
}

bool Slot::empty() const
{
    return m_cards.empty();
}

void Slot::appendCard(const CardData &card)
{
    Card *newCard = new Card(card, m_board, this);
    m_cards.append(newCard);
    if (!m_board->preparing()) {
        newCard->setSize(m_board->cardSize());
        updateLocations(newCard);
    }
}

void Slot::insertCard(int index, const CardData &card)
{
    Card *newCard = new Card(card, m_board, this);
    m_cards.insert(index, newCard);
    if (!m_board->preparing()) {
        newCard->setSize(m_board->cardSize());
        updateLocations();
    }
}

void Slot::removeCard(int index)
{
    Card *card = m_cards.takeAt(index);
    card->deleteLater();
    // TODO: Store to card cache and take it from there to new slot
}

CardList Slot::asCardData(Card *first) const
{
    CardList list;
    for (auto it = constFind(first); it != constEnd(); it++)
        list << (*it)->data();
    if (list.isEmpty()) {
        qCCritical(lcAisleriot) << "Returning an empty list of CardData";
        abort();
    }
    return list;
}

QList<Card *> Slot::take(Card *first)
{
    QList<Card *> tail = m_cards.mid(m_cards.indexOf(first));
    m_cards.erase(find(first), end());
    return tail;
}

void Slot::put(const QList<Card *> &cards)
{
    m_cards.append(cards);
    for (Card *card : cards) {
        card->setParent(this);
        card->setParentItem(this);
    }
    updateLocations();
}

bool Slot::contains(Card *card) const
{
    return m_cards.contains(card);
}

bool Slot::expanded() const
{
    return m_expansion != DoesNotExpand;
}

bool Slot::expandedRight() const
{
    return m_expansion & ExpandsInX;
}

bool Slot::expandedDown() const
{
    return m_expansion & ExpandsInY;
}

qreal Slot::delta(int index)
{
    if (index == 0 || !expanded() || index < firstExpandedIndex())
        return 0.0;

    if (!(m_expansion & DeltaCalculated)) {
        qreal expansion;
        if (expandedRight())
            expansion = (m_board->boardSize().width() - position().x()) / expansionDepth();
        else // expandedDown()
            expansion = (m_board->boardSize().height() - position().y()) / expansionDepth();

        qreal maximumExpansion = m_expansion & DeltaSet ? m_expansionDelta : CardStep;
        if (expansion < MinCardStep)
            expansion = MinCardStep;
        else if (expansion > maximumExpansion)
            expansion = maximumExpansion;

        QSizeF cardSize = m_board->cardSize();
        m_calculatedDelta = (expandedRight() ? cardSize.width() : cardSize.height()) * expansion;
        m_expansion |= DeltaCalculated;
    }

    return m_calculatedDelta*(index - firstExpandedIndex());
}

void Slot::setDelta(double delta)
{
    m_expansion |= DeltaSet;
    m_expansion &= ~DeltaCalculated;
    m_expansionDelta = delta;
}

int Slot::expansionDepth() const
{
    if (m_expansionDepth == Expansion::Full)
        return count();
    return m_expansionDepth;
}

int Slot::firstExpandedIndex() const
{
    if (m_expansionDepth == Expansion::Full)
        return 0;
    return count() - m_expansionDepth;
}

Slot::const_iterator Slot::constBegin() const
{
    return m_cards.constBegin();
}

Slot::const_iterator Slot::constEnd() const
{
    return m_cards.constEnd();
}

Slot::const_iterator Slot::constFind(Card *card) const
{
    if (!card)
        return constBegin();

    for (auto it = constEnd(); it-- != constBegin();) {
        if (*it == card)
            return it;
    }
    return constEnd();
}

Slot::iterator Slot::begin()
{
    return m_cards.begin();
}

Slot::iterator Slot::end()
{
    return m_cards.end();
}

Slot::iterator Slot::find(Card *card)
{
    if (!card)
        return begin();

    for (auto it = end(); it-- != begin();) {
        if (*it == card)
            return it;
    }
    return end();
}

void Slot::mousePressEvent(QMouseEvent *event)
{
    qCDebug(lcMouse) << event << "for" << *this;
    m_timer.start();
    m_startPoint = event->pos();
}

void Slot::mouseReleaseEvent(QMouseEvent *event)
{
    qCDebug(lcMouse) << event << "for" << *this;

    if (!m_timer.hasExpired(Constants::ClickTimeout)
            && (m_startPoint - event->pos()).manhattanLength() < Constants::DragDistance) {
        qCDebug(lcAisleriot) << "Detected click on" << this;
        emit doClick(-1, id());
    }
}

QDebug operator<<(QDebug debug, const Slot &slot)
{
    debug.nospace() << "Slot(id=";
    debug.nospace() << slot.id();
    debug.nospace() << ", #cards=";
    debug.nospace() << slot.count();
    debug.nospace() << ")";
    return debug.maybeSpace();
}
