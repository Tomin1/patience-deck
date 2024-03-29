/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2020-2022 Tomi Leppänen
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

#include <libintl.h>
#include <QDir>
#include <QSet>
#include "logging.h"
#include "patience.h"
#include "gamelist.h"
#include "constants.h"

namespace {

const QString FavoriteConf = QStringLiteral("/favorites");
int ShownLastPlayedGames = 5;
// Default favourites, keep sorted
const QString DefaultFavorites = QStringLiteral("freecell.scm;helsinki.scm;klondike.scm;spider.scm");

} // namespace

QHash<int, QByteArray> GameList::s_roleNames = {
    { Qt::DisplayRole, "display" }, // alias to translated
    { FileNameRole, "filename" },
    { NameRole, "name" },
    { TranslatedRole, "translated" },
    { CapitalizedRole, "capitalized" },
    { SectionRole, "section" },
    { FavoriteRole, "favorite" },
    { MatchedByRole, "matchedBy" },
};

GameList::GameList(QObject *parent)
    : QAbstractListModel(parent)
    , m_favoriteConf(Constants::ConfPath + FavoriteConf)
{
    m_games = QVector<QString>::fromList(gamesList());
    std::sort(m_games.begin(), m_games.end(), lessThan);

    m_lastPlayed.reserve(ShownLastPlayedGames);
    auto history = Patience::instance()->history();
    for (auto it = history.begin(); it != history.end() && m_lastPlayed.count() < ShownLastPlayedGames; ++it) {
        int index = findIndex(*it);
        if (index >= 0)
            m_lastPlayed.append(index);
    }

    auto favorites = m_favoriteConf.value(DefaultFavorites).toString().split(';');
    for (auto it = favorites.begin(); it != favorites.end(); ++it) {
        int index = findIndex(*it);
        if (index >= 0)
            m_favorites.append(index);
    }
    std::sort(m_favorites.begin(), m_favorites.end());

    connect(&m_favoriteConf, &MGConfItem::valueChanged, this, [&] {
        qCDebug(lcGameList) << "Saved favorites:" << m_favoriteConf.value().toString();
    });
}

void GameList::classBegin()
{
}

void GameList::componentComplete()
{
    if (!m_searchedText.isEmpty()) {
        resetSearch();
        filterSearch();
    }
}

int GameList::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (searching())
        return m_results.count();
    return m_lastPlayed.count() + m_favorites.count() + m_games.count();
}

QVariant GameList::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= rowCount())
        return QVariant();

    switch (role) {
    case DisplayRole:
    case TranslatedRole:
        return translated(getFileName(index.row()));
    case FileNameRole:
        return getFileName(index.row());
    case NameRole:
        return name(getFileName(index.row()));
    case CapitalizedRole:
        return capitalized(getFileName(index.row()));
    case SectionRole:
        return getSection(index.row());
    case FavoriteRole:
        return m_favorites.contains(getIndex(index.row()));
    case MatchedByRole:
        return matchedBy(index.row());
    default:
        return QVariant();
    }
}

QString GameList::capitalized(const QString &fileName)
{
    QString displayName(name(fileName));
    bool startOfWord = true;
    for (int i = 0; i < displayName.length(); i++) {
        if (startOfWord) {
            displayName[i] = QChar(displayName[i]).toUpper();
            startOfWord = false;
        } else if (displayName[i] == '-') {
            startOfWord = true;
            displayName[i] = ' ';
        }
    }
    return displayName;
}

QString GameList::translated(const QString &fileName)
{
    return QString(gettext(capitalized(fileName).toUtf8().constData()));
}

QString GameList::name(const QString &fileName)
{
    return fileName.left(fileName.length()-4);
}

GameList::MatchedBy GameList::matchedBy(int row) const
{
    if (searching()) {
        QString fileName = getFileName(row);
        if (translated(fileName).contains(m_searchedText, Qt::CaseInsensitive))
            return TranslatedName;
        if (capitalized(fileName).contains(m_searchedText, Qt::CaseInsensitive))
            return CapitalizedName;
    }
    return None;
}

QHash<int, QByteArray> GameList::roleNames() const
{
    return s_roleNames;
}

void GameList::setFavorite(int row, bool favorite)
{
    if (row < 0 || row >= rowCount())
        return;

    int index = getIndex(row);
    Section section = getSection(row);
    bool changed = false;

    auto it = std::lower_bound(m_favorites.begin(), m_favorites.end(), index);
    if (favorite) {
        if (it == m_favorites.end() || *it != index) {
            if (!searching()) {
                int pos = std::distance(m_favorites.begin(), it) + m_lastPlayed.count();
                beginInsertRows(QModelIndex(), pos, pos);
                m_favorites.insert(it, index);
                endInsertRows();
                qCDebug(lcGameList) << "Inserted favorite to" << pos << "for row" << row
                                    << "with value" << m_games[pos];
            } else {
                m_favorites.insert(it, index);
            }
            updateFavorites();
            changed = true;
        }
    } else {
        if (it != m_favorites.end() && *it == index) {
            if (!searching()) {
                int pos = std::distance(m_favorites.begin(), it) + m_lastPlayed.count();
                beginRemoveRows(QModelIndex(), pos, pos);
                m_favorites.erase(it);
                endRemoveRows();
                qCDebug(lcGameList) << "Removed favorite from" << pos << "for row" << row
                                    << "with value" << m_games[pos];
            } else {
                m_favorites.erase(it);
            }
            updateFavorites();
            changed = true;
        }
    }
    if (changed) {
        if (!searching()) {
            int lastPlayedRow = section == LastPlayed ? row : m_lastPlayed.indexOf(index);
            if (lastPlayedRow != -1)
                emitFavoriteChanged(lastPlayedRow);
            if (section == AllGames) {
                emitFavoriteChanged(row + (favorite ? 1 : -1));
            } else {
                emitFavoriteChanged(index + m_lastPlayed.count() + m_favorites.count());
            }
        } else {
            int resultsRow = std::distance(m_results.begin(),
                    std::lower_bound(m_results.begin(), m_results.end(), index, searchCompareFunction()));
            emitFavoriteChanged(resultsRow);
        }
    }
}

