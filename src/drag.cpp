#include <QGuiApplication>
#include <QScreen>
#include "drag.h"
#include "board.h"
#include "card.h"
#include "slot.h"
#include "constants.h"
#include "logging.h"

namespace {

QPointF mapPos(const QPointF &pos)
{
    switch (QGuiApplication::primaryScreen()->orientation()) {
    case Qt::LandscapeOrientation:
        return QPointF(pos.y(), -pos.x());
    case Qt::InvertedLandscapeOrientation:
        return QPointF(-pos.y(), pos.x());
    default:
        return QPointF(pos.x(), pos.y());
    };
}

CardList toCardData(const QList<Card *> &cards)
{
    CardList list;
    for (const Card *card : cards)
        list << card->data();
    if (list.isEmpty()) {
        qCCritical(lcAisleriot) << "Returning an empty list of CardData";
        abort();
    }
    return list;
}

}; // namespace

const qint64 Constants::ClickTimeout = 200;

const qreal Constants::DragDistance = 5;

quint32 Drag::s_count = 0;

Drag::Drag(QMouseEvent *event, Board *board, Slot *slot, Card *card)
    : QObject(card)
    , m_id(s_count++)
    , m_mayBeAClick(true)
    , m_completed(false)
    , m_board(board)
    , m_card(card)
    , m_source(slot)
    , m_target(nullptr)
{
    auto engine = Engine::instance();
    connect(this, &Drag::doDrag, engine, &Engine::drag);
    connect(this, &Drag::doCancelDrag, engine, &Engine::cancelDrag);
    connect(this, &Drag::doCheckDrop, engine, &Engine::checkDrop);
    connect(this, &Drag::doDrop, engine, &Engine::drop);
    connect(this, &Drag::doClick, engine, &Engine::click);
    connect(engine, &Engine::couldDrag, this, &Drag::handleCouldDrag);
    connect(engine, &Engine::couldDrop, this, &Drag::handleCouldDrop);
    connect(engine, &Engine::dropped, this, &Drag::handleDropped);

    m_startPoint = m_lastPoint = mapPos(event->screenPos());
    m_timer.start();

    emit doDrag(m_id, slot->id(), slot->asCardData(card));
}

Drag::~Drag()
{
    if (!m_cards.isEmpty()) {
        if (m_completed) {
            for (Card *card : m_cards) {
                card->setVisible(false);
                card->deleteLater();
            }
        } else {
            emit doCancelDrag(m_id, m_source->id(), toCardData(m_cards));
            m_source->put(m_cards);
        }
        m_cards.clear();
    }
}

Card *Drag::card() const
{
    return m_card;
}

Slot *Drag::source() const
{
    return m_source;
}

void Drag::update(QMouseEvent *event)
{
    testClick(event);

    if (m_cards.isEmpty())
        return;

    QPointF move = mapPos(event->screenPos()) - m_lastPoint;
    for (Card *card : m_cards) {
        card->setX(card->x() + move.x());
        card->setY(card->y() + move.y());
    }
    m_lastPoint = mapPos(event->screenPos());
}

void Drag::finish(QMouseEvent *event)
{
    if (testClick(event)) {
        qCDebug(lcAisleriot) << "Detected click on" << m_card;
        emit doClick(m_id, m_source->id());
        cancel();
        deleteLater();
        return;
    }

    if (m_cards.isEmpty())
        return;

    m_target = m_board->getSlotAt(m_board->mapFromScene(event->screenPos()), m_source);

    if (m_target) {
        qCDebug(lcAisleriot) << "Moving from" << m_source->id() << "to" << m_target->id();
        emit doCheckDrop(m_id, m_source->id(), m_target->id(), toCardData(m_cards));
    } else {
        cancel();
        deleteLater();
    }
}

void Drag::cancel()
{
    if (!m_cards.isEmpty()) {
        emit doCancelDrag(m_id, m_source->id(), toCardData(m_cards));
        m_source->put(m_cards);
        m_cards.clear();
    }

    deleteLater();
}

void Drag::handleCouldDrag(quint32 id, bool could)
{
    if (id != m_id)
        return;

    if (could) {
        m_cards = m_source->take(m_card);
        for (Card *card : m_cards) {
            card->setParentItem(m_board);
            QPointF position = m_board->mapFromItem(m_source, QPointF(card->x(), card->y()));
            card->setX(position.x());
            card->setY(position.y());
        }
    }
}

void Drag::handleCouldDrop(quint32 id, bool could)
{
    if (id != m_id)
        return;

    if (could) {
        emit doDrop(m_id, m_source->id(), m_target->id(), toCardData(m_cards));
    } else {
        cancel();
        deleteLater();
    }
}

void Drag::handleDropped(quint32 id, bool could)
{
    if (id != m_id)
        return;

    if (could)
        m_completed = true;
    else
        cancel();

    deleteLater();
}

bool Drag::testClick(QMouseEvent *event)
{
    if (!m_mayBeAClick)
        return false;

    if (m_timer.hasExpired(Constants::ClickTimeout))
        m_mayBeAClick = false;

    if ((m_startPoint - mapPos(event->screenPos())).manhattanLength() >= Constants::DragDistance)
        m_mayBeAClick = false;

    return m_mayBeAClick;
}
