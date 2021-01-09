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

#include <MGConfItem>
#include <QSet>
#include "constants.h"
#include "gamelist.h"
#include "gameoptionmodel.h"
#include "engine.h"
#include "patience.h"

const QString OptionsConfTemplate = QStringLiteral("/options/%1");

QHash<int, QByteArray> GameOptionModel::s_roleNames = {
    { Qt::DisplayRole, "display" },
    { SetRole, "set" },
    { TypeRole, "type" }
};

GameOptionModel::GameOptionModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(this, &GameOptionModel::rowsInserted, this, &GameOptionModel::countChanged);
    connect(this, &GameOptionModel::rowsRemoved, this, &GameOptionModel::countChanged);
    auto engine = Engine::instance();
    connect(this, &GameOptionModel::doRequestGameOptions, engine, &Engine::requestGameOptions);
    connect(engine, &Engine::gameOptions, this, &GameOptionModel::handleGameOptions);
    connect(this, &GameOptionModel::doSetGameOption, engine, &Engine::setGameOption);
    connect(this, &GameOptionModel::doSetGameOptions, engine, &Engine::setGameOptions);
    emit doRequestGameOptions();
}

int GameOptionModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_options.count();
}

QVariant GameOptionModel::data(const QModelIndex &index, int role) const
{
    if (index.row() > m_options.count())
        return QVariant();

    switch (role) {
    case DisplayRole:
        return m_options[index.row()].displayName;
    case SetRole:
        return m_options[index.row()].set;
    case TypeRole:
        return m_options[index.row()].type == CheckGameOption ? CheckType : RadioType;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> GameOptionModel::roleNames() const
{
    return s_roleNames;
}

void GameOptionModel::select(int row)
{
    if (row >= 0 && row < m_options.count()) {
        if (m_options[row].type == CheckGameOption) {
            m_options[row].set = !m_options[row].set;
            auto idx = index(row, 0);
            emit doSetGameOption(m_options[row]);
            emit dataChanged(idx, idx, QVector<int>() << SetRole);
        } else if (!m_options[row].set) { // && m_options[row].type == RadioGameOption
            int firstIndex = row-1;
            while (firstIndex >= 0 && m_options[firstIndex].type != CheckGameOption)
                firstIndex--;
            firstIndex++;
            int lastIndex = row+1;
            while (lastIndex < m_options.count() && m_options[lastIndex].type != CheckGameOption)
                lastIndex++;
            lastIndex--;
            for (int i = firstIndex; i <= lastIndex; i++)
                m_options[i].set = (i == row);
            emit doSetGameOptions(m_options.mid(firstIndex, lastIndex - firstIndex + 1));
            emit dataChanged(index(firstIndex, 0), index(lastIndex, 0), QVector<int>() << SetRole);
        }
        GameOptionModel::saveOptions(Patience::instance()->gameFile(), m_options);
    }
}

bool GameOptionModel::loadOptions(const QString &gameFile, GameOptionList &options)
{
    MGConfItem optionsConf(Constants::ConfPath + OptionsConfTemplate.arg(GameList::name(gameFile)));
    auto stored = optionsConf.value();
    if (stored.isValid()) {
        auto values = stored.toString().split(';').toSet();
        for (int i = 0; i < options.length(); i++) {
            options[i].set = values.contains(QString::number(options.at(i).index));
        }
        return true;
    }
    return false;
}

void GameOptionModel::saveOptions(const QString &gameFile, const GameOptionList &options)
{
    MGConfItem optionsConf(Constants::ConfPath + OptionsConfTemplate.arg(GameList::name(gameFile)));
    QStringList values;
    for (const GameOption &option : options) {
        if (option.set)
            values.append(QString::number(option.index));
    }
    optionsConf.set(values.join(';'));
}

void GameOptionModel::handleGameOptions(GameOptionList options)
{
    if (m_options.isEmpty() && !options.isEmpty()) {
        beginInsertRows(QModelIndex(), 0, options.count()-1);
        m_options.swap(options);
        endInsertRows();
    } else { // basically dead, untested code
        if (options.count() < m_options.count()) {
            beginInsertRows(QModelIndex(), options.count()-1, m_options.count()-1);
            m_options.swap(options);
            endInsertRows();
        } else if (options.count() > m_options.count()) {
            beginRemoveRows(QModelIndex(), m_options.count()-1, options.count()-1);
            m_options.swap(options);
            endRemoveRows();
        } else {
            m_options.swap(options);
        }
        if (!m_options.isEmpty())
            emit dataChanged(index(0, 0), index(std::min(options.count(), m_options.count())-1, 0));
    }
}
