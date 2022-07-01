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

/*
 * List of supported patience games.
 * These are tested and any bugs found must be addressed.
 */
QSet<QString> GameList::s_allowlist = {
    // Klondike and its variations
    QStringLiteral("aunt-mary"),
    QStringLiteral("athena"),
    QStringLiteral("gold-mine"),
    QStringLiteral("jumbo"),
    QStringLiteral("klondike"),
    QStringLiteral("saratoga"),
    QStringLiteral("thumb-and-pouch"),
    QStringLiteral("whitehead"),
    // Freecell / Bakers game
    QStringLiteral("bakers-game"),
    QStringLiteral("freecell"),
    QStringLiteral("seahaven"),
    // Spider and similar
    QStringLiteral("spider"),
    QStringLiteral("spiderette"),
    QStringLiteral("scorpion"),
    QStringLiteral("will-o-the-wisp"),
    // Elevator and similar
    QStringLiteral("elevator"),
    QStringLiteral("escalator"),
    QStringLiteral("thirteen"),
    QStringLiteral("treize"),
    QStringLiteral("yield"),
    // Canfield and similar
    QStringLiteral("agnes"),
    QStringLiteral("canfield"),
    QStringLiteral("hamilton"),
    QStringLiteral("kansas"),
    // Auld Lang Syne and Scuffle
    QStringLiteral("auld-lang-syne"),
    QStringLiteral("scuffle"),
    // First law, Fortunes
    QStringLiteral("first-law"),
    QStringLiteral("fortunes"),
    // Yukon, Odessa
    QStringLiteral("odessa"),
    QStringLiteral("yukon"),
    // Osmosis, Peek
    QStringLiteral("osmosis"),
    QStringLiteral("peek"),
    // Accordion, Maze
    QStringLiteral("accordion"),
    QStringLiteral("maze"),
    // Other
    QStringLiteral("backbone"),
    QStringLiteral("bear-river"),
    QStringLiteral("beleaguered-castle"),
    QStringLiteral("bristol"),
    QStringLiteral("camelot"),
    QStringLiteral("carpet"),
    QStringLiteral("clock"),
    QStringLiteral("cruel"),
    QStringLiteral("diamond-mine"),
    QStringLiteral("doublets"),
    QStringLiteral("easthaven"),
    QStringLiteral("eliminator"),
    QStringLiteral("forty-thieves"),
    QStringLiteral("gaps"),
    QStringLiteral("gay-gordons"),
    QStringLiteral("giant"),
    QStringLiteral("glenwood"),
    QStringLiteral("helsinki"),
    QStringLiteral("isabel"),
    QStringLiteral("jamestown"),
    QStringLiteral("king-albert"),
    QStringLiteral("kings-audience"),
    QStringLiteral("lady-jane"),
    QStringLiteral("napoleons-tomb"),
    QStringLiteral("neighbor"),
    QStringLiteral("plait"),
    QStringLiteral("poker"),
    QStringLiteral("quatorze"),
    QStringLiteral("streets-and-alleys"),
    QStringLiteral("ten-across"),
    QStringLiteral("terrace"),
    QStringLiteral("thieves"),
    QStringLiteral("triple-peaks"),
    QStringLiteral("union-square"),
    QStringLiteral("valentine"),
    QStringLiteral("wall"),
    QStringLiteral("westhaven"),
    QStringLiteral("zebra"),
};

QHash<int, QByteArray> GameList::s_roleNames = {
    { Qt::DisplayRole, "display" }, // alias to translated
    { FileNameRole, "filename" },
    { NameRole, "name" },
    { TranslatedRole, "translated" },
    { CapitalizedRole, "capitalized" },
    { SupportedRole, "supported" },
    { SectionRole, "section" },
    { FavoriteRole, "favorite" },
};

