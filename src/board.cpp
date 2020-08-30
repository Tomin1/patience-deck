#include <QBrush>
#include <QColor>
#include <QPainter>
#include <QSvgRenderer>
#include "board.h"
#include "constants.h"
#include "engine.h"
#include "slot.h"
#include "logging.h"

namespace {

const qreal CardRatio = 79.0 / 123.0;
const qreal CardStep = 0.2;
const qreal MinCardStep = 0.2;
const qreal SlotRounding = 10.0;
const int SlotOutlineWidth = 3;

}; // namespace

const QString Constants::DataDirectory = QStringLiteral("/usr/share/mobile-aisleriot/data");

Board::Board(QQuickItem *parent)
    : QQuickPaintedItem(parent)
    , m_margin(0, 0)
{
    // Fill the scene with suitable color, nothing behind this is displayed
    setOpaquePainting(true);
    QColor backgroundColor(Qt::darkGreen);
    setFillColor(backgroundColor);

    auto engine = Engine::instance();
    connect(engine, &Engine::newSlot, this, &Board::handleNewSlot);
    connect(engine, &Engine::setExpansionToDown, this, &Board::handleSetExpansionToDown);
    connect(engine, &Engine::setExpansionToRight, this, &Board::handleSetExpansionToRight);
    connect(engine, &Engine::clearSlot, this, &Board::handleClearSlot);
    connect(engine, &Engine::newCard, this, &Board::handleNewCard);
    connect(engine, &Engine::clearData, this, &Board::handleClearData);
    connect(engine, &Engine::widthChanged, this, &Board::handleWidthChanged);
    connect(engine, &Engine::heightChanged, this, &Board::handleHeightChanged);
    connect(this, &Board::heightChanged, this, &Board::updateCardSize);
    connect(this, &Board::widthChanged, this, &Board::updateCardSize);
}

void Board::paint(QPainter *painter)
{
    // TODO: Too slow!
    // TODO: Doesn't handle expansion depth
    if (!readyToPaint())
        return;
    qCDebug(lcAisleriot) << "Time to paint";
    QPen slotPen(Qt::gray);
    slotPen.setWidth(SlotOutlineWidth);
    painter->setPen(slotPen);
    QSvgRenderer cardRenderer(Constants::DataDirectory + QStringLiteral("/anglo.svg"));
    if (!cardRenderer.isValid())
        qCCritical(lcAisleriot) << "SVG file is not valid! Can not render cards!";
    for (auto it = m_slots.constBegin(); it != m_slots.constEnd(); it++) {
        Slot *slot = it.value();
        QPointF point = getPoint(slot->position());
        qCDebug(lcAisleriot) << "Drawing slot" << slot->id()
                             << "at" << slot->position() << "to" << point;
        QRectF target(point, m_cardSize);
        painter->drawRoundedRect(target, SlotRounding, SlotRounding, Qt::RelativeSize);
        if (slot->empty()) {
            // continue
        } else if (slot->expanded()) {
            qreal expansion = getExpansion(slot);
            bool first = true;
            for (auto cardIt = slot->constBegin(); cardIt != slot->constEnd(); cardIt++) {
                Card *card = *cardIt;
                auto element = card->elementName();
                if (first) {
                    first = false;
                } else if (slot->expandedRight()) {
                    qCDebug(lcAisleriot) << "expansion to right from"  << target.left() << "by" << expansion;
                    target.moveLeft(target.left() + expansion);
                } else if (slot->expandedDown()) {
                    qCDebug(lcAisleriot) << "expansion to down from" << target.top() << "by" << expansion;
                    target.moveTop(target.top() + expansion);
                }

                qCDebug(lcAisleriot) << "Rendering" << element << "to" << target;
                cardRenderer.render(painter, element, target);
            } // cards
        } else {
            // FIXME: Ugly code, add proper interface to Slot
            auto element = (*(--slot->constEnd()))->elementName();
            qCDebug(lcAisleriot) << "Rendering" << element << "to" << target;
            cardRenderer.render(painter, element, target);
        }
    } // slots
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
        update();
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
        update();
    }
}

void Board::handleNewSlot(int id, int type, double x, double y,
                          int expansionDepth, bool expandedDown, bool expandedRight)
{
    m_slots.insert(id, new Slot(id, SlotType(type), x, y, expansionDepth, expandedDown, expandedRight, this));
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

void Board::handleClearSlot(int id)
{
    m_slots[id]->clear();
    update();
}

void Board::handleNewCard(int slotId, int suit, int rank, bool show)
{
    m_slots[slotId]->addCard(Suit(suit), Rank(rank), show);
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
    update();
}

bool Board::readyToPaint()
{
    return m_boardSize.isValid() && m_cardSize.isValid();
}

QPointF Board::getPoint(const QPointF &position) const
{
    qreal x = m_margin.width() + (m_cardSpace.width() + m_margin.width()) * position.x();
    qreal y = m_margin.height() + (m_cardSpace.height() + m_margin.height()) * position.y();
    return QPointF(x + m_cardMargin.width(), y + m_cardMargin.height());
}

qreal Board::getExpansion(Slot *slot) const
{
    if (!slot->expanded())
        return 0.0;

    int count = slot->expansionDepth();
    if (count == Expansion::Full)
        count = slot->count();
    qreal maximumExpansion = slot->explicitDelta() ? slot->delta() : CardStep;
    qreal expansion;

    if (slot->expandedRight())
        expansion = (m_boardSize.width() - slot->position().x()) / m_boardSize.width() / slot->expansionDepth();
    else // slot->expandedDown()
        expansion = (m_boardSize.height() - slot->position().y()) / m_boardSize.height() / slot->expansionDepth();
    if (expansion < MinCardStep)
        expansion = MinCardStep;
    else if (expansion > maximumExpansion)
        expansion = maximumExpansion;

    return (slot->expandedRight() ? m_cardSize.width() : m_cardSize.height()) * expansion;
}
