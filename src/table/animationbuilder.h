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

#ifndef ANIMATIONBUILDER_H
#define ANIMATIONBUILDER_H

#include <functional>
#include <QAnimationGroup>
#include <QEasingCurve>
#include <QPointF>
#include <QQuickItem>
#include <QScopedPointer>

#define RADIAN(a) ((double)(a) * 3.141592653589793)

class AnimationBuilder {
public:
    static AnimationBuilder parallelAnimation(QObject *animationParent = nullptr);
    static AnimationBuilder sequentialAnimation(QObject *animationParent = nullptr);
    operator QAnimationGroup *() const;
    ~AnimationBuilder();

    AnimationBuilder &addPropertyAnimation(QQuickItem *item, const QByteArray &property, const QVariant &value, int duration, QEasingCurve::Type easingCurve);
    inline AnimationBuilder &addXAnimation(QQuickItem *item, int value, int duration, QEasingCurve::Type easingCurve = QEasingCurve::InOutQuad)
    {
        return addPropertyAnimation(item, "x", value, duration, easingCurve);
    }
    inline AnimationBuilder &addYAnimation(QQuickItem *item, int value, int duration, QEasingCurve::Type easingCurve = QEasingCurve::InOutQuad)
    {
        return addPropertyAnimation(item, "y", value, duration, easingCurve);
    }
    AnimationBuilder &addXYAnimation(QQuickItem *item, int x, int y, int duration, QEasingCurve::Type easingCurve = QEasingCurve::InOutQuad);
    AnimationBuilder &addARAnimation(QQuickItem *item, float angle, float radius, int duration, QEasingCurve::Type easingCurve = QEasingCurve::InOutQuad);
    inline AnimationBuilder &addZAnimation(QQuickItem *item, int value)
    {
        return addPropertyAnimation(item, "z", value, 0, QEasingCurve::Linear);
    }
    AnimationBuilder &addPause(int msecs);

    static QAnimationGroup *getXYAnimation(QQuickItem *item, const QPointF &newPosition, int duration, QEasingCurve::Type easingCurve = QEasingCurve::InOutQuad);

    class ParallelGroupGuard {
    public:
        ParallelGroupGuard(AnimationBuilder *builder, bool newGroup = false);
        ~ParallelGroupGuard();

    private:
        AnimationBuilder *m_builder;
        int m_newGroup;
    };

private:
    enum AnimationType {
        ParallelAnimation,
        SequentialAnimation,
    };

    AnimationBuilder(AnimationType type, QObject *animationParent);
    inline QAnimationGroup *tip() { return m_stack.last(); }

    mutable QAnimationGroup *m_base;
    mutable QList<QAnimationGroup *> m_stack;
};

#endif // ANIMATIONBUILDER_H
