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

#include <QSGSimpleRectNode>
#include <QSGSimpleTextureNode>
#include "table.h"
#include "card.h"
#include "constants.h"
#include "logging.h"
#include "slot.h"

namespace {

int getColumn(Rank rank) {
    switch (rank) {
    case RankAceHigh:
    case RankJoker:
    case BlackJoker:
        return 0;
    case RedJoker:
        return 1;
    case CardBack:
        return 2;
    default:
        return rank-1;
    }
}

int getRow(Rank rank, Suit suit)
{
    switch (rank) {
    case CardBack:
    case RankJoker:
    case BlackJoker:
    case RedJoker:
        return 4;
    default:
        return suit;
    }
}

} // namespace

Card::Card(const CardData &card, Table *table, Slot *slot, QObject *parent)
    : QQuickItem(slot)
    , m_table(table)
    , m_data(card)
    , m_dirty(true)
{
    setParent(parent);
    setAcceptedMouseButtons(Qt::LeftButton);
    setFlag(QQuickItem::ItemHasContents);
    connect(table, &Table::cardTextureUpdated, this, &Card::handleCardTextureUpdated);
    connect(table, &Table::doubleResolutionChanged, this, &Card::handleDoubleResolutionChanged);
}

QSGNode *Card::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    auto texture = m_table->cardTexture();
    if (!texture)
        return nullptr;

    auto *node = static_cast<QSGSimpleTextureNode *>(oldNode);
    if (!node || m_dirty) {
        if (!node)
            node = new QSGSimpleTextureNode();
        node->setTexture(texture);
        int column = getColumn(show() ? rank() : CardBack);
        int row = getRow(show() ? rank() : CardBack, suit());
        QSizeF size = m_table->cardSizeInTexture();
        QRect rect(column * size.width(), row * size.height(), size.width(), size.height());
        node->setSourceRect(rect);
        if (node->childCount() > 0)
            static_cast<QSGSimpleRectNode *>(node->firstChild())->setRect(boundingRect());
        m_dirty = false;
    }
    if (highlighted()) {
        if (node->childCount() < 1) {
            auto color = m_table->highlightColor();
            auto child = new QSGSimpleRectNode(boundingRect(), color);
            child->setFlag(QSGNode::OwnedByParent);
            node->appendChildNode(child);
        }
    } else { // !highlighted()
        if (node->childCount() > 0)
            node->removeAllChildNodes();
    }
    node->setRect(boundingRect());
    return node;
}

QSizeF Card::size() const
{
    return QSizeF(width(), height());
}

void Card::setSize(const QSizeF &size)
{
    if (width() != size.width() || height() != size.height()) {
        m_dirty = true;
        setWidth(size.width());
        setHeight(size.height());
    }
}

QPointF Card::topLeft() const
{
    return QPointF(x(), y());
}

void Card::setTopLeft(const QPointF &topLeft)
{
    setX(topLeft.x());
    setY(topLeft.y());
}

void Card::moveTo(QQuickItem *item)
{
    if (!item) {
        qCCritical(lcDrag) << "Tried to move" << this << "to non-existing item!";
        return;
    }

    if (parentItem() != item) {
        QPointF oldTopLeft = topLeft();
        QPointF newTopLeft = item->mapFromItem(parentItem(), oldTopLeft);
        if (newTopLeft != oldTopLeft)
            setTopLeft(newTopLeft);
        setParentItem(item);
    }
}

void Card::shred()
{
    setParentItem(nullptr);
    deleteLater();
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

CardData Card::data() const
{
    return m_data;
}

SuitAndRank Card::value() const
{
    return m_data.value();
}

void Card::setShow(bool show)
{
    if (m_data.show != show) {
        m_data.show = show;
        m_dirty = true;
    }
}

bool Card::operator==(const Card &other) const
{
    return m_data == other.m_data;
}

void Card::handleCardTextureUpdated()
{
    m_dirty = true;
    update();
}

void Card::handleDoubleResolutionChanged()
{
    m_dirty = true;
    update();
}

void Card::mousePressEvent(QMouseEvent *event)
{
    qCDebug(lcMouse) << event << "for" << this;

    if (m_table->drag(event, this))
        setKeepMouseGrab(true);
}

void Card::mouseReleaseEvent(QMouseEvent *event)
{
    qCDebug(lcMouse) << event << "for" << this;

    if (Drag *drag = m_table->drag(event, this)) {
        drag->finish(event);
        setKeepMouseGrab(false);
    }
}

void Card::mouseMoveEvent(QMouseEvent *event)
{
    qCDebug(lcMouse) << event << "for" << this;

    if (Drag *drag = m_table->drag(event, this))
        drag->update(event);
}

Slot *Card::slot() const
{
    return qobject_cast<Slot *>(parentItem());
}

bool Card::highlighted() const
{
    Slot *slot = this->slot();
    return slot && slot->highlighted(this);
}

QDebug operator<<(QDebug debug, const Card *card)
{
    if (card) {
        debug.nospace() << "Card(rank=";
        debug.nospace() << card->rank();
        debug.nospace() << ", suit=";
        debug.nospace() << card->suit();
        debug.nospace() << ", show=";
        debug.nospace() << card->show();
        debug.nospace() << ")";
    } else {
        debug.nospace() << "placeholder card";
    }
    return debug.space();
}
