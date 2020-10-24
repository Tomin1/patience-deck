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

#include <algorithm>
#include <QBrush>
#include <QColor>
#include <QPainter>
#include "table.h"
#include "constants.h"
#include "engine.h"
#include "slot.h"
#include "logging.h"

namespace {

const qreal CardRatio = 79.0 / 123.0;

}; // namespace

const QString Constants::DataDirectory = QStringLiteral(QUOTE(DATADIR) "/data");

Table::Table(QQuickItem *parent)
    : QQuickPaintedItem(parent)
    , m_minimumSideMargin(0)
    , m_sideMargin(0)
    , m_cardRenderer(Constants::DataDirectory + QStringLiteral("/anglo.svg"))
    , m_preparing(true)
{
    setFlag(QQuickItem::ItemClipsChildrenToShape);

    // Fill the scene with suitable color, nothing behind this is displayed
    setOpaquePainting(true);
    QColor backgroundColor(Qt::darkGreen);
    setFillColor(backgroundColor);

    auto engine = Engine::instance();
    connect(engine, &Engine::newSlot, this, &Table::handleNewSlot);
    connect(engine, &Engine::setExpansionToDown, this, &Table::handleSetExpansionToDown);
    connect(engine, &Engine::setExpansionToRight, this, &Table::handleSetExpansionToRight);
    connect(engine, &Engine::insertCard, this, &Table::handleInsertCard);
    connect(engine, &Engine::appendCard, this, &Table::handleAppendCard);
    connect(engine, &Engine::removeCard, this, &Table::handleRemoveCard);
    connect(engine, &Engine::clearSlot, this, &Table::handleClearSlot);
    connect(engine, &Engine::clearData, this, &Table::handleClearData);
    connect(engine, &Engine::gameStarted, this, &Table::handleGameStarted);
    connect(engine, &Engine::widthChanged, this, &Table::handleWidthChanged);
    connect(engine, &Engine::heightChanged, this, &Table::handleHeightChanged);
    connect(this, &Table::heightChanged, this, &Table::updateCardSize);
    connect(this, &Table::widthChanged, this, &Table::updateCardSize);

    if (!m_cardRenderer.isValid())
        qCCritical(lcPatience) << "SVG file is not valid! Can not render cards!";
}

void Table::paint(QPainter *painter)
{
    Q_UNUSED(painter) // Nothing to paint here
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
    return m_preparing;
}

QSvgRenderer *Table::cardRenderer()
{
    return &m_cardRenderer;
}

Slot *Table::getSlotAt(const QPointF &point, Slot *source)
{
    for (Slot *slot : m_slots) {
        if (slot == source)
            continue;
        QRectF children = slot->childrenRect();
        if (children.isValid() && (children.x() != 0 || children.y() != 0)) {
            qCCritical(lcPatience) << "Children rect" << children
                                   << "for slot" << slot->id() << "looks wrong"
                                   << "while source slot is" << source->id();
        }
        QRectF box(slot->x(), slot->y(),
                   std::max(slot->width(), children.width()),
                   std::max(slot->height(), children.height()));
        if (box.contains(point))
            return slot;
    }
    return nullptr;
}

void Table::handleNewSlot(int id, const CardList &cards, int type,
                          double x, double y, int expansionDepth,
                          bool expandedDown, bool expandedRight)
{
    m_slots.insert(id, new Slot(id, cards, SlotType(type), x, y, expansionDepth,
                                expandedDown, expandedRight, this));
    update();
}

void Table::handleSetExpansionToDown(int id, double expansion)
{
    Slot *slot = m_slots[id];
    if (slot->expandedRight()) {
        qCWarning(lcPatience) << "Can not set delta for expansion to down when expansion to right is set";
    } else if (slot->expandedDown()) {
        slot->setDelta(expansion);
        update();
    } else {
        qCWarning(lcPatience) << "Can not set delta when expansion is not set";
    }
}

void Table::handleSetExpansionToRight(int id, double expansion)
{
    update();
    Slot *slot = m_slots[id];
    if (slot->expandedDown()) {
        qCWarning(lcPatience) << "Can not set delta for expansion to right when expansion to down is set";
    } else if (slot->expandedRight()) {
        slot->setDelta(expansion);
        update();
    } else {
        qCWarning(lcPatience) << "Can not set delta when expansion is not set";
    }
}

void Table::handleInsertCard(int slotId, int index, const CardData &card)
{
    m_slots[slotId]->insertCard(index, card);
    update();
}

void Table::handleAppendCard(int slotId, const CardData &card)
{
    m_slots[slotId]->appendCard(card);
    update();
}

void Table::handleRemoveCard(int slotId, int index)
{
    m_slots[slotId]->removeCard(index);
    update();
}

void Table::handleClearSlot(int slotId)
{
    m_slots[slotId]->clear();
    update();
}


void Table::handleClearData()
{
    m_preparing = true;
    for (auto it = m_slots.begin(); it != m_slots.end(); it++) {
        it.value()->deleteLater();
    }
    m_slots.clear();
    m_tableSize = QSizeF();
    m_cardSize = QSizeF();
    update();
}

void Table::handleGameStarted()
{
    m_preparing = false;
}

void Table::handleWidthChanged(double width)
{
    m_tableSize.setWidth(width);
    updateCardSize();
}

void Table::handleHeightChanged(double height)
{
    m_tableSize.setHeight(height);
    updateCardSize();
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
                                  << " and table size of " << m_tableSize;

    qreal verticalSpace = width() - m_minimumSideMargin*2.0;
    qreal horizontalSpace = height() - m_margin.height()*2.0;
    qreal maximumWidth = (verticalSpace + m_margin.width()) / m_tableSize.width() - m_margin.width();
    qreal maximumHeight = (horizontalSpace + m_margin.height()) / m_tableSize.height() - m_margin.height();
    if ((maximumHeight * CardRatio) < maximumWidth) {
        m_cardSize = QSizeF(maximumHeight * CardRatio, maximumHeight);
        m_cardMargin = QSizeF((maximumWidth - m_cardSize.width()) / 2.0, 0.0);
    } else {
        m_cardSize = QSizeF(maximumWidth, maximumWidth / CardRatio);
        m_cardMargin = QSizeF(0.0, (maximumHeight - m_cardSize.height()) / 2.0);
    }

    qCDebug(lcPatience) << "Calculated maximum space of" << QSizeF(maximumWidth, maximumHeight)
                        << "and card margin of" << m_cardMargin;

    if (m_maximumMargin.width() > 0 && m_cardMargin.width() + m_margin.width() > m_maximumMargin.width())
        m_cardMargin.setWidth(std::max(m_maximumMargin.width() - m_margin.width(), (qreal)0.0F));
    if (m_maximumMargin.height() > 0 && m_cardMargin.height() + m_margin.height() > m_maximumMargin.height())
        m_cardMargin.setHeight(std::max(m_maximumMargin.height() - m_margin.height(), (qreal)0.0F));

    m_cardSpace = m_cardSize + m_cardMargin;
    m_sideMargin = (width() - (m_cardSpace.width()+m_margin.width())*m_tableSize.width() + m_margin.width()) / 2.0;
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
}
