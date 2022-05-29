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

#ifndef TIMER_H
#define TIMER_H

#include <QObject>
#include <QTimer>

class Timer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString elapsed READ elapsed NOTIFY tick)

public:
    explicit Timer(QObject *parent = nullptr);

    enum TimerStatus {
        TimerPaused,
        TimerRunning,
        TimerStopped
    };

    void start();
    void reset();
    void pause();
    void unpause();
    void stop();
    void extend();
    TimerStatus status() const;
    QString elapsed() const;
    qint64 elapsedMSecs() const;
    void setElapsedMSecs(qint64);

signals:
    void tick();
    void statusChanged();

private:
    qint64 sinceLastStarted() const;

    TimerStatus m_status;
    qint64 m_elapsed;
    qint64 m_lastStarted;
    QTimer m_tick;
};

#endif // TIMER_H
