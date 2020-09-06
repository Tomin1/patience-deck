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
    , m_margin(0, 0)
    , m_cardRenderer(Constants::DataDirectory + QStringLiteral("/anglo.svg"))
{
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
    connect(engine, &Engine::widthChanged, this, &Board::handleWidthChanged);
    connect(engine, &Engine::heightChanged, this, &Board::handleHeightChanged);
    connect(this, &Board::heightChanged, this, &Board::updateCardSize);
    connect(this, &Board::widthChanged, this, &Board::updateCardSize);
    connect(this, &Board::doCheckDrag, engine, &Engine::checkDrag);
    connect(this, &Board::doCheckDrop, engine, &Engine::checkDrop);
    connect(this, &Board::doDrop, engine, &Engine::drop);

    if (!m_cardRenderer.isValid())
        qCCritical(lcAisleriot) << "SVG file is not valid! Can not render cards!";
}

void Board::paint(QPainter *painter)
{
    Q_UNUSED(painter) // Nothing to paint here
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
        updateSlots();
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
        updateSlots();
    }
}

QSizeF Board::margin() const
{
    return m_margin;
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

QSvgRenderer *Board::cardRenderer()
{
    return &m_cardRenderer;
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
    for (auto it = m_slots.begin(); it != m_slots.end(); it++) {
        it.value()->deleteLater();
    }
    m_slots.clear();
    m_boardSize = QSizeF();
    m_cardSize = QSizeF();
    update();
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
    if (!m_boardSize.isValid())
        return;
    qCDebug(lcAisleriot) << "Drawing to" << QSize(width(), height()) << "area with margin of"
                         << m_margin << "and board size of" << m_boardSize;
    qreal maximumWidth = (width()-m_margin.width()) / m_boardSize.width() - m_margin.width();
    qreal maximumHeight = (height()-m_margin.height()) / m_boardSize.height() - m_margin.height();
    if ((maximumHeight * CardRatio) < maximumWidth) {
        m_cardSize = QSizeF(maximumHeight * CardRatio, maximumHeight);
        m_cardMargin = QSizeF((maximumWidth - m_cardSize.width()) / 2.0, 0.0);
    } else {
        m_cardSize = QSizeF(maximumWidth, maximumWidth / CardRatio);
        m_cardMargin = QSizeF(0.0, (maximumHeight - m_cardSize.height()) / 2.0);
    }
    m_cardSpace = QSizeF(maximumWidth, maximumHeight);
    qCInfo(lcAisleriot) << "Set card dimensions to" << m_cardSize
                        << "with space of" << m_cardSpace
                        << "and margin of" << m_cardMargin;
    updateSlots();
}

void Board::updateSlots() const
{
    for (auto it = m_slots.constBegin(); it != m_slots.constEnd(); it++) {
        Slot *slot = it.value();
        slot->updateDimensions();
    }
}
