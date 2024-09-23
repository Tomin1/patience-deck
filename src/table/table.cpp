/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2020-2024 Tomi Leppänen
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
#include <functional>
#include <math.h>
#include <MGConfItem>
#include <QBrush>
#include <QColor>
#include <QCommandLineParser>
#include <QGuiApplication>
#include <QQuickWindow>
#include <QRunnable>
#include <QSGSimpleRectNode>
#include <QSGTexture>
#include <QStyleHints>
#include <random>
#include "animationbuilder.h"
#include "card.h"
#include "constants.h"
#include "drag.h"
#include "engine.h"
#include "feedbackevent.h"
#include "itertools.h"
#include "logging.h"
#include "patience.h"
#include "selection.h"
#include "slot.h"
#include "table.h"
#include "texturerenderer.h"

#define ANGLE(a) ((double)(a) / 3.141592653589793)

namespace {

const qreal CardBaseWidth = 79.0;
const qreal CardBaseHeight = 123.0;
const qreal CardRatio = CardBaseWidth / CardBaseHeight;
const qreal SlotMargin = 3 / CardBaseWidth;
const qreal SlotOutlineWidth = 3 / CardBaseWidth;
const QColor DefaultBackgroundColor(Qt::darkGreen);
const QColor DefaultHighlightColor(Qt::blue);
const qreal DefaultHighlightOpacity = 0.25;
const int CardsInMoveMinimum = 4;

float pow2(float value) { return value * value; }

float distanceToWall(float angle, const QPointF &point, const QSize &area)
{
    float width, height, alpha, beta;
    if (angle < 0.5) {
        width = area.width() - point.x();
        height = area.height() - point.y();
        alpha = angle;
        beta = 0.5 - angle;
    } else if (angle < 1) {
        width = point.x();
        height = area.height() - point.y();
        alpha = 1.0 - angle;
        beta = angle - 0.5;
    } else if (angle < 1.5) {
        width = point.x();
        height = point.y();
        alpha = angle - 1.0;
        beta = 1.5 - angle;
    } else /* (angle < 2.0) */ {
        width = area.width() - point.x();
        height = point.y();
        alpha = 2.0 - angle;
        beta = angle - 1.5;
    }
    return std::sqrt(std::min(pow2(width) + pow2(width * std::tan(RADIAN(alpha))),
                              pow2(height) + pow2(height * std::tan(RADIAN(beta)))));
}

class AngleDistribution
{
    static const float width;

public:
    AngleDistribution(float minAngle, float maxAngle)
        : m_distribution((maxAngle - minAngle) / 2.0 + minAngle, (maxAngle - minAngle) / (2.0 * width))
        , m_minAngle(minAngle)
        , m_maxAngle(maxAngle)
    {
    }

    template<typename URNG>
    float operator()(URNG &generator)
    {
        float value = m_distribution(generator);
        if (value < m_minAngle || value > m_maxAngle)
            return operator()(generator);
        return fixAngle(value);
    }

    static AngleDistribution fromPointAndArea(const QPointF &point, const QSize &area)
    {
        // Nearest corner
        int corner = 0;
        if (point.x() >= area.width() / 2)
            corner += 1;
        if (point.y() > area.height() / 2)
            corner += 2;
        float min = 0.0;
        switch (corner) {
        case 0: // top left
            min = -ANGLE(std::atan2(point.y(), point.x()));
            break;
        case 1: // top right
            min = ANGLE(std::atan2(point.y(), area.width() - point.x()));
            break;
        case 2: // bottom left
            min = 1.0f + ANGLE(std::atan2(area.height() - point.y(), point.x()));
            break;
        case 3: // bottom right
            min = 0.5f + ANGLE(std::atan2(area.width() - point.x(), area.height() - point.y()));
            break;
        default:
            Q_UNREACHABLE();
        }
        qCDebug(lcAnimation) << "computed angle:" << min << "-" << min + 1 << "for case" << corner
                             << "from" << point << "in" << area;
        return AngleDistribution(min, min + 1);
    }

    static AngleDistribution fromEndpoints(float a, float b)
    {
        return (a <= b) ? AngleDistribution(a, b) : AngleDistribution(b, a);
    }

private:
    static float fixAngle(float angle)
    {
        float result = std::remainder(angle, 2.0f);
        return result >= 0.0 ? result : result + 2.0;
    }

    std::normal_distribution<float> m_distribution;
    float m_minAngle;
    float m_maxAngle;
};

const float AngleDistribution::width = 2.0;

class RadiusDistribution
{
    static const float df;

public:
    RadiusDistribution(float minRadius, float maxRadius)
        : m_distribution(df)
        , m_minRadius(minRadius)
        , m_maxRadius(maxRadius)
        , m_multiplier(-(maxRadius - minRadius) / (df * 2.0))
    {
    }

