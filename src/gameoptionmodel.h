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

#ifndef GAMEOPTIONLIST_H
#define GAMEOPTIONLIST_H

#include <QAbstractListModel>
#include "enginedata.h"

class GameOptionModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged);

public:
    explicit GameOptionModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QHash<int, QByteArray> roleNames() const;
    Q_INVOKABLE void select(int index);
    static bool loadOptions(const QString &gameFile, GameOptionList &options);
    static void saveOptions(const QString &gameFile, const GameOptionList &options);

    enum Roles {
        DisplayRole = Qt::DisplayRole,
        SetRole = Qt::UserRole,
        TypeRole = Qt::UserRole+1
    };

    enum Types {
        CheckType,
        RadioType
    };

signals:
    void doRequestGameOptions();
    void doSetGameOption(const GameOption &option);
    void doSetGameOptions(const GameOptionList &options);
    void countChanged();

private slots:
    void handleGameOptions(GameOptionList options);

private:
    static QHash<int, QByteArray> s_roleNames;

    GameOptionList m_options;
};

#endif // GAMEOPTIONLIST_H
