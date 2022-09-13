/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2022 Tomi Leppänen
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

#ifndef PERFTIMER_H
#define PERFTIMER_H

#include <QElapsedTimer>
#include <QObject>

class PerfTimer : public QObject, public QElapsedTimer
{
    Q_OBJECT
public:
    explicit PerfTimer(QObject *parent = nullptr)
        : QObject(parent) {}
};

#endif // PERFTIMER_H
