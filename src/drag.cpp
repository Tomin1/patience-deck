#include <QGuiApplication>
#include <QScreen>
#include "drag.h"
#include "board.h"
#include "card.h"
#include "slot.h"
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

Drag::Drag(QMouseEvent *event, Slot *slot, Card *card, Board *board)
    : QObject(board)
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

    m_lastPoint = mapPos(event->screenPos());

    connect(engine, &Engine::couldDrag, this, &Drag::handleCouldDrag);
    emit doDrag(slot->id(), slot->asCardData(card));
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
    if (m_cards.isEmpty())
        return;

    QPointF move = mapPos(event->screenPos()) - m_lastPoint;
    for (Card *card : m_cards) {
        card->setX(card->x() + move.x());
        card->setY(card->y() + move.y());
    }
    m_lastPoint = mapPos(event->screenPos());
}

void Drag::finish(Slot *target)
{
    if (m_cards.isEmpty())
        return;

    if (target) {
        m_target = target;
        qCDebug(lcAisleriot) << "Moving from" << m_source->id() << "to" << m_target->id();
        connect(Engine::instance(), &Engine::couldDrop, this, &Drag::handleCouldDrop);
        emit doCheckDrop(m_source->id(), m_target->id(), toCardData(m_cards));
    } else {
        emit doCancelDrag(m_source->id(), toCardData(m_cards));
        m_source->put(m_cards);
        m_cards.clear();
        end();
    }
}

void Drag::handleCouldDrag(bool could)
{
    if (could)
        m_cards = m_source->take(m_card);
    disconnect(Engine::instance(), &Engine::couldDrag, this, &Drag::handleCouldDrag);
}

void Drag::handleCouldDrop(bool could)
{
    if (could) {
        connect(Engine::instance(), &Engine::dropped, this, &Drag::handleDropped);
        emit doDrop(m_source->id(), m_target->id(), toCardData(m_cards));
    } else {
        if (!m_cards.isEmpty()) {
            emit doCancelDrag(m_source->id(), toCardData(m_cards));
            m_source->put(m_cards);
            m_cards.clear();
        }
        end();
    }
    disconnect(Engine::instance(), &Engine::couldDrop, this, &Drag::handleCouldDrop);
}

void Drag::handleDropped(bool could)
{
    if (!could && !m_cards.isEmpty()) {
        emit doCancelDrag(m_source->id(), toCardData(m_cards));
        m_source->put(m_cards);
        m_cards.clear();
    }
    end();
    disconnect(Engine::instance(), &Engine::dropped, this, &Drag::handleDropped);
}

void Drag::end()
{
    if (!m_cards.isEmpty()) {
        for (Card *card : m_cards) {
            card->setVisible(false);
            card->deleteLater();
        }
        m_cards.clear();
    }

    deleteLater();
}