GameList::GameList(QObject *parent)
    : QAbstractListModel(parent)
    , m_favoriteConf(Constants::ConfPath + FavoriteConf)
{
    bool showAll = GameList::showAll();
    QDir gameDirectory(Constants::GameDirectory);
    for (auto &entry : gameDirectory.entryList(QStringList() << QStringLiteral("*.scm"),
                                               QDir::Files | QDir::Readable)) {
        if (showAll || isSupported(entry))
            m_games.append(entry);
    }
    std::sort(m_games.begin(), m_games.end(), lessThan);

    m_lastPlayed = Patience::instance()->history().mid(0, ShownLastPlayedGames);

    auto favorites = m_favoriteConf.value(DefaultFavorites).toString().split(';');
    favorites.removeAll(QString());
    std::sort(favorites.begin(), favorites.end(), lessThan);
    m_favorites.swap(favorites);

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
    case SupportedRole:
        return isSupported(getFileName(index.row()));
    case SectionRole:
        return getSection(index.row());
    case FavoriteRole:
        return m_favorites.contains(getFileName(index.row()));
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

QHash<int, QByteArray> GameList::roleNames() const
{
    return s_roleNames;
}

void GameList::setFavorite(int row, bool favorite)
{
    if (row < 0 || row >= rowCount())
        return;

    QString fileName = getFileName(row);
    Section section = getSection(row);
    bool changed = false;

    auto it = std::lower_bound(m_favorites.begin(), m_favorites.end(), fileName, lessThan);
    if (favorite) {
        if (it == m_favorites.end() || *it != fileName) {
            if (!searching()) {
                int pos = std::distance(m_favorites.begin(), it) + m_lastPlayed.count();
                beginInsertRows(QModelIndex(), pos, pos);
                m_favorites.insert(it, fileName);
                endInsertRows();
                qCDebug(lcGameList) << "Inserted favorite to" << pos << "for row" << row
                                    << "with value" << fileName;
            } else {
                m_favorites.insert(it, fileName);
            }
            m_favoriteConf.set(m_favorites.join(';'));
            changed = true;
        }
    } else {
        if (it != m_favorites.end() && *it == fileName) {
            if (!searching()) {
                int pos = std::distance(m_favorites.begin(), it) + m_lastPlayed.count();
                beginRemoveRows(QModelIndex(), pos, pos);
                m_favorites.erase(it);
                endRemoveRows();
                qCDebug(lcGameList) << "Removed favorite from" << pos << "for row" << row
                                    << "with value" << fileName;
            } else {
                m_favorites.erase(it);
            }
            m_favoriteConf.set(m_favorites.join(';'));
            changed = true;
        }
    }
    if (changed) {
        if (!searching()) {
            int lastPlayedRow = section == LastPlayed ? row : m_lastPlayed.indexOf(fileName);
            if (lastPlayedRow != -1)
                emitFavoriteChanged(lastPlayedRow);
            if (section == AllGames) {
                emitFavoriteChanged(row + (favorite ? 1 : -1));
            } else {
                int allGamesRow = std::distance(m_games.begin(),
                        std::lower_bound(m_games.begin(), m_games.end(), fileName, lessThan));
                emitFavoriteChanged(allGamesRow + m_lastPlayed.count() + m_favorites.count());
            }
        } else {
            int allGamesRow = std::distance(m_games.begin(),
                    std::lower_bound(m_games.begin(), m_games.end(), fileName, lessThan));
            int resultsRow = std::distance(m_results.begin(),
                    std::lower_bound(m_results.begin(), m_results.end(), allGamesRow, searchCompareFunction()));
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
    return [this](int a, int b) {
        int iA = translated(m_games[a]).indexOf(m_searchedText, 0, Qt::CaseInsensitive);
        int iB = translated(m_games[b]).indexOf(m_searchedText, 0, Qt::CaseInsensitive);
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
        if (translated(m_games[i]).contains(m_searchedText, Qt::CaseInsensitive))
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

bool GameList::isSupported(const QString &fileName)
{
    return s_allowlist.contains(name(fileName));
}

bool GameList::showAll()
{
    return showAllConf()->value().toBool();
}

void GameList::setShowAll(bool show)
{
    showAllConf()->set(show);
}

MGConfItem *GameList::showAllConf()
{
    static MGConfItem *confItem = nullptr;
    if (!confItem)
        confItem = new MGConfItem(Constants::ConfPath + QStringLiteral("/showAllGames"));
    return confItem;
}

int GameList::supportedCount()
{
    return s_allowlist.count();
}

QString GameList::getFileName(int row) const
{
    if (searching())
        return m_games[m_results[row]];
    if (row < m_lastPlayed.count())
        return m_lastPlayed[row];
    if (row < m_lastPlayed.count() + m_favorites.count())
        return m_favorites[row - m_lastPlayed.count()];
    return m_games[row - m_lastPlayed.count() - m_favorites.count()];
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

bool GameList::lessThan(const QString &a, const QString &b)
{
    return QString::localeAwareCompare(translated(a), translated(b)) < 0;
}