    template<typename URNG>
    float operator()(URNG &generator)
    {
        float value = m_distribution(generator);
        value *= m_multiplier;
        value += m_maxRadius;
        if (value > m_maxRadius)
            qCWarning(lcAnimation) << "Bad value for radius generated:" << value
                                   << "[" << m_minRadius << "," << m_maxRadius << "]";
        if (value < m_minRadius)
            return operator()(generator);
        return value;
    }

private:
    std::chi_squared_distribution<float> m_distribution;
    float m_minRadius;
    float m_maxRadius;
    float m_multiplier;
};

const float RadiusDistribution::df = 4.0;

class TextureCleaningJob : public QRunnable
{
public:
    TextureCleaningJob(QSGTexture *texture)
        : texture(texture)
    {
        setAutoDelete(true);
    }

    void run() override
    {
        qCDebug(lcTable) << "Destroying texture";
        delete texture;
    }
private:
    QSGTexture *texture;
};

} // namespace

class SlotNode : public QSGGeometryNode
{
public:
    SlotNode()
        : QSGGeometryNode()
        , m_hidden(false)
    {
    }

    bool isSubtreeBlocked()
    {
        return m_hidden;
    }

    void setHidden(bool hidden)
    {
        m_hidden = hidden;
    }

private:
    bool m_hidden;
};

#define dirty(flag) (m_dirty & (flag))
#define smudge(flag) m_dirty |= (flag)
#define clean() m_dirty = Clean

Table::Table(QQuickItem *parent)
    : QQuickItem(parent)
    , m_minimumSideMargin(0)
    , m_sideMargin(0)
    , m_dirty(Filthy)
    , m_dirtyCardSize(true)
    , m_backgroundColor(DefaultBackgroundColor)
    , m_doubleResolution(false)
    , m_highlightedSlot(nullptr)
    , m_highlightColor(DefaultHighlightColor)
    , m_manager(this)
    , m_interaction(nullptr)
    , m_cardTexture(nullptr)
    , m_pendingCardTexture(nullptr)
    , m_previousWindow(nullptr)
    , m_animation(nullptr)
    , m_animate(false)
{
    setAcceptedMouseButtons(Qt::LeftButton);
    setFlag(QQuickItem::ItemClipsChildrenToShape);
    setFlag(QQuickItem::ItemHasContents);
    m_highlightColor.setAlphaF(DefaultHighlightOpacity);

    connect(this, &QQuickItem::windowChanged, this, &Table::connectWindowSignals);

    auto renderer = new TextureRenderer();
    renderer->moveToThread(&m_textureThread);
    QMetaObject::invokeMethod(renderer, "loadDocument", Qt::QueuedConnection);
    connect(&m_textureThread, &QThread::finished, renderer, &TextureRenderer::deleteLater);
    connect(this, &Table::doRenderCardTexture, renderer, &TextureRenderer::renderTexture);
    connect(renderer, &TextureRenderer::textureRendered, this, &Table::handleCardTextureRendered);
    connect(renderer, &TextureRenderer::doubleSizeTextureRendered, this, &Table::handleDoubleSizeTextureRendered);
    m_textureThread.start();

    auto engine = Engine::instance();
    connect(engine, &Engine::gameLoaded, this, &Table::update);
    connect(engine, &Engine::gameContinued, this, &Table::handleGameContinued);
    connect(engine, &Engine::setExpansionToDown, this, &Table::handleSetExpansionToDown);
    connect(engine, &Engine::setExpansionToRight, this, &Table::handleSetExpansionToRight);
    connect(engine, &Engine::widthChanged, this, &Table::handleWidthChanged);
    connect(engine, &Engine::heightChanged, this, &Table::handleHeightChanged);
    connect(engine, &Engine::engineFailure, this, &Table::handleEngineFailure);
    connect(this, &Table::heightChanged, this, &Table::handleSizeChanged);
    connect(this, &Table::widthChanged, this, &Table::handleSizeChanged);
    connect(this, &Table::doClick, engine, &Engine::click);

    Patience::instance()->newTable(this);
}

Table::~Table()
{
    m_textureThread.quit();
    m_textureThread.wait();
    setCardTexture(nullptr);
    setPendingCardTexture(nullptr);
}

QSize Table::size() const
{
    return QSize(width(), height());
}

void Table::addArguments(QCommandLineParser *parser)
{
    parser->addOptions({
        {{"b", "background"}, "Set background style", "default|<color>|adaptive|transparent"},
    });
}

