/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2020  Tomi Leppänen
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

Slot::Slot(int id, const CardList &cards, SlotType type, double x, double y,
           int expansionDepth, bool expandedDown, bool expandedRight, Table *table)
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
{
    for (const CardData &card : cards)
        m_cards.append(new Card(card, table, this));
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

    for (auto it = first; it != end(); it++) {
        Card *card = *it;
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
    Card *newCard = new Card(card, m_table, this);
    m_cards.append(newCard);
    if (!m_table->preparing()) {
        newCard->setSize(m_table->cardSize());
        updateLocations(expanded() ? firstExpanded() : end());
    }
}

void Slot::insertCard(int index, const CardData &card)
{
    Card *newCard = new Card(card, m_table, this);
    m_cards.insert(index, newCard);
    if (!m_table->preparing()) {
        newCard->setSize(m_table->cardSize());
        updateLocations();
    }
}

void Slot::removeCard(int index)
{
    Card *card = m_cards.takeAt(index);
    card->deleteLater();
    if (expanded())
        updateLocations();
    // TODO: Store to card cache and take it from there to new slot
}

void Slot::clear()
{
    for (Card *card : m_cards) {
        card->setParentItem(nullptr);
        card->deleteLater();
    }
    m_cards.clear();
    // TODO: Store to card cache and take from there to new slot
}

CardList Slot::asCardData(Card *first) const
{
    CardList list;
    for (auto it = constFind(first); it != constEnd(); it++)
        list << (*it)->data();
    if (list.isEmpty()) {
        qCCritical(lcPatience) << "Returning an empty list of CardData";
        abort();
    }
    return list;
}

QList<Card *> Slot::take(Card *first)
{
    QList<Card *> tail = m_cards.mid(m_cards.indexOf(first));
    m_cards.erase(find(first), end());
    if (expanded())
       updateLocations();
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

qreal Slot::delta(Slot::const_iterator iter)
{
    if (iter == begin() || !expanded() || iter < firstExpanded())
        return 0.0;

    return m_calculatedDelta*(iter - firstExpanded());
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

Slot::iterator Slot::firstExpanded()
{
    if (m_expansionDepth == Expansion::Full || m_expansionDepth >= m_cards.count())
        return begin();
    return end() - m_expansionDepth;
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

bool Slot::reevaluateDelta()
{
    qreal oldDelta = m_calculatedDelta;

    qreal expansion;
    if (expandedRight())
        expansion = (m_table->tableSize().width() - position().x()) / expansionDepth();
    else // expandedDown()
        expansion = (m_table->tableSize().height() - position().y()) / expansionDepth();

    qreal maximumExpansion = m_expansion & DeltaSet ? m_expansionDelta : CardStep;
    if (expansion < MinCardStep)
        expansion = MinCardStep;
    else if (expansion > maximumExpansion)
        expansion = maximumExpansion;

    QSizeF cardSize = m_table->cardSize();
    m_calculatedDelta = (expandedRight() ? cardSize.width() : cardSize.height()) * expansion;

    return oldDelta != m_calculatedDelta;
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
