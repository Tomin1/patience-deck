/*
 * Tests for Patience Deck itertools
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

#include <iostream>
#include <random>
#include <string>
#include <unordered_set>
#include <vector>
#include "itertools.h"

#define ASSERT_TRUE(test) do { \
    if (!(test)) { \
        cout << "assertion (" << #test << ") is true failed" << endl; \
        return false; \
    } \
} while (0)
#define ASSERT_FALSE(test) do { \
    if (test) { \
        cout << "assertion (" << #test << ") is false failed" << endl; \
        return false; \
        return false; \
    } \
} while (0)
#define ASSERT_SAME(a, b) do { \
    if ((a) != (b)) { \
        cout << "assertion (" << #a << " == " << #b << ") failed" << endl; \
        return false; \
    } \
} while (0)
#define SUCCESS() do { return true; } while (0)

namespace {
    using std::advance;
    using std::cout;
    using std::endl;
    using std::function;
    using std::get;
    using std::mt19937;
    using std::string;
    using std::tuple;
    using std::unitbuf;
    using std::unordered_set;
    using std::vector;

    const vector<int> data { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
} // namespace

bool test_range_simple()
{
    Range range(0, 10);
    auto it1 = range.begin();
    auto it2 = data.begin();
    while (it1 != range.end() || it2 != data.end()) {
        ASSERT_SAME(*it1, *it2);
        ++it1, ++it2;
    }
    ASSERT_FALSE(it1 != range.end());
    ASSERT_TRUE(it2 == data.end());
    SUCCESS();
}

bool test_range_consistent_begin()
{
    Range range(0, 100);
    auto first = range.begin();
    auto second = range.begin();
    ASSERT_FALSE(first != second);
    ASSERT_TRUE(++first != second);
    ASSERT_FALSE(first != ++second);
    SUCCESS();
}

bool test_range_consistent_end()
{
    Range range(10, 12);
    ASSERT_SAME(range.end(), range.end());
    auto it = range.begin();
    advance(it, 4);
    ASSERT_SAME(it, range.end());
    SUCCESS();
}

bool test_range_empty()
{
    Range range(1, 1);
    ASSERT_SAME(range.begin(), range.end());
    SUCCESS();
}

bool test_indexed_simple()
{
    vector<int> indices { 8, 9, 2, 5, 4, 3, 6, 7, 1, 0 };
    vector<int> values(data); 
    IndexedIterator<vector<int>::iterator> iterator(values.begin(), values.end(), indices);
    auto it1 = iterator.begin();
    auto it2 = indices.begin();
    while (it1 != iterator.end() || it2 != indices.end()) {
        ASSERT_SAME(*it1, *it2);
        ++it1, ++it2;
    }
    ASSERT_FALSE(it1 != iterator.end());
    ASSERT_TRUE(it2 == indices.end());
    SUCCESS();
}

bool test_indexed_consistent_begin()
{
    IndexedIterator<vector<int>::const_iterator> iterator(data.begin(), data.end(), data);
    auto first = iterator.begin();
    auto second = iterator.begin();
    ASSERT_FALSE(first != second);
    ASSERT_TRUE(++first != second);
    ASSERT_FALSE(first != ++second);
    SUCCESS();
}

bool test_indexed_consistent_end()
{
    IndexedIterator<vector<int>::const_iterator> iterator(data.begin(), data.end(), data);
    ASSERT_SAME(iterator.end(), iterator.end());
    auto it = iterator.begin();
    advance(it, 10);
    ASSERT_SAME(it, iterator.end());
    SUCCESS();
}

bool test_indexed_empty()
{
    IndexedIterator<vector<int>::const_iterator> iterator(data.begin(), data.begin(), std::vector<int>());
    ASSERT_SAME(iterator.begin(), iterator.end());
    SUCCESS();
}

bool test_shuffled_simple(int value, const vector<int> &expected)
{
    vector<int> values(data);
    mt19937 generator(value);
    IndexedIterator<vector<int>::iterator> iterator = shuffled(values, generator);
    auto it1 = iterator.begin();
    auto it2 = expected.begin();
    while (it1 != iterator.end() || it2 != expected.end()) {
        ASSERT_SAME(*it1, *it2);
        ++it1, ++it2;
    }
    ASSERT_FALSE(it1 != iterator.end());
    ASSERT_TRUE(it2 == expected.end());
    SUCCESS();
}

bool test_shuffled_simple_0()
{
    const vector<int> expected { 0, 2, 1, 5, 9, 8, 4, 7, 6, 3 };
    return test_shuffled_simple(0, expected);
}

bool test_shuffled_simple_42()
{
    const vector<int> expected { 1, 6, 7, 0, 5, 9, 8, 2, 3, 4 };
    return test_shuffled_simple(42, expected);
}

bool test_grouped_simple()
{
    vector<int> groups { 3, 2, 4, 1 };
    vector<int> values(data);
    GroupedIterator<vector<int>::iterator> iterator(values.begin(), values.end(), groups);
    vector<vector<int>> expected {
        { 0, 1, 2 },
        { 3, 4 },
        { 5, 6, 7, 8 },
        { 9 },
    };
    auto itA1 = iterator.begin();
    auto itA2 = expected.begin();
    while (itA1 != iterator.end() || itA2 != expected.end()) {
        auto itB1 = (*itA1).begin();
        auto itB2 = itA2->begin();
        while (itB1 != (*itA1).end() || itB2 != itA2->end()) {
            ASSERT_SAME(*itB1, *itB2);
            ++itB1, ++itB2;
        }
        ASSERT_FALSE(itB1 != (*itA1).end());
        ASSERT_TRUE(itB2 == itA2->end());
        ++itA1, ++itA2;
    }
    ASSERT_FALSE(itA1 != iterator.end());
    ASSERT_TRUE(itA2 == expected.end());
    SUCCESS();
}

bool test_grouped_consistent_begin()
{
    const vector<int> groups { 2, 2, 2, 2, 2 };
    GroupedIterator<vector<int>::const_iterator> iterator(data.begin(), data.end(), groups);
    auto first = iterator.begin();
    auto second = iterator.begin();
    ASSERT_FALSE(first != second);
    ASSERT_TRUE(++first != second);
    ASSERT_FALSE(first != ++second);
    SUCCESS();
}

bool test_grouped_consistent_end()
{
    const vector<int> groups { 2, 2, 2, 2, 2 };
    GroupedIterator<vector<int>::const_iterator> iterator(data.begin(), data.end(), groups);
    ASSERT_SAME(iterator.end(), iterator.end());
    auto it = iterator.begin();
    advance(it, 5);
    ASSERT_SAME(it, iterator.end());
    SUCCESS();
}

bool test_grouped_empty()
{
    GroupedIterator<vector<int>::const_iterator> iterator(data.begin(), data.begin(), std::vector<int>());
    ASSERT_SAME(iterator.begin(), iterator.end());
    SUCCESS();
}

bool test_groupedfunc_simple()
{
    unordered_set<int> groups { 3, 5, 8 };
    vector<int> values(data);
    GroupedIterator<vector<int>::iterator> iterator = grouped<vector<int>>(
            values, [&groups](int value) { return groups.find(value) != groups.end(); });
    vector<vector<int>> expected {
        { 0, 1, 2, 3 },
        { 4, 5 },
        { 6, 7, 8 },
        { 9 },
    };
    auto itA1 = iterator.begin();
    auto itA2 = expected.begin();
    while (itA1 != iterator.end() || itA2 != expected.end()) {
        auto itB1 = (*itA1).begin();
        auto itB2 = itA2->begin();
        while (itB1 != (*itA1).end() || itB2 != itA2->end()) {
            ASSERT_SAME(*itB1, *itB2);
            ++itB1, ++itB2;
        }
        ASSERT_FALSE(itB1 != (*itA1).end());
        ASSERT_TRUE(itB2 == itA2->end());
        ++itA1, ++itA2;
    }
    ASSERT_FALSE(itA1 != iterator.end());
    ASSERT_TRUE(itA2 == expected.end());
    SUCCESS();
}

bool test_groupedfunc_empty()
{
    std::vector<int> data;
    GroupedIterator<vector<int>::iterator> iterator = grouped(data, [](int value) {
        (void)value;
        return true;
    });
    ASSERT_SAME(iterator.begin(), iterator.end());
    SUCCESS();
}

vector<tuple<string, function<bool()>>> tests = {
    { "range/simple", test_range_simple },
    { "range/consistent_begin", test_range_consistent_begin },
    { "range/consistent_end", test_range_consistent_end },
    { "range/empty", test_range_empty },
    { "indexed/simple", test_indexed_simple },
    { "indexed/consistent_begin", test_indexed_consistent_begin },
    { "indexed/consistent_end", test_indexed_consistent_end },
    { "indexed/empty", test_indexed_empty },
    { "shuffled/simple(0)", test_shuffled_simple_0 },
    { "shuffled/simple(42)", test_shuffled_simple_42 },
    { "grouped/simple", test_grouped_simple },
    { "grouped/consistent_begin", test_grouped_consistent_begin },
    { "grouped/consistent_end", test_grouped_consistent_end },
    { "grouped/empty", test_grouped_empty },
    { "groupedfunc/simple", test_groupedfunc_simple },
    { "groupedfunc/empty", test_groupedfunc_empty },
};

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    cout << unitbuf;

    uint count = 0;
    for (auto test : tests) {
        string name = get<0>(test);
        cout << "Test '" << name << "' ";
        bool success = get<1>(test)();
        if (success)
            cout << "succeeded" << endl;
        count += success;
    }

    cout << count << "/" << tests.size() << " tests succeeded" << endl;
    return tests.size() - count;
}
