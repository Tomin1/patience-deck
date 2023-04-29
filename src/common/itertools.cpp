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

#include "itertools.h"

Range::iterator::iterator(int current, int max)
    : m_current(current)
    , m_max(max)
{
}

void Range::iterator::operator=(const Range::iterator &other)
{
    m_current = other.m_current;
    m_max = other.m_max;
}

bool Range::iterator::operator!=(const Range::iterator &other) const
{
    return m_current != other.m_current || m_max != other.m_max;
}

Range::iterator &Range::iterator::operator++()
{
    if (m_current < m_max)
        ++m_current;
    return *this;
}

int Range::iterator::operator*() const
{
    return m_current;
}

Range::Range(int max)
    : m_first(0)
    , m_max(max)
{
    if (m_first > m_max)
        abort();
}

Range::Range(int first, int max)
    : m_first(first)
    , m_max(max)
{
    if (m_first > m_max)
        abort();
}

Range::iterator Range::begin() const
{
    return iterator(m_first, m_max);
}

Range::iterator Range::end() const
{
    return iterator(m_max, m_max);
}