bool GameList::searching() const
{
    return !m_searchedText.isEmpty();
}

QString GameList::searchedText() const
{
    return m_searchedText;
}

void GameList::setSearchedText(const QString &text)
{
    if (m_searchedText != text) {
        if (text.isEmpty()) {
            m_searchedText.clear();
            clearSearch();
        } else {
            int previousCount = (m_searchedText.isEmpty()
                ? m_lastPlayed.count() + m_favorites.count() + m_games.count()
                : m_results.count());
            if (m_searchedText.isEmpty() || !text.startsWith(m_searchedText))
                resetSearch();
            m_searchedText = text;
            filterSearch();
            emitChangedEntries(previousCount, m_results.count());
        }
        emit searchedTextChanged();
    }
}

std::function<bool(int, int)> GameList::searchCompareFunction() const
{
    /* This comparison function weights following:
     * 1. Matches in translated name are better.
     * 2. Matches earlier in translated name are better.
     * 3. Otherwise equal matches are ordered by their translated names.
     * If there are matches that match only by their capitalized name,
     * those are at the bottom of the list ordered by their translated name.
     * It's expected that there won't be many of those so it doesn't
     * make sense to put a lot of effort into ordering them and besides
     * this looks reasonably good anyway.
     */
    return [this](int a, int b) {
        int iA = translated(m_games[a]).indexOf(m_searchedText, 0, Qt::CaseInsensitive);
        int iB = translated(m_games[b]).indexOf(m_searchedText, 0, Qt::CaseInsensitive);
        if (iA == -1 && iB != -1)
            return false;
        else if (iB == -1 && iA != -1)
            return true;
        if (iA == iB)
            return a < b;
        return iA < iB;
    };
}

void GameList::filterSearch()
{
    qCDebug(lcGameList) << m_results.count() << "games to search from";
    QVector<int> results;
    for (int i : m_results) {
        const QString &fileName = m_games[i];
        if (translated(fileName).contains(m_searchedText, Qt::CaseInsensitive) ||
                capitalized(fileName).contains(m_searchedText, Qt::CaseInsensitive))
            results.append(i);
    }
    qCInfo(lcGameList) << results.count() << "matched to searched text";
    std::sort(results.begin(), results.end(), searchCompareFunction());
    m_results.swap(results);
}

void GameList::clearSearch()
{
    emitChangedEntries(m_results.count(), m_lastPlayed.count() + m_favorites.count() + m_games.count());
    m_results.clear();
    qCInfo(lcGameList) << "Cleared search";
}

void GameList::resetSearch()
{
    m_results.clear();
    m_results.reserve(m_games.count());
    for (int i = 0; i < m_games.count(); i++)
        m_results.append(i);
}

void GameList::emitChangedEntries(int oldCount, int newCount)
{
    if (oldCount > 0 && newCount > 0) {
        emit dataChanged(createIndex(0, 0), createIndex(std::min(oldCount, newCount) - 1, 0));
        qCDebug(lcGameList) << "Data changed for items between 0 and" << std::min(oldCount, newCount) - 1;
    }
    if (newCount < oldCount) {
        beginRemoveRows(QModelIndex(), newCount, oldCount - 1);
        qCDebug(lcGameList) << "Removed items between" << newCount << "and" << oldCount - 1;
        endRemoveRows();
    } else if (newCount > oldCount) {
        beginInsertRows(QModelIndex(), oldCount, newCount - 1);
        qCDebug(lcGameList) << "Inserted items between" << oldCount << "and" << newCount - 1;
        endInsertRows();
    }
}

QStringList GameList::gamesList()
{
    QDir gameDirectory(Constants::GameDirectory);
    return gameDirectory.entryList(QStringList() << QStringLiteral("*.scm"), QDir::Files | QDir::Readable);
}

int GameList::count()
{
    static int count = gamesList().count();
    return count;
}

int GameList::findIndex(const QString &fileName) const
{
    auto it = std::lower_bound(m_games.begin(), m_games.end(), fileName, lessThan);
    if (it == m_games.end())
        return -1;
    return std::distance(m_games.begin(), it);
}

int GameList::getIndex(int row) const
{
    if (searching())
        return m_results[row];
    if (row < m_lastPlayed.count())
        return m_lastPlayed[row];
    if (row < m_lastPlayed.count() + m_favorites.count())
        return m_favorites[row - m_lastPlayed.count()];
    return row - m_lastPlayed.count() - m_favorites.count();
}

QString GameList::getFileName(int row) const
{
    return m_games[getIndex(row)];
}

GameList::Section GameList::getSection(int row) const
{
    if (searching())
        return SearchResults;
    if (row < m_lastPlayed.count())
        return LastPlayed;
    if (row < m_lastPlayed.count() + m_favorites.count())
        return Favorites;
    return AllGames;
}

void GameList::emitFavoriteChanged(int row)
{
    auto index = createIndex(row, 0);
    emit dataChanged(index, index, QVector<int>() << FavoriteRole);
}

void GameList::updateFavorites()
{
    QStringList favorites;
    favorites.reserve(m_favorites.count());
    for (int index : m_favorites)
        favorites.append(m_games[index]);
    m_favoriteConf.set(favorites.join(';'));
}

bool GameList::lessThan(const QString &a, const QString &b)
{
    return QString::localeAwareCompare(translated(a), translated(b)) < 0;
}
