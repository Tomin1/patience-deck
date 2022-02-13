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

#ifndef COUNTABLEID_H
#define COUNTABLEID_H

#include <QtGlobal>
#include <QVector>

class CountableId {
    // Construction and destruction is not thread safe, use only in UI thread!
public:
    quint32 id() const;

protected:
    CountableId();
    explicit CountableId(const CountableId *id);
    ~CountableId();

private:
    // QVector is fine, we won't be storing very many ids here
    static QVector<quint32> s_ids;
    static quint32 s_count;

    quint32 m_id;
};

#endif // COUNTABLEID_H
