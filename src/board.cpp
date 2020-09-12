#include <algorithm>
#include <QBrush>
#include <QColor>
#include <QPainter>
#include "board.h"
#include "constants.h"
#include "engine.h"
#include "slot.h"
#include "logging.h"

namespace {

const qreal CardRatio = 79.0 / 123.0;

}; // namespace

const QString Constants::DataDirectory = QStringLiteral("/usr/share/mobile-aisleriot/data");

Board::Board(QQuickItem *parent)
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
    connect(engine, &Engine::newSlot, this, &Board::handleNewSlot);
    connect(engine, &Engine::setExpansionToDown, this, &Board::handleSetExpansionToDown);
    connect(engine, &Engine::setExpansionToRight, this, &Board::handleSetExpansionToRight);
    connect(engine, &Engine::insertCard, this, &Board::handleInsertCard);
    connect(engine, &Engine::appendCard, this, &Board::handleAppendCard);
    connect(engine, &Engine::removeCard, this, &Board::handleRemoveCard);
    connect(engine, &Engine::clearData, this, &Board::handleClearData);
    connect(engine, &Engine::gameStarted, this, &Board::handleGameStarted);
    connect(engine, &Engine::widthChanged, this, &Board::handleWidthChanged);
    connect(engine, &Engine::heightChanged, this, &Board::handleHeightChanged);
    connect(this, &Board::heightChanged, this, &Board::updateCardSize);
    connect(this, &Board::widthChanged, this, &Board::updateCardSize);

    if (!m_cardRenderer.isValid())
        qCCritical(lcAisleriot) << "SVG file is not valid! Can not render cards!";
}

void Board::paint(QPainter *painter)
{
    Q_UNUSED(painter) // Nothing to paint here
}

qreal Board::minimumSideMargin() const
{
    return m_minimumSideMargin;
}

void Board::setMinimumSideMargin(qreal minimumSideMargin)
{
    if (m_minimumSideMargin != minimumSideMargin) {
        m_minimumSideMargin = minimumSideMargin;
        emit minimumSideMarginChanged();
        updateCardSize();
    }
}

qreal Board::horizontalMargin() const
{
    return m_margin.width();
}

void Board::setHorizontalMargin(qreal horizontalMargin)
{
    if (m_margin.width() != horizontalMargin) {
        m_margin.setWidth(horizontalMargin);
        emit horizontalMarginChanged();
        updateCardSize();
    }
}

qreal Board::maximumHorizontalMargin() const
{
    return m_maximumMargin.width();
}

void Board::setMaximumHorizontalMargin(qreal maximumHorizontalMargin)
{
    if (m_maximumMargin.width() != maximumHorizontalMargin) {
        m_maximumMargin.setWidth(maximumHorizontalMargin);
        emit maximumHorizontalMarginChanged();
        updateCardSize();
    }
}

qreal Board::verticalMargin() const
{
    return m_margin.height();
}

void Board::setVerticalMargin(qreal verticalMargin)
{
    if (m_margin.height() != verticalMargin) {
        m_margin.setHeight(verticalMargin);
        emit verticalMarginChanged();
        updateCardSize();
    }
}

qreal Board::maximumVerticalMargin() const
{
    return m_maximumMargin.height();
}

void Board::setMaximumVerticalMargin(qreal maximumVerticalMargin)
{
    if (m_maximumMargin.height() != maximumVerticalMargin) {
        m_maximumMargin.setHeight(maximumVerticalMargin);
        emit maximumVerticalMarginChanged();
        updateCardSize();
    }
}

qreal Board::sideMargin() const
{
    return m_sideMargin;
}

QSizeF Board::margin() const
{
    return m_margin;
}

QSizeF Board::maximumMargin() const
{
    return m_maximumMargin;
}

QSizeF Board::boardSize() const
{
    return m_boardSize;
}

QSizeF Board::cardSize() const
{
    return m_cardSize;
}

QSizeF Board::cardSpace() const
{
    return m_cardSpace;
}

QSizeF Board::cardMargin() const
{
    return m_cardMargin;
}

bool Board::preparing() const
{
    return m_preparing;
}

QSvgRenderer *Board::cardRenderer()
{
    return &m_cardRenderer;
}

