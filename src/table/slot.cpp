/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2020-2022 Tomi Leppänen
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

#include <math.h>
#include "table.h"
#include "card.h"
#include "slot.h"
#include "constants.h"
#include "logging.h"

namespace {

const qreal CardStep = 0.2;
const qreal MinCardStep = 0.05;

} // namespace

Slot::Slot(int id, SlotType type, double x, double y, int expansionDepth,
           bool expandedDown, bool expandedRight, Table *table)
    : QQuickItem(table)
    , m_table(table)
    , m_id(id)
    , m_type(type)
    , m_exposed(false)
    , m_position(x, y)
    , m_expansionDelta(0.0)
    , m_calculatedDelta(0.0)
    , m_expansion(expandedDown ? ExpandsInY : DoesNotExpand
                | expandedRight ? ExpandsInX : DoesNotExpand)
    , m_expansionDepth(expansionDepth)
    , m_firstExpandedValid(false)
    , m_highlighted(false)
    , m_firstHighlightedCard(nullptr)
{
}

void Slot::updateDimensions()
{
    QSizeF margin = m_table->margin();
    QSizeF cardSpace = m_table->cardSpace();
    QSizeF cardMargin = m_table->cardMargin();
    setX(round(m_table->sideMargin() + (cardSpace.width() + margin.width()) * m_position.x() + cardMargin.width()));
    setY(round(margin.height() + (cardSpace.height() + margin.height()) * m_position.y() + cardMargin.height()));

    QSizeF cardSize = m_table->cardSize();
    setWidth(cardSize.width());
    setHeight(cardSize.height());
    for (Card *card : m_cards)
        if (card)
            card->setSize(cardSize);

    updateLocations();
}

void Slot::updateLocations()
{
    updateLocations(begin());
}

void Slot::updateLocations(iterator first)
{
    if (reevaluateDelta())
        first = begin();

    for (auto it = first; it != end(); ++it) {
        Card *card = *it;
        if (card) {
            card->setZ(it - begin());
            card->setVisible(it >= firstExpanded());
            if (expandedRight()) {
                card->setX(round(delta(it)));
                card->setY(0);
            } else if (expandedDown()) {
                card->setX(0);
                card->setY(round(delta(it)));
            } else {
                card->setX(0);
                card->setY(0);
            }
        }
    }
}

