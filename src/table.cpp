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

#include <algorithm>
#include <math.h>
#include <QBrush>
#include <QColor>
#include <QGuiApplication>
#include <QSGSimpleRectNode>
#include <QStyleHints>
#include "table.h"
#include "constants.h"
#include "card.h"
#include "drag.h"
#include "engine.h"
#include "slot.h"
#include "logging.h"

namespace {

const qreal CardRatio = 79.0 / 123.0;
const QMarginsF SlotMargins(3, 3, 3, 3);
const QMarginsF SlotOutlineWidth(3, 3, 3, 3);
const QColor DefaultHighlightColor(Qt::blue);
const qreal DefaultHighlightOpacity = 0.25;

} // namespace

const QString Constants::DataDirectory = QStringLiteral(QUOTE(DATADIR) "/data");

Table::Table(QQuickItem *parent)
    : QQuickItem(parent)
    , m_minimumSideMargin(0)
    , m_sideMargin(0)
    , m_dirty(true)
    , m_highlightedSlot(nullptr)
    , m_highlightColor(DefaultHighlightColor)
    , m_manager(this)
    , m_drag(nullptr)
{
    setAcceptedMouseButtons(Qt::LeftButton);
    setFlag(QQuickItem::ItemClipsChildrenToShape);
    setFlag(QQuickItem::ItemHasContents);
    m_highlightColor.setAlphaF(DefaultHighlightOpacity);

    auto engine = Engine::instance();
    connect(engine, &Engine::setExpansionToDown, this, &Table::handleSetExpansionToDown);
    connect(engine, &Engine::setExpansionToRight, this, &Table::handleSetExpansionToRight);
    connect(engine, &Engine::widthChanged, this, &Table::handleWidthChanged);
    connect(engine, &Engine::heightChanged, this, &Table::handleHeightChanged);
    connect(engine, &Engine::engineFailure, this, &Table::handleEngineFailure);
    connect(this, &Table::heightChanged, this, &Table::updateCardSize);
    connect(this, &Table::widthChanged, this, &Table::updateCardSize);
    connect(this, &Table::doClick, engine, &Engine::click);
}

QSGNode *Table::getPaintNodeForSlot(Slot *slot)
{
    auto target = QRectF(slot->x(), slot->y(), slot->width(), slot->height()) - SlotMargins;
    QColor outlineColor(Qt::gray);
    auto *node = new QSGSimpleRectNode(target, outlineColor);
    QColor backgroundColor(Qt::darkGreen);
    auto *innerNode = new QSGSimpleRectNode(target - SlotOutlineWidth, backgroundColor);
    node->appendChildNode(innerNode);
    if (slot->highlighted() && slot->isEmpty()) {
        node->appendChildNode(new QSGSimpleRectNode(target, m_highlightColor));
    }
    return node;
}

QSGNode *Table::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    auto *node = static_cast<QSGSimpleRectNode *>(oldNode);
    if (!node || m_dirty) {
        if (!node) {
            QColor backgroundColor(Qt::darkGreen);
            node = new QSGSimpleRectNode(boundingRect(), backgroundColor);
        }
        if (m_dirty) {
            node->setRect(boundingRect());
            node->removeAllChildNodes();
        }
        for (Slot *slot : m_slots) {
            auto slotNode = getPaintNodeForSlot(slot);
            node->appendChildNode(slotNode);
        }
    }
    return node;
}

qreal Table::minimumSideMargin() const
{
    return m_minimumSideMargin;
}

void Table::setMinimumSideMargin(qreal minimumSideMargin)
{
    if (m_minimumSideMargin != minimumSideMargin) {
        m_minimumSideMargin = minimumSideMargin;
        emit minimumSideMarginChanged();
        updateCardSize();
    }
}

qreal Table::horizontalMargin() const
{
    return m_margin.width();
}

void Table::setHorizontalMargin(qreal horizontalMargin)
{
    if (m_margin.width() != horizontalMargin) {
        m_margin.setWidth(horizontalMargin);
        emit horizontalMarginChanged();
        updateCardSize();
    }
}

qreal Table::maximumHorizontalMargin() const
{
    return m_maximumMargin.width();
}

void Table::setMaximumHorizontalMargin(qreal maximumHorizontalMargin)
{
    if (m_maximumMargin.width() != maximumHorizontalMargin) {
        m_maximumMargin.setWidth(maximumHorizontalMargin);
        emit maximumHorizontalMarginChanged();
        updateCardSize();
    }
}

qreal Table::verticalMargin() const
{
    return m_margin.height();
}

void Table::setVerticalMargin(qreal verticalMargin)
{
    if (m_margin.height() != verticalMargin) {
        m_margin.setHeight(verticalMargin);
        emit verticalMarginChanged();
        updateCardSize();
    }
}