Slot *Board::getSlotAt(const QPointF &point, Slot *source)
{
    for (Slot *slot : m_slots) {
        if (slot == source)
            continue;
        QRectF children = slot->childrenRect();
        if (children.isValid() && (children.x() != 0 || children.y() != 0)) {
            qCCritical(lcAisleriot) << "Children rect" << children
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

void Board::handleNewSlot(int id, const CardList &cards, int type,
                          double x, double y, int expansionDepth,
                          bool expandedDown, bool expandedRight)
{
    m_slots.insert(id, new Slot(id, cards, SlotType(type), x, y, expansionDepth,
                                expandedDown, expandedRight, this));
    update();
}

void Board::handleSetExpansionToDown(int id, double expansion)
{
    Slot *slot = m_slots[id];
    if (slot->expandedRight()) {
        qCWarning(lcAisleriot) << "Can not set delta for expansion to down when expansion to right is set";
    } else if (slot->expandedDown()) {
        slot->setDelta(expansion);
        update();
    } else {
        qCWarning(lcAisleriot) << "Can not set delta when expansion is not set";
    }
}

void Board::handleSetExpansionToRight(int id, double expansion)
{
    update();
    Slot *slot = m_slots[id];
    if (slot->expandedDown()) {
        qCWarning(lcAisleriot) << "Can not set delta for expansion to right when expansion to down is set";
    } else if (slot->expandedRight()) {
        slot->setDelta(expansion);
        update();
    } else {
        qCWarning(lcAisleriot) << "Can not set delta when expansion is not set";
    }
}

void Board::handleInsertCard(int slotId, int index, const CardData &card)
{
    m_slots[slotId]->insertCard(index, card);
    update();
}

void Board::handleAppendCard(int slotId, const CardData &card)
{
    m_slots[slotId]->appendCard(card);
    update();
}

void Board::handleRemoveCard(int slotId, int index)
{
    m_slots[slotId]->removeCard(index);
    update();
}


void Board::handleClearData()
{
    m_preparing = true;
    for (auto it = m_slots.begin(); it != m_slots.end(); it++) {
        it.value()->deleteLater();
    }
    m_slots.clear();
    m_boardSize = QSizeF();
    m_cardSize = QSizeF();
    update();
}

void Board::handleGameStarted()
{
    m_preparing = false;
}

void Board::handleWidthChanged(double width)
{
    m_boardSize.setWidth(width);
    updateCardSize();
}

void Board::handleHeightChanged(double height)
{
    m_boardSize.setHeight(height);
    updateCardSize();
}

void Board::updateCardSize()
{
    // TODO: Queue this, don't update many times in a row
    if (!m_boardSize.isValid())
        return;

    qCDebug(lcAisleriot).nospace() << "Drawing to " << QSize(width(), height())
                                   << " area with margin of " << m_margin
                                   << ", maximum margin of " << m_maximumMargin
                                   << ", minimum side margin of " << m_minimumSideMargin
                                   << " and board size of " << m_boardSize;

    qreal verticalSpace = width() - m_minimumSideMargin*2.0;
    qreal horizontalSpace = height() - m_margin.height()*2.0;
    qreal maximumWidth = (verticalSpace + m_margin.width()) / m_boardSize.width() - m_margin.width();
    qreal maximumHeight = (horizontalSpace + m_margin.height()) / m_boardSize.height() - m_margin.height();
    if ((maximumHeight * CardRatio) < maximumWidth) {
        m_cardSize = QSizeF(maximumHeight * CardRatio, maximumHeight);
        m_cardMargin = QSizeF((maximumWidth - m_cardSize.width()) / 2.0, 0.0);
    } else {
        m_cardSize = QSizeF(maximumWidth, maximumWidth / CardRatio);
        m_cardMargin = QSizeF(0.0, (maximumHeight - m_cardSize.height()) / 2.0);
    }

    qCDebug(lcAisleriot) << "Calculated maximum space of" << QSizeF(maximumWidth, maximumHeight)
                         << "and card margin of" << m_cardMargin;

    if (m_maximumMargin.width() > 0 && m_cardMargin.width() + m_margin.width() > m_maximumMargin.width())
        m_cardMargin.setWidth(std::max(m_maximumMargin.width() - m_margin.width(), (qreal)0.0F));
    if (m_maximumMargin.height() > 0 && m_cardMargin.height() + m_margin.height() > m_maximumMargin.height())
        m_cardMargin.setHeight(std::max(m_maximumMargin.height() - m_margin.height(), (qreal)0.0F));

    m_cardSpace = m_cardSize + m_cardMargin;
    m_sideMargin = (width() - (m_cardSpace.width()+m_margin.width())*m_boardSize.width() + m_margin.width()) / 2.0;
    if (m_sideMargin < m_minimumSideMargin)
        qCWarning(lcAisleriot) << "Miscalculated side margin! Current is" << m_sideMargin
                               << "but it should be" << m_minimumSideMargin;

    qCInfo(lcAisleriot) << "Set card dimensions to" << m_cardSize
                        << "with space of" << m_cardSpace
                        << "and margin of" << m_cardMargin;
    qCDebug(lcAisleriot) << "Side margin is" << m_sideMargin;

    for (auto it = m_slots.constBegin(); it != m_slots.constEnd(); it++) {
        Slot *slot = it.value();
        slot->updateDimensions();
    }
}
