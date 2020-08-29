#include <QBrush>
#include <QColor>
#include <QPainter>
#include "board.h"
#include "constants.h"
#include "engine.h"
#include "slot.h"
#include "logging.h"

Board::Board(QQuickItem *parent)
    : QQuickPaintedItem(parent)
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
}

void Board::paint(QPainter *painter)
{
    qCDebug(lcAisleriot) << "Time to paint";
    Q_UNUSED(painter) // TODO
}

void Board::handleNewSlot(int id, int type, double x, double y,
                          int expansionDepth, bool expandedDown, bool expandedRight)
{
    m_slots.insert(id, new Slot(id, SlotType(type), x, y, expansionDepth, expandedDown, expandedRight));
}

void Board::handleSetExpansionToDown(int id, double expansion)
{
    m_slots[id]->setExpansionToDown(expansion);
}

void Board::handleSetExpansionToRight(int id, double expansion)
{
    m_slots[id]->setExpansionToRight(expansion);
}

void Board::handleClearSlot(int id)
{
    m_slots[id]->clear();
}

void Board::handleNewCard(int slotId, int suit, int rank, bool faceDown)
{
    m_slots[slotId]->addCard(Suit(suit), Rank(rank), faceDown);
}