qreal Table::maximumVerticalMargin() const
{
    return m_maximumMargin.height();
}

void Table::setMaximumVerticalMargin(qreal maximumVerticalMargin)
{
    if (m_maximumMargin.height() != maximumVerticalMargin) {
        m_maximumMargin.setHeight(maximumVerticalMargin);
        emit maximumVerticalMarginChanged();
        updateCardSize();
    }
}

QColor Table::highlightColor() const
{
    return m_highlightColor;
}

void Table::setHighlightColor(QColor color)
{
    if (m_highlightColor != color) {
        m_highlightColor = color;
        emit highlightColorChanged();
    }
}

void Table::resetHighlightColor()
{
    QColor color(DefaultHighlightColor);
    color.setAlphaF(DefaultHighlightOpacity);
    setHighlightColor(color);
}

qreal Table::sideMargin() const
{
    return m_sideMargin;
}

QSizeF Table::margin() const
{
    return m_margin;
}

QSizeF Table::maximumMargin() const
{
    return m_maximumMargin;
}

QSizeF Table::tableSize() const
{
    return m_tableSize;
}

QSizeF Table::cardSize() const
{
    return m_cardSize;
}

QSizeF Table::cardSpace() const
{
    return m_cardSpace;
}

QSizeF Table::cardMargin() const
{
    return m_cardMargin;
}

bool Table::preparing() const
{
    return m_manager.preparing();
}

QList<Slot *> Table::getSlotsFor(const Card *card, Slot *source)
{
    auto rect = mapRectFromItem(card, card->boundingRect());
    QMap<qreal, Slot *> results;
    for (Slot *slot : m_slots) {
        QRectF children = mapRectFromItem(slot, slot->childrenRect());
        auto box = mapRectFromItem(slot, slot->boundingRect()).united(children);
        auto overlapped = rect.intersected(box);
        if (!overlapped.isEmpty())
            results.insert(overlapped.height() * overlapped.width(), slot);
    }
    QList<Slot *> sorted;
    auto values = results.values();
    for (auto it = values.rbegin(); it != values.rend(); it++) {
        sorted.append(*it);
        if (*it == source)
            break;
    }
    return sorted;
}

void Table::highlight(Slot *slot)
{
    if (m_highlightedSlot != slot) {
        if (m_highlightedSlot)
            m_highlightedSlot->removeHighlight();
        if (slot)
            slot->highlight();
        m_highlightedSlot = slot;
        update();
    }
}

void Table::addSlot(Slot *slot)
{
    connect(slot, &Slot::slotEmptied, this, &Table::handleSlotEmptied);
    m_slots.insert(slot->id(), slot);
    m_dirty = true;
}

Slot *Table::slot(int id) const
{
    return m_slots.value(id);
}

void Table::clear()
{
    for (auto it = m_slots.begin(); it != m_slots.end(); it++)
        it.value()->deleteLater();
    m_slots.clear();
    m_tableSize = QSizeF();
    m_cardSize = QSizeF();
}

void Table::store(const QList<Card *> &cards)
{
    m_manager.store(cards);
}

Drag *Table::drag(QMouseEvent *event, Card *card)
{
    if (event->type() != QEvent::MouseButtonPress && m_drag && m_drag->card() == card)
        return m_drag;

    if (m_drag)
        m_drag->cancel();

    if (event->type() == QEvent::MouseButtonPress) {
        m_drag = new Drag(event, this, card->slot(), card);
        Drag *drag = m_drag;
        connect(m_drag, &Drag::destroyed, this, [this, drag] {
            if (m_drag == drag)
                m_drag = nullptr;
        });
        return m_drag;
    }

    return nullptr;
}

void Table::handleSetExpansionToDown(int id, double expansion)
{
    if (!preparing()) {
        qCWarning(lcPatience) << "Trying to change expansion to down while Table is not being prepared";
    } else {
        Slot *slot = m_slots[id];
        if (slot->expandedRight()) {
            qCWarning(lcPatience) << "Can not set delta for expansion to down when expansion to right is set";
        } else if (slot->expandedDown()) {
            slot->setDelta(expansion);
        } else {
            qCWarning(lcPatience) << "Can not set delta when expansion is not set";
        }
    }
}

void Table::handleSetExpansionToRight(int id, double expansion)
{
    if (!preparing()) {
        qCWarning(lcPatience) << "Trying to change expansion to down while Table is not being prepared";
    } else {
        Slot *slot = m_slots[id];
        if (slot->expandedDown()) {
            qCWarning(lcPatience) << "Can not set delta for expansion to right when expansion to down is set";
        } else if (slot->expandedRight()) {
            slot->setDelta(expansion);
        } else {
            qCWarning(lcPatience) << "Can not set delta when expansion is not set";
        }
    }
}

