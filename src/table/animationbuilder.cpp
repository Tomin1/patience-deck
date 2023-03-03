/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2023 Tomi Leppänen
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

#include <cmath>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include "animationbuilder.h"
#include "logging.h"

AnimationBuilder::ParallelGroupGuard::ParallelGroupGuard(AnimationBuilder *builder, bool newGroup)
    : m_builder(builder)
    , m_newGroup(0)
{
    if (newGroup || !qobject_cast<QParallelAnimationGroup *>(builder->tip())) {
        auto *group = new QParallelAnimationGroup(m_builder->tip());
        m_builder->tip()->addAnimation(group);
        m_builder->m_stack.append(group);
        m_newGroup = m_builder->m_stack.count();
    }
}

AnimationBuilder::ParallelGroupGuard::~ParallelGroupGuard()
{
    if (m_newGroup) {
        int group = m_builder->m_stack.count();
        if (group == 1) {
            qCCritical(lcAnimation) << "Cannot pop the last item in animation group stack!";
        } else {
            if (group != m_newGroup)
                qCCritical(lcAnimation) << "Unexpected group number! Are you sure stacks are right?"
                                        << "Got" << group << "while expecting" << m_newGroup;
            m_builder->m_stack.removeLast();
        }
    }
}

AnimationBuilder::AnimationBuilder(AnimationType type, QObject *animationParent)
{
    if (type == ParallelAnimation)
        m_base = new QParallelAnimationGroup(animationParent);
    else
        m_base = new QSequentialAnimationGroup(animationParent);
    m_stack.append(m_base);
}

AnimationBuilder::~AnimationBuilder()
{
    delete m_base;
}

AnimationBuilder::operator QAnimationGroup *() const
{
    auto *group = m_base;
    m_base = nullptr;
    m_stack.clear();
    return group;
}

AnimationBuilder AnimationBuilder::parallelAnimation(QObject *animationParent)
{
    return AnimationBuilder{ParallelAnimation, animationParent};
}

AnimationBuilder AnimationBuilder::sequentialAnimation(QObject *animationParent)
{
    return AnimationBuilder{SequentialAnimation, animationParent};
}

AnimationBuilder &AnimationBuilder::addPropertyAnimation(QQuickItem *item, const QByteArray &property, const QVariant &value, int duration, QEasingCurve::Type easingCurve)
{
    auto *animation = new QPropertyAnimation(item, property, tip());
    animation->setEndValue(value);
    animation->setDuration(duration);
    animation->setEasingCurve(easingCurve);
    tip()->addAnimation(animation);
    return *this;
}

AnimationBuilder &AnimationBuilder::addXYAnimation(QQuickItem *item, int x, int y, int duration, QEasingCurve::Type easingCurve)
{
    ParallelGroupGuard guard(this);
    addXAnimation(item, x, duration, easingCurve);
    addYAnimation(item, y, duration, easingCurve);
    return *this;
}

AnimationBuilder &AnimationBuilder::addARAnimation(QQuickItem *item, float angle, float radius, int duration, QEasingCurve::Type easingCurve)
{
    addXYAnimation(item, item->x() + std::cos(RADIAN(angle)) * radius, item->y() + std::sin(RADIAN(angle)) * radius, duration, easingCurve);
    return *this;
}

AnimationBuilder &AnimationBuilder::addPause(int msecs)
{
    auto *group = qobject_cast<QSequentialAnimationGroup *>(tip());
    if (!group) {
        qCCritical(lcAnimation) << "Cannot add pause as the tip is not a sequential group";
        return *this;
    }
    group->addPause(msecs);
    return *this;
}

QAnimationGroup *AnimationBuilder::getXYAnimation(QQuickItem *item, const QPointF &newPosition, int duration, QEasingCurve::Type easingCurve)
{
    return parallelAnimation(item)
            .addXAnimation(item, newPosition.x(), duration, easingCurve)
            .addYAnimation(item, newPosition.y(), duration, easingCurve);
}
