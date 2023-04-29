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

#ifndef ITERTOOLS_H
#define ITERTOOLS_H

#include <algorithm>
#include <functional>
#include <iterator>
#include <queue>
#include <vector>

class Range {
public:
    class iterator
    {
    public:
        using difference_type = int;
        using value_type = int;
        using pointer = int *;
        using reference = int &;
        using iterator_category = std::input_iterator_tag;
        iterator(int current, int max);
        void operator=(const iterator &other);
        bool operator!=(const iterator &other) const;
        iterator &operator++();
        value_type operator*() const;

    private:
        int m_current;
        int m_max;
    };

    explicit Range(int max);
    Range(int first, int max);
    iterator begin() const;
    iterator end() const;

private:
    int m_first;
    int m_max;
};


template <typename Iterator>
class IndexedIterator {
public:
    class iterator
    {
    public:
        using difference_type = int;
        using value_type = typename std::iterator_traits<Iterator>::value_type;
        using pointer = typename std::iterator_traits<Iterator>::pointer;
        using reference = typename std::iterator_traits<Iterator>::reference;
        using iterator_category = std::input_iterator_tag;
        iterator(Iterator begin, const std::queue<int> indices)
            : m_begin(begin)
            , m_indices(indices)
            , m_index(0)
        {
            if (m_indices.empty())
                abort();
            m_index = indices.front();
            m_indices.pop();
        }
        void operator=(const iterator &other)
        {
            m_begin = other.m_begin;
            m_indices = other.m_indices;
            m_index = other.m_index;
        }
        bool operator!=(const iterator &other) const
        {
            if (m_indices.empty() && other.m_indices.empty())
                return false;
            return m_begin != other.m_begin
                || m_indices != other.m_indices
                || m_index != other.m_index;
        }
        iterator &operator++()
        {
            if (m_indices.empty())
                abort();
            m_index = m_indices.front();
            m_indices.pop();
            return *this;
        }
        reference operator*() const
        {
            if (m_indices.empty() || m_index >= m_indices.back())
                abort();
            return m_begin[m_index];
        }

    private:
        Iterator m_begin;
        std::queue<int> m_indices;
        int m_index;
    };

    IndexedIterator(Iterator begin, Iterator end, const std::vector<int> &indices)
        : m_begin(begin)
        , m_indices(indices)
        , m_end(std::distance(begin, end))
    {
    }
    iterator begin() const
    {
        std::queue<int> q = queueFromVector(m_indices);
        q.push(m_end);
        return iterator(m_begin, q);
    }
    iterator end() const {
        std::queue<int> q;
        q.push(m_end);
        return iterator(m_begin, q);
    }

private:
    static std::queue<int> queueFromVector(const std::vector<int> &v)
    {
        std::queue<int> q;
        for (int index : v)
            q.push(index);
        return q;
    }

    const Iterator m_begin;
    const std::vector<int> m_indices;
    const int m_end;
};

template <typename Iterator>
class GroupedIterator {
public:
    class slice
    {
    public:
        slice(Iterator begin, int count)
            : m_begin(begin)
            , m_count(count)
        {}
        void operator=(const slice &other)
        {
            m_begin = other.m_begin;
            m_count = other.m_count;
        }
        Iterator begin() const { return m_begin; }
        Iterator end() const
        {
            Iterator it(m_begin);
            if (m_count > 0)
                std::advance(it, m_count);
            return it;
        }

    private:
        Iterator m_begin;
        int m_count;
    };

    class iterator
    {
    public:
        using difference_type = int;
        using value_type = slice;
        using pointer = slice *;
        using reference = slice &;
        using iterator_category = std::input_iterator_tag;
        iterator(Iterator begin, std::queue<int> groups)
            : m_current(begin)
            , m_groups(groups)
        {
        }
        void operator=(const iterator &other)
        {
            m_current = other.m_current;
            m_groups = other.m_groups;
        }
        bool operator!=(const iterator &other) const
        {
            if (m_groups.empty() && other.m_groups.empty())
                return false;
            return m_current != other.m_current
                || m_groups != other.m_groups;
        }
        iterator &operator++()
        {
            if (m_groups.empty())
                abort();
            if (m_groups.front() > 0)
                std::advance(m_current, m_groups.front());
            m_groups.pop();
            return *this;
        }
        value_type operator*() const
        {
            return slice(m_current, m_groups.front());
        }

    private:
        Iterator m_current;
        std::queue<int> m_groups;
    };

    GroupedIterator(Iterator begin, Iterator end, std::vector<int> groups)
        : m_begin(begin)
        , m_end(end)
        , m_groups(groups)
    {
    }
    iterator begin() const
    {
        return iterator(m_begin, queueFromVector(m_groups));
    }
    iterator end() const
    {
        return iterator(m_end, std::queue<int>());
    }

private:
    static std::queue<int> queueFromVector(const std::vector<int> &v)
    {
        std::queue<int> q;
        for (int index : v)
            q.push(index);
        return q;
    }

    const Iterator m_begin;
    const Iterator m_end;
    const std::vector<int> m_groups;
};

template<typename Iterable, typename RandomGenerator>
IndexedIterator<typename Iterable::iterator> shuffled(Iterable &iterable, RandomGenerator &generator)
{
    Range range(std::distance(iterable.begin(), iterable.end()));
    std::vector<int> indices(range.begin(), range.end());
    std::shuffle(indices.begin(), indices.end(), generator);
    return IndexedIterator<typename Iterable::iterator>(iterable.begin(), iterable.end(), indices);
}

template<typename V>
using Grouper = std::function<bool(const V)>;

template<class Iterable>
GroupedIterator<typename Iterable::iterator> grouped(Iterable &iterable, Grouper<typename Iterable::iterator::reference> grouper)
{
    std::vector<int> groups{ 0 };
    for (typename Iterable::iterator::reference value : iterable) {
        ++groups.back();
        if (grouper(value))
            groups.push_back(0);
    }
    if (!groups.empty() && groups.back() == 0)
        groups.pop_back();
    return GroupedIterator<typename Iterable::iterator>(iterable.begin(), iterable.end(), groups);
}

#endif // ITERTOOLS_H
