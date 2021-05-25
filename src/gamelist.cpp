/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2020-2021 Tomi Leppänen
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

#include <QDir>
#include <QSet>
#include "logging.h"
#include "patience.h"
#include "gamelist.h"
#include "constants.h"

namespace {

const QString FavoriteConf = QStringLiteral("/favorites");
int ShownLastPlayedGames = 5;

} // namespace

/*
 * List of supported patience games.
 * These are tested and any bugs found must be addressed.
 */
QSet<QString> GameList::s_allowlist = {
    // Klondike and its variations
    QStringLiteral("aunt-mary"),
    QStringLiteral("athena"),
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
    // Other
    QStringLiteral("bear-river"),
    QStringLiteral("beleaguered-castle"),
    QStringLiteral("bristol"),
    QStringLiteral("camelot"),
    QStringLiteral("carpet"),
    QStringLiteral("clock"),
    QStringLiteral("easthaven"),
    QStringLiteral("eliminator"),
    QStringLiteral("forty-thieves"),
    QStringLiteral("giant"),
    QStringLiteral("helsinki"),
    QStringLiteral("isabel"),
    QStringLiteral("lady-jane"),
    QStringLiteral("napoleons-tomb"),
    QStringLiteral("poker"),
    QStringLiteral("ten-across"),
    QStringLiteral("triple-peaks"),
    QStringLiteral("valentine"),
    QStringLiteral("westhaven"),
    QStringLiteral("yukon"),
    QStringLiteral("zebra"),
};

QHash<int, QByteArray> GameList::s_roleNames = {
    { Qt::DisplayRole, "display" },
    { FileNameRole, "filename" },
    { NameRole, "name" },
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
                                               QDir::Files | QDir::Readable, QDir::Name)) {
        if (showAll || isSupported(entry))
            m_games.append(entry);
    }

    m_lastPlayed = Patience::instance()->history().mid(0, ShownLastPlayedGames);

    auto favorites = m_favoriteConf.value().toString().split(';');
    favorites.removeAll(QString());
    m_favorites = favorites;

    connect(&m_favoriteConf, &MGConfItem::valueChanged, this, [&] {
        qCDebug(lcGameList) << "Saved favorites:" << m_favoriteConf.value().toString();
    });
}

int GameList::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_lastPlayed.count() + m_favorites.count() + m_games.count();
}

QVariant GameList::data(const QModelIndex &index, int role) const
{
    if (index.row() > rowCount())
        return QVariant();

    switch (role) {
    case DisplayRole:
        return displayable(getFileName(index.row()));
    case FileNameRole:
        return getFileName(index.row());
    case NameRole:
        return name(getFileName(index.row()));
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

QString GameList::displayable(const QString &fileName)
{
    QString displayName(name(fileName));
    bool startOfWord = true;
    for (int i = 0; i < displayName.length(); i++) {
        if (startOfWord) {
            displayName[i] = QChar(displayName[i]).toUpper();
            startOfWord = false;
        } else if (displayName[i] == QChar('-')) {
            startOfWord = true;
            displayName[i] = QChar(' ');
        }
    }
    return displayName;
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
    if (row < 0 || row > rowCount())
        return;

    QString fileName = getFileName(row);
    Section section = getSection(row);
    bool changed = false;

    auto it = std::lower_bound(m_favorites.begin(), m_favorites.end(), fileName);
    if (favorite) {
        if (it == m_favorites.end() || *it != fileName) {
            int pos = std::distance(m_favorites.begin(), it) + m_lastPlayed.count();
            beginInsertRows(QModelIndex(), pos, pos);
            m_favorites.insert(it, fileName);
            endInsertRows();
            changed = true;
            m_favoriteConf.set(m_favorites.join(';'));
            qCDebug(lcGameList) << "Inserted favorite to" << pos << "for row" << row
                                << "with value" << fileName;
        }
    } else {
        if (it != m_favorites.end() && *it == fileName) {
            int pos = std::distance(m_favorites.begin(), it) + m_lastPlayed.count();
            beginRemoveRows(QModelIndex(), pos, pos);
            m_favorites.erase(it);
            endRemoveRows();
            changed = true;
            m_favoriteConf.set(m_favorites.join(';'));
            qCDebug(lcGameList) << "Removed favorite from" << pos << "for row" << row
                                << "with value" << fileName;
        }
    }
    if (changed) {
        int lastPlayedRow = section == LastPlayed ? row : m_lastPlayed.indexOf(fileName);
        if (lastPlayedRow != -1)
            emitFavoriteChanged(lastPlayedRow);
        if (section == AllGames) {
            emitFavoriteChanged(row + (favorite ? 1 : -1));
        } else {
            int allGamesRow = std::distance(m_games.begin(),
                    std::lower_bound(m_games.begin(), m_games.end(), fileName));
            emitFavoriteChanged(allGamesRow + m_lastPlayed.count() + m_favorites.count());
        }
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
    if (row < m_lastPlayed.count())
        return m_lastPlayed[row];
    if (row < m_lastPlayed.count() + m_favorites.count())
        return m_favorites[row - m_lastPlayed.count()];
    return m_games[row - m_lastPlayed.count() - m_favorites.count()];
}

GameList::Section GameList::getSection(int row) const
{
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