void Table::setArguments(QCommandLineParser *parser)
{
    if (parser->isSet("background")) {
        MGConfItem backgroundConf(Constants::ConfPath + QStringLiteral("/backgroundColor"));
        QString value = parser->value("background");
        if (value == QLatin1String("default") || value == QLatin1String("adaptive"))
            backgroundConf.set(QString());
        else if (value == QLatin1String("transparent"))
            backgroundConf.set(QColor(0, 0, 0, 0).name(QColor::HexArgb));
        else
            backgroundConf.set(QColor(value).name(QColor::HexRgb));
        backgroundConf.sync();
    }
}

void Table::playWinAnimation()
{
    if (m_animation) {
        qCWarning(lcAnimation) << "Win animation is already playing!";
        return;
    }

    resetCardPositions();
    m_animate = true;
    polish();
}

void Table::createWinAnimation()
{
    // SAFETY: While Engine is owned by another thread, it's not destroyed until
    // the app closes and the seed only changes when a new game is started
    std::mt19937 generator(Engine::instance()->seed());
    AnimationBuilder builder = AnimationBuilder::sequentialAnimation(this);
    using SlotIterator = typename QVector<Slot *>::iterator;
    IndexedIterator<SlotIterator> shuffledIt = shuffled(m_slots, generator);
    using ShuffledSlotIterator = typename IndexedIterator<SlotIterator>::iterator;
    int count = 0;
    GroupedIterator<ShuffledSlotIterator> onTable = grouped(
            shuffledIt,
            [this, &count](const Slot *slot) {
                count += slot->count();
                if (count >= CardsInMoveMinimum) {
                    count = 0;
                    return true;
                }
                return false;
            });
    using SlotGroupIterator = typename GroupedIterator<ShuffledSlotIterator>::slice;
    int z = 1;
    for (SlotGroupIterator groupIt : onTable) {
        AnimationBuilder::ParallelGroupGuard guard(&builder);
        for (Slot *slot : groupIt) {
            if (!slot->isEmpty()) {
                builder.addZAnimation(slot, z++);
                auto angleDistribution = AngleDistribution::fromPointAndArea(
                        mapFromItem(slot, slot->center()), size());
                for (auto it = slot->begin(); it != slot->end(); ++it) {
                    float angle = angleDistribution(generator);
                    Card *card = *it;
                    float maxRadius = distanceToWall(angle, mapFromItem(slot, card->center()), size());
                    float minRadius = m_cardSize.height() * 1.5 < maxRadius ? m_cardSize.height() : m_cardSize.width();
                    RadiusDistribution radiusDistribution(minRadius, maxRadius);
                    card->setVisible(true);
                    float radius = radiusDistribution(generator);
                    qCDebug(lcAnimation) << "Card from" << slot << "to" << angle
                                         << "by" << radius << "<" << maxRadius;
                    builder.addARAnimation(card, angle, radius, 1000);
                }
            }
        }
    }
    count = 0;
    GroupedIterator<Manager::iterator> offTable = grouped(
            m_manager,
            [this, &count](const Card *) {
                if (++count >= CardsInMoveMinimum) {
                    count = 0;
                    return true;
                }
                return false;
            });
    using CardGroupIterator = typename GroupedIterator<Manager::iterator>::slice;
    std::uniform_int_distribution<int> widthDistribution(0, width() - m_cardSize.width());
    for (CardGroupIterator slice : offTable) {
        AnimationBuilder::ParallelGroupGuard guard(&builder);
        for (Card *card : slice) {
            card->setParentItem(this);
            card->setX(widthDistribution(generator));
            card->setY(height() + m_cardSize.height() * .5);
            card->setZ(z++);
            int left = card->x() + m_cardSize.width() * .5;
            int right = width() - left;
            auto angleDistribution = AngleDistribution::fromEndpoints(
                    ANGLE(std::atan2(m_cardSize.height() * 1.5, left)) + 1.0f,
                    2.0f - ANGLE(std::atan2(m_cardSize.height() * 1.5, right)));
            float angle = angleDistribution(generator);
            float maxRadius = distanceToWall(angle, card->center(), size());
            RadiusDistribution radiusDistribution(maxRadius * .5, maxRadius);
            card->setVisible(true);
            float radius = radiusDistribution(generator);
            qCDebug(lcAnimation) << "Card from store to" << angle << "by" << radius << "<" << maxRadius;
            builder.addARAnimation(card, angle, radius, 1000);
        }
    };
    m_animation = builder;
    connect(m_animation, &QAbstractAnimation::finished, this, [this] {
        qCInfo(lcAnimation) << "Win animation finished";
        if (m_animation) {
            m_animation->deleteLater();
            m_animation = nullptr;
            emit animationPlayingChanged();
        }
    });
    m_animate = false;
    emit animationPlayingChanged();
    m_animation->start();
    qCInfo(lcAnimation) << "Win animation started";
}