void Table::handleSlotEmptied()
{
    m_dirty = true;
    if (!preparing())
        update();
}

void Table::handleWidthChanged(double width)
{
    if (!preparing()) {
        qCWarning(lcPatience) << "Trying to table width while Table is not being prepared";
    } else {
        m_tableSize.setWidth(width);
    }
}

void Table::handleHeightChanged(double height)
{
    if (!preparing()) {
        qCWarning(lcPatience) << "Trying to table height while Table is not being prepared";
    } else {
        m_tableSize.setHeight(height);
    }
}

void Table::updateCardSize()
{
    // TODO: Queue this, don't update many times in a row
    if (!m_tableSize.isValid())
        return;

    qCDebug(lcPatience).nospace() << "Drawing to " << QSize(width(), height())
                                  << " area with margin of " << m_margin
                                  << ", maximum margin of " << m_maximumMargin
                                  << ", minimum side margin of " << m_minimumSideMargin
                                  << "and table size of " << m_tableSize;

    qreal verticalSpace = width() - m_minimumSideMargin*2.0;
    qreal horizontalSpace = height() - m_margin.height()*2.0;
    qreal maximumWidth = (verticalSpace + m_margin.width()) / m_tableSize.width() - m_margin.width();
    qreal maximumHeight = (horizontalSpace + m_margin.height()) / m_tableSize.height() - m_margin.height();
    if ((maximumHeight * CardRatio) < maximumWidth) {
        m_cardSize = QSizeF(round(maximumHeight * CardRatio), round(maximumHeight));
        m_cardMargin = QSizeF((maximumWidth - m_cardSize.width()) / 2.0, 0.0);
    } else {
        m_cardSize = QSizeF(round(maximumWidth), round(maximumWidth / CardRatio));
        m_cardMargin = QSizeF(0.0, (maximumHeight - m_cardSize.height()) / 2.0);
    }

    qCDebug(lcPatience) << "Calculated maximum space of" << QSizeF(maximumWidth, maximumHeight)
                        << "and card margin of" << m_cardMargin;

    if (m_maximumMargin.width() > 0 && m_cardMargin.width() + m_margin.width() > m_maximumMargin.width())
        m_cardMargin.setWidth(std::max(m_maximumMargin.width() - m_margin.width(), (qreal)0.0F));
    if (m_maximumMargin.height() > 0 && m_cardMargin.height() + m_margin.height() > m_maximumMargin.height())
        m_cardMargin.setHeight(std::max(m_maximumMargin.height() - m_margin.height(), (qreal)0.0F));

    m_cardSpace = m_cardSize + m_cardMargin;
    m_sideMargin = ceil((width() - (m_cardSpace.width()+m_margin.width())*m_tableSize.width() + m_margin.width()) / 2.0);
    if (m_sideMargin < m_minimumSideMargin)
        qCWarning(lcPatience) << "Miscalculated side margin! Current is" << m_sideMargin
                              << "but it should be" << m_minimumSideMargin;

    qCInfo(lcPatience) << "Set card dimensions to" << m_cardSize
                       << "with space of" << m_cardSpace
                       << "and margin of" << m_cardMargin;
    qCDebug(lcPatience) << "Side margin is" << m_sideMargin;

    for (auto it = m_slots.constBegin(); it != m_slots.constEnd(); it++) {
        Slot *slot = it.value();
        slot->updateDimensions();
    }

    m_dirty = true;
    if (!preparing())
        update();
}

void Table::handleEngineFailure()
{
    setEnabled(false);
}

Table::iterator Table::begin()
{
    return m_slots.keyBegin();
}

Table::iterator Table::end()
{
    return m_slots.keyEnd();
}

void Table::mousePressEvent(QMouseEvent *event)
{
    qCDebug(lcMouse) << event << "for" << *this;

    for (Slot *slot : m_slots) {
        QPointF point = mapToItem(slot, event->pos());
        if (slot->contains(point)) {
            qCDebug(lcMouse) << "Found slot" << *slot << "on click position";
            m_timer.start();
            m_startPoint = event->pos();
        }
    }
}

void Table::mouseReleaseEvent(QMouseEvent *event)
{
    qCDebug(lcMouse) << event << "for" << *this;

    auto styleHints = QGuiApplication::styleHints();
    if (!m_timer.hasExpired(styleHints->startDragTime())
            && (m_startPoint - event->pos()).manhattanLength() < styleHints->startDragDistance()) {
        for (Slot *slot : m_slots) {
            QPointF point = mapToItem(slot, event->pos());
            if (slot->contains(point)) {
                qCDebug(lcPatience) << "Detected click on" << *slot;
                emit doClick(-1, slot->id());
            }
        }
    }
}

QDebug operator<<(QDebug debug, const Table &)
{
    debug.nospace() << "Table()";
    return debug.space();
}
