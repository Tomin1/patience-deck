/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2020  Tomi Leppänen
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
#include "gamelist.h"
#include "constants.h"

QHash<int, QByteArray> GameList::s_roleNames = {
    { Qt::DisplayRole, "display" },
    { FileNameRole, "filename" },
    { NameRole, "name" }
};

GameList::GameList(QObject *parent)
    : QAbstractListModel(parent)
{
    QDir gameDirectory(Constants::GameDirectory);
    m_games = gameDirectory.entryList(QStringList() << QStringLiteral("*.scm"),
                                      QDir::Files | QDir::Readable, QDir::Name);
}

int GameList::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_games.count();
}

QVariant GameList::data(const QModelIndex &index, int role) const
{
    if (index.row() > m_games.count())
        return QVariant();

    switch (role) {
    case DisplayRole:
        return displayable(m_games[index.row()]);
    case FileNameRole:
        return m_games[index.row()];
    case NameRole:
        return name(m_games[index.row()]);
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