void Table::stopAnimation()
{
    if (m_animation) {
        qCInfo(lcAnimation) << "Win animation canceled";
        bool wasPaused = animationPaused();
        m_animation->stop();
        if (wasPaused)
            emit animationPausedChanged();
        disconnect(m_animation, &QAbstractAnimation::finished, 0, 0);
        m_animation->deleteLater();
        m_animation = nullptr;
        resetCardPositions();
        emit animationPlayingChanged();
    }
}

void Table::resetCardPositions()
{
    for (Slot *slot : m_slots) {
        slot->setZ(0);
        slot->updateLocations();
    }
    m_manager.forEach([](Card *card) {
        card->setZ(0);
        card->setParentItem(nullptr);
    });
}

void Table::handleGameContinued()
{
    qCDebug(lcAnimation) << "Clearing animation as game continues";
    stopAnimation();
}

void Table::updatePolish()
{
    if (m_dirtyCardSize)
        updateCardSize();
    swapCardTexture();

    if (m_animate)
        createWinAnimation();
}

QRectF Table::getSlotOutline(Slot *slot)
{
    qreal margin = SlotMargin * slot->width();
    QMarginsF margins(margin, margin, margin, margin);
    return QRectF(slot->x(), slot->y(), slot->width(), slot->height()) - margins;
}

void Table::setGeometryForSlotNode(SlotNode *node, Slot *slot)
{
    auto *geometry = node->geometry();
    if (!geometry) {
        geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 10);
        geometry->setVertexDataPattern(QSGGeometry::StaticPattern);
        geometry->setIndexDataPattern(QSGGeometry::StaticPattern);
        node->setGeometry(geometry);
        node->setFlag(QSGNode::OwnsGeometry);
    }

    auto outside = getSlotOutline(slot);
    qreal outline = SlotOutlineWidth * slot->width();
    auto inside = outside - QMarginsF(outline, outline, outline, outline);
    auto *data = geometry->vertexDataAsPoint2D();
    data[0].set(outside.left(), outside.top());
    data[1].set(inside.left(), inside.top());
    data[2].set(outside.right(), outside.top());
    data[3].set(inside.right(), inside.top());
    data[4].set(outside.right(), outside.bottom());
    data[5].set(inside.right(), inside.bottom());
    data[6].set(outside.left(), outside.bottom());
    data[7].set(inside.left(), inside.bottom());
    data[8].set(outside.left(), outside.top());
    data[9].set(inside.left(), inside.top());
    node->markDirty(QSGNode::DirtyGeometry);
}

void Table::setHighlightForSlotNode(SlotNode *node, Slot *slot)
{
    auto child = static_cast<QSGSimpleRectNode *>(node->firstChild());
    if (dirty(HighlightedSlot)) {
        if (slot->highlighted() && slot->isEmpty()) {
            if (!child) {
                child = new QSGSimpleRectNode(getSlotOutline(slot), m_highlightColor);
                child->setFlag(QSGNode::OwnedByParent);
                node->appendChildNode(child);
                return;
            }
        } else if (child) {
            node->removeAllChildNodes();
            return;
        }
    }
    if (child && dirty(HighlightColor))
        child->setColor(m_highlightColor);
    if (child && dirty(SlotSize))
        child->setRect(getSlotOutline(slot));
}

void Table::setMaterialForSlotNode(SlotNode *node)
{
    static QSGFlatColorMaterial *material = nullptr;
    if (!material) {
        material = new QSGFlatColorMaterial;
        material->setColor(QColor(Qt::gray));
    }
    node->setMaterial(material);
}

