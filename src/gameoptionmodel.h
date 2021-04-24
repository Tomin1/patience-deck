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

#ifndef GAMEOPTIONLIST_H
#define GAMEOPTIONLIST_H

#include <QAbstractItemModel>
#include "enginedata.h"

class GameOptionModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged);

public:
    explicit GameOptionModel(QObject *parent = nullptr);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    QHash<int, QByteArray> roleNames() const;

    static bool loadOptions(const QString &gameFile, GameOptionList &options);
    static void saveOptions(const QString &gameFile, const GameOptionList &options);
    static void clearOptions(const QString &gameFile);

    enum Roles {
        DisplayRole = Qt::DisplayRole,
        SetRole = Qt::UserRole,
        CurrentRole,
        TypeRole
    };

    enum Type {
        CheckType,
        RadioType
    };
    Q_ENUM(Type)

signals:
    void doRequestGameOptions();
    void doSetGameOption(const GameOption &option);
    void doSetGameOptions(const GameOptionList &options);
    void countChanged();

private slots:
    void handleGameOptions(GameOptionList options);

private:
    static QHash<int, QByteArray> s_roleNames;

    int groupIndex(int optionsIndex) const;
    int getCurrent(int first) const;

    GameOptionList m_options;
    QVector<int> m_groups;
};

#endif // GAMEOPTIONLIST_H
