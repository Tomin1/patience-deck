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

#include "countableid.h"

// Zero is special, it's never allowed as an id
QVector<quint32> CountableId::s_ids = { 0 };

quint32 CountableId::s_count = 0;

CountableId::CountableId()
    : m_id(++s_count)
{
    // Skip values that are already in use or not acceptable
    while (s_ids.contains(m_id))
        m_id = ++s_count;
    s_ids.append(m_id);
}

CountableId::CountableId(const CountableId *id)
    : m_id(id->m_id)
{
    // Creates a copy of this countable with the very same id
}

CountableId::~CountableId()
{
    s_ids.removeAll(m_id);
}

quint32 CountableId::id() const
{
    return m_id;
}