QSGNode *Table::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    auto *node = oldNode;
    if (!node || m_dirty) {
        if (!node || dirty(BackgroundType)) {
            if (transparentBackground())
                node = new QSGNode();
            else
                node = new QSGSimpleRectNode(boundingRect(), m_backgroundColor);
            m_dirty = SlotCount;
        }

        if (dirty(BackgroundSize) && !transparentBackground())
            static_cast<QSGSimpleRectNode *>(node)->setRect(boundingRect());

        if (dirty(BackgroundColor) && !transparentBackground())
            static_cast<QSGSimpleRectNode *>(node)->setColor(m_backgroundColor);

        if (dirty(SlotCount)) {
            node->removeAllChildNodes();
            for (int i = 0; i < m_slots.count(); i++) {
                auto *slotNode = new SlotNode();
                setMaterialForSlotNode(slotNode);
                slotNode->setFlag(QSGNode::OwnedByParent);
                node->appendChildNode(slotNode);
            }
            smudge(HighlightedSlot | HighlightColor | SlotSize | HiddenSlots);
        }

        if (dirty(HighlightedSlot | HighlightColor | SlotSize | HiddenSlots)) {
            auto slotIt = m_slots.constBegin();
            auto nodeIt = static_cast<SlotNode *>(node->firstChild());
            while (slotIt != m_slots.constEnd() && nodeIt != nullptr) {
                if (dirty(SlotSize))
                    setGeometryForSlotNode(nodeIt, *slotIt);
                if (dirty(HiddenSlots))
                    nodeIt->setHidden((*slotIt)->isEmpty());
                if (dirty(HighlightedSlot | HighlightColor | SlotSize))
                    setHighlightForSlotNode(nodeIt, *slotIt);
                ++slotIt;
                nodeIt = static_cast<SlotNode *>(nodeIt->nextSibling());
            }
            if (slotIt != m_slots.constEnd())
                qCCritical(lcTable) << "Failed to iterate through all slots!";
            if (nodeIt != nullptr)
                qCCritical(lcTable) << "Failed to iterate through all slot nodes!";
        }

        clean();
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
        setDirtyCardSize();
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
        setDirtyCardSize();
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
        setDirtyCardSize();
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
        setDirtyCardSize();
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
        setDirtyCardSize();
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
        smudge(HighlightColor);
        if (!preparing())
            update();
    }
}

void Table::resetHighlightColor()
{
    QColor color(DefaultHighlightColor);
    color.setAlphaF(DefaultHighlightOpacity);
    setHighlightColor(color);
}

QColor Table::backgroundColor() const
{
    return m_backgroundColor;
}

void Table::setBackgroundColor(QColor color)
{
    if (m_backgroundColor != color) {
        if (transparentBackground() && color.alpha())
            smudge(BackgroundType);
        else if (!transparentBackground() && !color.alpha())
            smudge(BackgroundType);

        m_backgroundColor = color;
        emit backgroundColorChanged();
        smudge(BackgroundColor);

        if (!preparing())
            update();
    }
}

void Table::resetBackgroundColor()
{
    setBackgroundColor(DefaultBackgroundColor);
}

bool Table::transparentBackground() const
{
    return !m_backgroundColor.alpha();
}

bool Table::doubleResolution() const
{
    return m_doubleResolution;
}

void Table::setDoubleResolution(bool doubleResolution)
{
    if (m_doubleResolution != doubleResolution) {
        m_doubleResolution = doubleResolution;
        emit doubleResolutionChanged();
        if (!m_doubleSizeImage.isNull())
            createCardTexture();
    }
}

bool Table::animationPlaying() const
{
    return m_animation;
}

bool Table::animationPaused() const
{
    return m_animation && m_animation->state() == QAbstractAnimation::Paused;
}

