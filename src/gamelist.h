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

#ifndef GAMELIST_H
#define GAMELIST_H

#include <QAbstractListModel>

class GameList : public QAbstractListModel
{
    Q_OBJECT

public:
    static QString displayable(const QString &fileName);
    static QString name(const QString &fileName);

    explicit GameList(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QHash<int, QByteArray> roleNames() const;

    enum Roles {
        DisplayRole = Qt::DisplayRole,
        FileNameRole = Qt::UserRole,
        NameRole = Qt::UserRole + 1,
    };

private:
    static QHash<int, QByteArray> s_roleNames;

    QStringList m_games;
};

#endif // GAMELIST_H
