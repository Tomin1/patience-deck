/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2021 Tomi Leppänen
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

#ifndef FEEDBACKEVENT_H
#define FEEDBACKEVENT_H

#include <QObject>
#include <QQmlEngine>
#include <QtQml>

class FeedbackEventAttachedType : public QObject
{
    Q_OBJECT

public:
    explicit FeedbackEventAttachedType(QObject *parent);

signals:
    void clicked();
    void dropSucceeded();
    void dropFailed();
    void selectionChanged();
};

class FeedbackEvent : public QObject
{
    Q_OBJECT

public:
    static FeedbackEventAttachedType *qmlAttachedProperties(QObject *object)
    {
        return new FeedbackEventAttachedType(object);
    }
};

QML_DECLARE_TYPEINFO(FeedbackEvent, QML_HAS_ATTACHED_PROPERTIES)

#endif // FEEDBACKEVENT_H
