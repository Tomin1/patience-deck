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

#include <QDateTime>
#include "timer.h"
#include "logging.h"

Timer::Timer(QObject *parent)
    : QObject(parent)
    , m_status(TimerStopped)
    , m_elapsed(0)
    , m_lastStarted(0)
{
    m_tick.setInterval(1000);
    connect(&m_tick, &QTimer::timeout, this, &Timer::tick);
}

void Timer::start()
{
    m_status = TimerRunning;
    m_elapsed = 0;
    m_lastStarted = QDateTime::currentMSecsSinceEpoch();
    m_tick.start();
    emit statusChanged();
    qCDebug(lcTimer) << "Timer started at" << m_lastStarted << "msecs since epoch";
}

void Timer::reset()
{
    m_status = TimerStopped;
    m_elapsed = 0;
    m_lastStarted = 0;
    emit tick();
    emit statusChanged();
    qCDebug(lcTimer) << "Timer reset";
}

void Timer::pause()
{
    if (m_status == TimerRunning) {
        m_elapsed += sinceLastStarted();
        m_lastStarted = 0;
        m_status = TimerPaused;
        m_tick.stop();
        emit tick();
        emit statusChanged();
        qCDebug(lcTimer) << "Timer paused at" << m_elapsed << "msecs";
    } else {
        qCWarning(lcTimer) << "Can not pause" << (m_status == TimerPaused ? "paused" : "stopped")
                           << "timer";
    }
}

void Timer::unpause()
{
    if (m_status == TimerPaused) {
        m_lastStarted = QDateTime::currentMSecsSinceEpoch();
        m_status = TimerRunning;
        m_tick.start();
        emit statusChanged();
        qCDebug(lcTimer) << "Timer unpaused at" << m_lastStarted << "msecs since epoch";
    } else {
        qCWarning(lcTimer) << "Can not unpause"
                           << (m_status == TimerRunning ? "running" : "stopped") << "timer";
    }
}

void Timer::stop()
{
    if (m_status == TimerRunning || m_status == TimerPaused) {
        m_elapsed += sinceLastStarted();
        m_lastStarted = 0;
        m_status = TimerStopped;
        m_tick.stop();
        emit tick();
        emit statusChanged();
        qCDebug(lcTimer) << "Timer stopped at" << m_elapsed << "msecs";
    } else {
        qCWarning(lcTimer) << "Can not stop stopped timer";
    }
}

void Timer::extend()
{
    if (m_status == TimerStopped) {
        m_lastStarted = QDateTime::currentMSecsSinceEpoch();
        m_status = TimerRunning;
        m_tick.start();
        emit statusChanged();
        qCDebug(lcTimer) << "Timer extended at" << m_lastStarted << "msecs since epoch";
    } else {
        qCWarning(lcTimer) << "Can not extend"
                           << (m_status == TimerRunning ? "running" : "paused") << "timer";
    }
}

Timer::TimerStatus Timer::status() const
{
    return m_status;
}

QString Timer::elapsed() const
{
    qint64 elapsed = elapsedMSecs();
    qint64 seconds = elapsed / 1000;
    qint64 minutes = seconds / 60;
    qint64 hours = minutes / 60;
    minutes = minutes % 60;
    seconds = seconds % 60;
    return QStringLiteral("%1:%2:%3").arg(hours)
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'));
}

qint64 Timer::elapsedMSecs() const
{
    return m_elapsed + sinceLastStarted();
}

qint64 Timer::sinceLastStarted() const
{
    if (m_status == TimerRunning)
        return QDateTime::currentMSecsSinceEpoch() - m_lastStarted;
    return 0;
}