void Table::setAnimationPaused(bool paused)
{
    if (m_animation
            && ((paused && m_animation->state() == QAbstractAnimation::Running)
            || (!paused && m_animation->state() == QAbstractAnimation::Paused))) {
        m_animation->setPaused(paused);
        emit animationPausedChanged();
        qCDebug(lcAnimation) << "Win animation" << (paused ? "paused" : "resumed");
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

QSizeF Table::cardSizeInTexture() const
{
    if (textureIsDoubleSize())
        return m_cardSizeInTexture * 2;
    return m_cardSizeInTexture;
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

QList<Slot *> Table::getSlotsFor(const QRectF &rect, Slot *source)
{
    QMap<qreal, Slot *> results;
    for (Slot *slot : m_slots) {
        auto box = mapRectFromItem(slot, slot->box());
        auto overlapped = rect.intersected(box);
        if (!overlapped.isEmpty())
            results.insert(overlapped.height() * overlapped.width(), slot);
    }
    QList<Slot *> sorted;
    auto values = results.values();
    for (auto it = values.rbegin(); it != values.rend(); ++it) {
        sorted.append(*it);
        if (*it == source)
            break;
    }
    return sorted;
}

QRectF Table::getBoundingRect(const QList<Card *> &cards)
{
    // Assumes that cards are in order
    Card *first = cards.first();
    Card *last = cards.last();
    return mapRectFromItem(first, first->boundingRect()).united(mapRectFromItem(last, last->boundingRect()));
}

QList<Slot *> Table::getSlotsFor(const Card *card, const QList<Card *> cards, Slot *source)
{
    QList<Slot *> results = getSlotsFor(mapRectFromItem(card, card->boundingRect()), source);
    for (Slot *slot : getSlotsFor(getBoundingRect(cards), source)) {
        if (!results.contains(slot))
            results.append(slot);
    }
    return results;
}

void Table::highlight(Slot *slot, Card *card)
{
    if (m_highlightedSlot != slot) {
        if (m_highlightedSlot)
            m_highlightedSlot->removeHighlight();
        if (slot)
            slot->highlight(card);
        m_highlightedSlot = slot;
        smudge(HighlightedSlot);
        update();
    }
}

void Table::addSlot(Slot *slot)
{
    connect(slot, &Slot::slotEmptied, this, &Table::handleSlotEmptied);
    if (slot->id() >= m_slots.size())
        m_slots.resize(slot->id() + 1);
    m_slots[slot->id()] = slot;
    smudge(SlotCount);
}

Slot *Table::slot(int id) const
{
    return m_slots.value(id);
}

void Table::clear()
{
    m_slots.clear();
    m_highlightedSlot = nullptr;
    if (m_interaction)
        m_interaction->deleteLater();
    m_interaction = nullptr;
    m_tableSize = QSizeF();
    m_cardSize = QSizeF();
    setCardTexture(nullptr);
    setPendingCardTexture(nullptr);
    m_cardSizeInTexture = QSizeF();
    m_cardImage = QImage();
    m_doubleSizeImage = QImage();
    smudge(SlotCount);
}

void Table::store(const QList<Card *> &cards)
{
    m_manager.store(cards);
}

Drag *Table::drag(QMouseEvent *event, Card *card)
{
    Selection *selection = qobject_cast<Selection *>(m_interaction);
    if (selection && event->type() == QEvent::MouseButtonPress) {
        if (selection->contains(card))
            // Give this selection to a new drag to handle
            m_interaction = nullptr;
        else
            selection->finish(card->slot());
    } else if (Drag *drag = qobject_cast<Drag *>(m_interaction)) {
        if (event->type() != QEvent::MouseButtonPress && drag->card() == card)
            return drag;

        if (event->type() == QEvent::MouseButtonPress)
            qCWarning(lcTable) << "Ignoring mouse press due to previous drag still existing" << drag;
        else if (event->type() == QEvent::MouseButtonRelease)
            qCWarning(lcTable) << "Ignoring mouse release due to previous drag still existing" << drag;
        else if (event->type() == QEvent::MouseMove)
            qCWarning(lcTable) << "Ignoring mouse move due to previous drag still existing" << drag;
    }

    if (!m_interaction && event->type() == QEvent::MouseButtonPress && card->slot()) {
        Drag *drag = new Drag(event, this, card, selection);
        m_interaction = drag;
        connect(drag, &Drag::finished, this, [this, drag] {
            if (m_interaction == drag)
                m_interaction = nullptr;
        });
        return drag;
    }

    return nullptr;
}

void Table::select(Card *card)
{
    Selection *selection = new Selection(this, card->slot(), card);
    m_interaction = selection;
    connect(selection, &Selection::finished, this, [this, selection] {
        if (m_interaction == selection)
            m_interaction = nullptr;
    });
}

void Table::unselect()
{
    Selection *selection = qobject_cast<Selection *>(m_interaction);
    if (selection) {
        selection->cancel();
        emit feedback()->selectionChanged();
    }
}

void Table::cancelDrag()
{
    Drag *drag = qobject_cast<Drag *>(m_interaction);
    if (drag) {
        qCDebug(lcTable) << "Canceling drag by request";
        drag->abort();
    }
}

void Table::handleSetExpansionToDown(int id, double expansion)
{
    if (!preparing()) {
        qCWarning(lcTable) << "Trying to change expansion to down while Table is not being prepared";
    } else {
        Slot *slot = m_slots.at(id);
        if (slot->expandedRight()) {
            qCWarning(lcTable) << "Can not set delta for expansion to down when expansion to right is set";
        } else if (slot->expandedDown()) {
            slot->setDelta(expansion);
        } else {
            qCWarning(lcTable) << "Can not set delta when expansion is not set";
        }
    }
}

void Table::handleSetExpansionToRight(int id, double expansion)
{
    if (!preparing()) {
        qCWarning(lcTable) << "Trying to change expansion to right while Table is not being prepared";
    } else {
        Slot *slot = m_slots.at(id);
        if (slot->expandedDown()) {
            qCWarning(lcTable) << "Can not set delta for expansion to right when expansion to down is set";
        } else if (slot->expandedRight()) {
            slot->setDelta(expansion);
        } else {
            qCWarning(lcTable) << "Can not set delta when expansion is not set";
        }
    }
}

void Table::handleSlotEmptied()
{
    Slot *slot = qobject_cast<Slot *>(sender());
    if (slot && slot == m_highlightedSlot)
        smudge(HighlightedSlot);
    smudge(HiddenSlots);
    if (!preparing())
        update();
}

void Table::handleWidthChanged(double width)
{
    if (!preparing()) {
        qCWarning(lcTable) << "Trying to table width while Table is not being prepared";
    } else {
        m_tableSize.setWidth(width);
    }
}

void Table::handleHeightChanged(double height)
{
    if (!preparing()) {
        qCWarning(lcTable) << "Trying to table height while Table is not being prepared";
    } else {
        m_tableSize.setHeight(height);
    }
}

void Table::handleSizeChanged()
{
    smudge(BackgroundSize);
    setDirtyCardSize();
}

void Table::setDirtyCardSize()
{
    m_dirtyCardSize = true;
    polish();
}

void Table::updateCardSize()
{
    if (!m_tableSize.isValid())
        return;

    if (m_animation)
        stopAnimation();

    qCDebug(lcTable).nospace() << "Drawing to " << size()
                               << " area with margin of " << m_margin
                               << ", maximum margin of " << m_maximumMargin
                               << ", minimum side margin of " << m_minimumSideMargin
                               << " and table size of " << m_tableSize;

    qreal verticalSpace = width() - m_minimumSideMargin*2.0;
    qreal horizontalSpace = height() - m_margin.height()*2.0;
    qreal maximumWidth = (verticalSpace + m_margin.width()) / m_tableSize.width() - m_margin.width();
    qreal maximumHeight = (horizontalSpace + m_margin.height()) / m_tableSize.height() - m_margin.height();
    QSizeF newCardSize;
    if ((maximumHeight * CardRatio) < maximumWidth) {
        newCardSize = QSizeF(round(maximumHeight * CardRatio), round(maximumHeight));
        m_cardMargin = QSizeF((maximumWidth - newCardSize.width()) / 2.0, 0.0);
    } else {
        newCardSize = QSizeF(round(maximumWidth), round(maximumWidth / CardRatio));
        m_cardMargin = QSizeF(0.0, (maximumHeight - newCardSize.height()) / 2.0);
    }

    if (m_cardSize != newCardSize) {
        m_cardSize = newCardSize;
        m_cardImage = QImage();
        m_doubleSizeImage = QImage();
        QSize size(m_cardSize.width() * 13, m_cardSize.height() * 5);
        bool doubleSize = (m_cardSize.height() * 3 < height() && m_cardSize.width() * 7 < width())
            || (m_cardSize.height() * 3 < width() && m_cardSize.width() * 7 < height());
        emit doRenderCardTexture(size, doubleSize);
    }

    qCDebug(lcTable) << "Calculated maximum space of" << QSizeF(maximumWidth, maximumHeight)
                     << "and card margin of" << m_cardMargin;

    if (m_maximumMargin.width() > 0 && m_cardMargin.width() + m_margin.width() > m_maximumMargin.width())
        m_cardMargin.setWidth(std::max(m_maximumMargin.width() - m_margin.width(), (qreal)0.0F));
    if (m_maximumMargin.height() > 0 && m_cardMargin.height() + m_margin.height() > m_maximumMargin.height())
        m_cardMargin.setHeight(std::max(m_maximumMargin.height() - m_margin.height(), (qreal)0.0F));

    m_cardSpace = m_cardSize + m_cardMargin;
    m_sideMargin = ceil((width() - (m_cardSpace.width()+m_margin.width())*m_tableSize.width() + m_margin.width()) / 2.0);
    if (m_sideMargin < (m_minimumSideMargin - 1))
        qCWarning(lcTable) << "Miscalculated side margin! Current is" << m_sideMargin
                           << "but it should be" << m_minimumSideMargin;

    qCInfo(lcTable) << "Set card dimensions to" << m_cardSize << "with space of" << m_cardSpace
                    << "and margin of" << m_cardMargin;
    qCDebug(lcTable) << "Side margin is" << m_sideMargin;

    m_dirtyCardSize = false;

    for (Slot *slot : m_slots) {
        slot->setZ(0);
        slot->updateDimensions();
    }
    m_manager.forEach([this](Card *card) {
        card->setZ(0);
        card->setParentItem(nullptr);
        card->setSize(m_cardSize);
    });

    smudge(SlotSize);
    if (!preparing())
        update();
}

void Table::disableActions(bool disabled)
{
    // During animation we may not allow any user interaction
    emit actionsDisabled(disabled);
    setEnabled(!disabled);
}

QSGTexture *Table::cardTexture()
{
    return m_cardTexture;
}

bool Table::textureIsDoubleSize() const
{
    return m_doubleResolution && !m_doubleSizeImage.isNull();
}

void Table::setCardTexture(QSGTexture *texture)
{
    if (m_cardTexture)
        deleteTexture(m_cardTexture);
    m_cardTexture = texture;
}

void Table::setPendingCardTexture(QSGTexture *texture)
{
    if (m_pendingCardTexture)
        deleteTexture(m_pendingCardTexture);
    m_pendingCardTexture = texture;
}

void Table::deleteTexture(QSGTexture *texture)
{
    if (window()) {
        window()->scheduleRenderJob(
                new TextureCleaningJob(texture),
                QQuickWindow::AfterRenderingStage);
    } else {
        qWarning(lcTable) << "No window. Deleting texture later. This may leak fds!";
        texture->deleteLater();
    }
}

void Table::connectWindowSignals(QQuickWindow *window)
{
    if (m_previousWindow)
        m_previousWindow->disconnect(this);
    if (window) {
        connect(window, &QQuickWindow::sceneGraphInitialized, this, &Table::createCardTexture);
        connect(window, &QQuickWindow::sceneGraphInvalidated, this, &Table::handleSceneGraphInvalidated);
    }
    m_previousWindow = window;
}

void Table::createCardTexture()
{
    if (!m_cardImage.isNull()) {
        QSGTexture *texture;
        if (textureIsDoubleSize())
            texture = window()->createTextureFromImage(m_doubleSizeImage);
        else
            texture = window()->createTextureFromImage(m_cardImage);
        setPendingCardTexture(texture);
        qCDebug(lcTable) << "New card texture ready for card size of" << m_cardSize;
        polish();
    }
}

void Table::swapCardTexture() {
    if (m_pendingCardTexture) {
        setCardTexture(m_pendingCardTexture);
        m_pendingCardTexture = nullptr;
        emit cardTextureUpdated();
    }
}

void Table::handleCardTextureRendered(QImage image, const QSize &size)
{
    QSize expectedSize(m_cardSize.width() * 13, m_cardSize.height() * 5);
    if (expectedSize == size) {
        m_cardImage = image;
        m_cardSizeInTexture = m_cardSize;
        createCardTexture();
    }
}

void Table::handleDoubleSizeTextureRendered(QImage image, const QSize &size)
{
    QSize expectedSize(m_cardSize.width() * 13, m_cardSize.height() * 5);
    if (expectedSize == size) {
        m_doubleSizeImage = image;
        if (m_doubleResolution)
            createCardTexture();
    }
}

void Table::handleSceneGraphInvalidated()
{
    setCardTexture(nullptr);
    setPendingCardTexture(nullptr);
}

void Table::handleEngineFailure()
{
    setEnabled(false);
}

Table::iterator Table::begin()
{
    return m_slots.begin();
}

Table::iterator Table::end()
{
    return m_slots.end();
}

Slot *Table::findSlotAtPoint(const QPointF point) const
{
    for (Slot *slot : m_slots) {
        if (slot->contains(mapToItem(slot, point)))
            return slot;
    }
    return nullptr;
}

void Table::mousePressEvent(QMouseEvent *event)
{
    qCDebug(lcMouse) << event << "for" << this;

    Slot *slot = findSlotAtPoint(event->pos());
    if (Selection *selection = qobject_cast<Selection *>(m_interaction)) {
        if (slot) {
            selection->finish(slot);
        } else {
            selection->cancel();
            emit feedback()->selectionChanged();
        }
        m_timer.invalidate(); // Ensure that release event does nothing
    } else if (slot) {
        qCDebug(lcMouse) << "Found" << slot << "on click position";
        m_timer.start();
        m_startPoint = event->pos();
    }
}

void Table::mouseReleaseEvent(QMouseEvent *event)
{
    qCDebug(lcMouse) << event << "for" << this;

    auto styleHints = QGuiApplication::styleHints();
    if (m_timer.isValid() && !m_timer.hasExpired(styleHints->startDragTime())
            && (m_startPoint - event->pos()).manhattanLength() < styleHints->startDragDistance()) {
        for (Slot *slot : m_slots) {
            QPointF point = mapToItem(slot, event->pos());
            if (slot->contains(point)) {
                qCDebug(lcTable) << "Detected click on" << slot;
                emit doClick(id(), slot->id());
            }
        }
    }
}

FeedbackEventAttachedType *Table::feedback()
{
    return qobject_cast<FeedbackEventAttachedType*>(qmlAttachedPropertiesObject<FeedbackEvent>(this));
}

QDebug operator<<(QDebug debug, const Table *table)
{
    if (table)
        debug.nospace() << "Table()";
    else
        debug.nospace() << "invalid table";
    return debug.space();
}
