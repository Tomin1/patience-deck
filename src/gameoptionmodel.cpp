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

#include <MGConfItem>
#include <QSet>
#include "constants.h"
#include "gamelist.h"
#include "gameoptionmodel.h"
#include "engine.h"
#include "logging.h"
#include "patience.h"

namespace {

bool isGroupsIndex(quintptr ptr)
{
    return static_cast<int>(ptr) < 0;
}

int toGroupsIndex(quintptr ptr)
{
    int i = static_cast<int>(ptr);
    return i < 0 ? -(i+1) : -1;
}

int toOptionsIndex(quintptr ptr)
{
    int i = static_cast<int>(ptr);
    return i >= 0 ? i : -1;
}

quintptr fromGroupsIndex(int i)
{
    return static_cast<quintptr>(-i-1);
}

quintptr fromOptionsIndex(int i)
{
    return static_cast<quintptr>(i);
}

} // namespace

const QString OptionsConfTemplate = QStringLiteral("/options/%1");

QHash<int, QByteArray> GameOptionModel::s_roleNames = {
    { Qt::DisplayRole, "display" },
    { SetRole, "set" },
    { TypeRole, "type" },
    { CurrentRole, "current" }
};

GameOptionModel::GameOptionModel(QObject *parent)
    : QAbstractItemModel(parent)
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

int GameOptionModel::groupIndex(int optionsIndex) const
{
    auto it = std::lower_bound(m_groups.constBegin(), m_groups.constEnd(), optionsIndex);
    return it - m_groups.constBegin();
}

int GameOptionModel::getCurrent(int first) const
{
    for (int i = first; i < m_options.count() && m_options[i].type == RadioGameOption; i++) {
        if (m_options[i].set)
            return i - first;
    }
    return 0;
}

Qt::ItemFlags GameOptionModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

int GameOptionModel::rowCount(const QModelIndex &parent) const
{
    qCDebug(lcOptionList) << "row count for" << parent;

    if (parent.isValid()) {
        int i = fromGroupsIndex(parent.internalId());
        if (i < 0 || (i+1) >= m_groups.count())
            return 0;
        return m_groups[i+1] - m_groups[i];
    }

    return m_groups.count() - 1;
}

int GameOptionModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 0;
}

QModelIndex GameOptionModel::index(int row, int column, const QModelIndex &parent) const
{
    qCDebug(lcOptionList) << "index for" << row << "," << column << "with parent" << parent;

    if (row < 0 || column < 0)
        return QModelIndex();

    if (parent.isValid()) {
        int i = toGroupsIndex(parent.internalId());
        if ((i+1) < m_groups.count() && m_groups[i] + row < m_groups[i + 1])
            return createIndex(row, 0, fromOptionsIndex(m_groups[i] + row));
    } else if (row < m_groups.count()) {
        return createIndex(row, 0, m_options[m_groups[row]].type == CheckGameOption
                ? fromOptionsIndex(m_groups[row]) : fromGroupsIndex(row));
    }

    return QModelIndex();
}

QModelIndex GameOptionModel::parent(const QModelIndex &index) const
{
    qCDebug(lcOptionList) << "parent for" << index;

    if (!index.isValid())
        return QModelIndex();

    int i = toOptionsIndex(index.internalId());
    if (i >= 0 && i < m_options.count() && m_options[i].type == RadioGameOption) {
        int group = groupIndex(i);
        return createIndex(group, 0, fromGroupsIndex(group));
    }

    return QModelIndex();
}

QVariant GameOptionModel::data(const QModelIndex &index, int role) const
{
    qCDebug(lcOptionList) << "get" << index << "with role" << role;

    if (!index.isValid())
        return QVariant();

    int i;
    if (isGroupsIndex(index.internalId())) {
        i = toGroupsIndex(index.internalId());
        if (i >= m_groups.count())
            return QVariant();
        i = m_groups[i];
    } else {
        i = toOptionsIndex(index.internalId());
        if (i >= m_options.count())
            return QVariant();
    }

    switch (role) {
    case DisplayRole:
        return m_options[i].displayName;
    case TypeRole:
        return m_options[i].type == CheckGameOption ? CheckType : RadioType;
    case SetRole:
        return m_options[i].set;
    case CurrentRole:
        return getCurrent(i);
    default:
        return QVariant();
    }
}

bool GameOptionModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    qCDebug(lcOptionList) << "set" << index << "with role" << role << "to" << value;

    if (!index.isValid())
        return false;

    if (role == SetRole) {
        int i = toOptionsIndex(index.internalId());
        if (i < 0 || i >= m_options.count() || m_options[i].type != CheckGameOption)
            return false;
        bool set = value.toBool();
        if (m_options[i].set != set) {
            m_options[i].set = set;
            emit doSetGameOption(m_options[i]);
            emit dataChanged(index, index, QVector<int>() << SetRole);
        }
    } else if (role == CurrentRole) {
        int group = toGroupsIndex(index.internalId());
        if (group < 0 || group >= m_groups.count())
            return false;
        int first = m_groups[group];
        if (m_options[first].type != RadioGameOption)
            return false;
        int end = m_groups[group + 1];
        int i = first + value.toInt();
        for (int j = first; j < end; j++)
            m_options[j].set = (i == j);
        emit doSetGameOptions(m_options.mid(first, end - first));
        emit dataChanged(index, index, QVector<int>() << CurrentRole);
    } else {
        return false;
    }
    GameOptionModel::saveOptions(Patience::instance()->gameFile(), m_options);
    return true;
}

QHash<int, QByteArray> GameOptionModel::roleNames() const
{
    return s_roleNames;
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
    optionsConf.sync();
}

void GameOptionModel::clearOptions(const QString &gameFile)
{
    MGConfItem optionsConf(Constants::ConfPath + OptionsConfTemplate.arg(GameList::name(gameFile)));
    optionsConf.set(QVariant());
    optionsConf.sync();
}

void GameOptionModel::handleGameOptions(GameOptionList options)
{
    if (!m_options.isEmpty()) {
        // In practice this should not be needed
        qCWarning(lcOptionList) << "Clearing game options";
        beginRemoveRows(QModelIndex(), 0, m_groups.count()-1);
        m_options.clear();
        m_groups.clear();
        endRemoveRows();
    }
    m_groups.append(0);
    if (!options.isEmpty()) {
        m_options.swap(options);
        GameOptionType lastType = m_options[0].type;
        for (auto it = ++m_options.constBegin(); it != m_options.constEnd(); it++) {
            if (it->type == CheckGameOption || it->type != lastType) {
                lastType = it->type;
                m_groups.append(it - m_options.constBegin());
            }
        }
        qCDebug(lcOptionList) << "Set up" << m_options.count() << "game options"
                              << "in" << m_groups.count() << "groups";
        beginInsertRows(QModelIndex(), 0, m_groups.count()-1);
        m_groups.append(m_options.count());
        endInsertRows();
    }
}