QPointF Slot::nextPosition() const
{
    qreal delta;
    if (isEmpty() || !expanded() || m_expansionDepth == Expansion::None)
        delta = 0.0;
    else if (m_expansionDepth == Expansion::Full || m_expansionDepth > m_cards.count())
        delta = m_calculatedDelta * m_cards.count();
    else
        delta = m_calculatedDelta * (m_expansionDepth - 1);

    if (expandedDown()) {
        return QPointF(0.0, round(delta));
    } else if (expandedRight()) {
        return QPointF(round(delta), 0.0);
    } else /* !expanded() */ {
        return QPointF(0.0, 0.0);
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

bool Slot::isEmpty() const
{
    return m_cards.isEmpty();
}

bool Slot::highlighted(const Card *card) const
{
    if (!m_highlighted || !card)
        return m_highlighted;

    if (!m_firstHighlightedCard)
        return top() == card;

    for (auto it = constFind(m_firstHighlightedCard); it != constEnd(); ++it) {
        if (*it == card)
            return true;
    }
    return false;
}

void Slot::insert(int index, Card *card)
{
    auto it = m_cards.begin() + index;
    m_cards.insert(it, card);
    m_firstExpandedValid = false;
    if (card)
        card->moveTo(this);
    // TODO: Do adjustments once move ends
    if (!m_table->preparing())
        updateLocations();
    qCDebug(lcSlot) << "Inserted" << card << "to" << this;
}

void Slot::set(int index, Card *card)
{
    if (m_cards.at(index)) {
        qCCritical(lcSlot) << "Tried to replace card in filled location in" << this << "at" << index;
    } else if (!card) {
        qCCritical(lcSlot) << "Tried to replace with null card in" << this << "at" << index;
    } else {
        m_cards.replace(index, card);
        card->moveTo(this);
        // TODO: Do adjustments once move ends
        if (!m_table->preparing())
            updateLocations();
        qCDebug(lcSlot) << "Replaced" << card << "at" << this << "index" << index;
    }
}

Card *Slot::takeAt(int index)
{
    Card *card = m_cards.takeAt(index);
    m_firstExpandedValid = false;
    if (card == m_firstHighlightedCard)
        m_firstHighlightedCard = nullptr;
    if (!m_table->preparing())
        updateLocations();
    qCDebug(lcSlot) << "Removed" << card << "from" << this;
    if (isEmpty())
        emit slotEmptied();
    return card;
}

QList<Card *> Slot::takeAll()
{
    QList<Card *> cards;
    m_cards.swap(cards);
    m_firstExpandedValid = false;
    m_firstHighlightedCard = nullptr;
    qCDebug(lcSlot) << "Removed all cards from" << this;
    emit slotEmptied();
    return cards;
}

void Slot::highlight(Card *card)
{
    m_highlighted = true;
    m_firstHighlightedCard = card;
    if (!isEmpty()) {
        for (auto it = find(card ?: top()); it != end(); ++it)
            (*it)->update();
    }
    qCDebug(lcSlot) << this << "is now highlighted";
}

void Slot::removeHighlight()
{
    m_highlighted = false;
    if (!isEmpty()) {
        for (auto it = find(m_firstHighlightedCard ?: top()); it != end(); ++it)
            (*it)->update();
    }
    m_firstHighlightedCard = nullptr;
    qCDebug(lcSlot) << this << "is no longer highlighted";
}

CardList Slot::asCardData(Card *first) const
{
    CardList list;
    for (auto it = constFind(first); it != constEnd(); ++it) {
        if (*it)
            list << (*it)->data();
        else
            qCCritical(lcSlot) << "Tried to convert non-existing card to data";
    }
    if (list.isEmpty())
        qCCritical(lcSlot) << "Returning an empty list of CardData";
    return list;
}

QList<Card *> Slot::take(Card *first)
{
    if (!first) {
        qCCritical(lcSlot) << "Tried to take cards with nullptr from" << this;
        return QList<Card *>();
    }

    auto it = find(first);
    QList<Card *> tail;
    for (auto it2 = it; it2 != end(); ++it2)
        tail.append(*it2);
    m_cards.erase(it, end());
    m_firstExpandedValid = false;
    if (m_firstHighlightedCard) {
        if (tail.contains(m_firstHighlightedCard))
            m_firstHighlightedCard = nullptr;
    }
    if (!m_table->preparing())
        updateLocations();
    qCDebug(lcSlot) << "Removed" << tail.count() << "cards from" << this;
    return tail;
}

void Slot::put(const QList<Card *> &cards)
{
    m_cards.append(cards);
    m_firstExpandedValid = false;
    for (Card *card : cards) {
        if (card)
            card->moveTo(this);
    }
    if (!m_table->preparing())
        updateLocations();
    qCDebug(lcSlot) << "Added" << cards.count() << "cards to" << this;
}

Card *Slot::top() const
{
    return m_cards.last();
}

bool Slot::contains(Card *card) const
{
    return m_cards.contains(card);
}

QRectF Slot::box() const
{
    QRectF box = boundingRect();
    if (!isEmpty())
        box = box.united(mapRectFromItem(m_cards.last(), m_cards.last()->boundingRect()));
    return box;
}

QPointF Slot::center() const
{
    if (!isEmpty() && !expanded())
        return top()->center();
    QRectF area = box();
    return QPointF(area.width() / 2, area.height() / 2);
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

qreal Slot::delta(const Slot::const_iterator &iter) const
{
    if (iter == constBegin() || !expanded() || iter < firstExpanded())
        return 0.0;

    return m_calculatedDelta * (iter - firstExpanded());
}

void Slot::setDelta(double delta)
{
    m_expansion |= DeltaSet;
    m_expansionDelta = delta;
}

int Slot::expansionDepth() const
{
    if (m_expansionDepth == Expansion::Full)
        return count();
    return m_expansionDepth;
}

Slot::const_iterator Slot::firstExpanded() const
{
    if (!m_firstExpandedValid) {
        if (m_expansionDepth == Expansion::Full || m_expansionDepth >= m_cards.count())
            m_firstExpanded = constBegin();
        else if (m_expansionDepth == Expansion::None /* && !isEmpty() */)
            m_firstExpanded = constEnd() - 1;
        else
            m_firstExpanded = constEnd() - m_expansionDepth;
        m_firstExpandedValid = true;
    }
    return m_firstExpanded;
}

Slot::const_iterator Slot::constBegin() const
{
    return m_cards.constBegin();
}

Slot::const_iterator Slot::constEnd() const
{
    return m_cards.constEnd();
}

Slot::const_iterator Slot::constFind(const Card *card) const
{
    if (!card)
        return constBegin();

    for (auto it = constEnd(); it-- != constBegin();) {
        if (*it == card)
            return it;
    }
    qCWarning(lcSlot) << this << "did not contain" << card;
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

Slot::iterator Slot::find(const Card *card)
{
    if (!card)
        return begin();

    for (auto it = end(); it-- != begin();) {
        if (*it == card)
            return it;
    }
    qCWarning(lcSlot) << this << "did not contain" << card;
    return end();
}

bool Slot::reevaluateDelta()
{
    qreal oldDelta = m_calculatedDelta;
    QSizeF cardSize = m_table->cardSize();

    qreal expansion;
    if (expandedRight())
        expansion = (m_table->width() - x()) / cardSize.width() / expansionDepth();
    else // expandedDown()
        expansion = (m_table->height() - y()) / cardSize.height() / expansionDepth();

    qreal maximumExpansion = m_expansion & DeltaSet ? m_expansionDelta : CardStep;
    if (expansion < MinCardStep)
        expansion = MinCardStep;
    else if (expansion > maximumExpansion)
        expansion = maximumExpansion;

    m_calculatedDelta = (expandedRight() ? cardSize.width() : cardSize.height()) * expansion;

    return oldDelta != m_calculatedDelta;
}

QDebug operator<<(QDebug debug, const Slot *slot)
{
    if (slot) {
        debug.nospace() << "Slot(id=";
        debug.nospace() << slot->id();
        debug.nospace() << ", #cards=";
        debug.nospace() << slot->count();
        debug.nospace() << ")";
    } else {
        debug.nospace() << "invalid slot";
    }
    return debug.space();
}
