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

#ifndef GAMELIST_H
#define GAMELIST_H

#include <functional>
#include <MGConfItem>
#include <QAbstractListModel>
#include <QQmlParserStatus>

class GameList : public QAbstractListModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QString searchedText READ searchedText WRITE setSearchedText NOTIFY searchedTextChanged)

public:
    static QString capitalized(const QString &fileName);
    static QString translated(const QString &fileName);
    static QString name(const QString &fileName);
    static QStringList gamesList();
    static int count();

    Q_INVOKABLE void setFavorite(int row, bool favorite);
    QString searchedText() const;
    void setSearchedText(const QString &text);

    explicit GameList(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QHash<int, QByteArray> roleNames() const;

    void classBegin() override;
    void componentComplete() override;

    enum Roles {
        DisplayRole = Qt::DisplayRole,
        FileNameRole = Qt::UserRole,
        NameRole,
        TranslatedRole,
        CapitalizedRole,
        SectionRole,
        FavoriteRole,
        MatchedByRole,
    };

    enum Section {
        AllGames,
        LastPlayed,
        Favorites,
        SearchResults,
    };
    Q_ENUM(Section)

    enum MatchedBy {
        None,
        TranslatedName,
        CapitalizedName,
    };
    Q_ENUM(MatchedBy)

signals:
    void searchedTextChanged();

private:
    static MGConfItem *showAllConf();
    static bool lessThan(const QString &a, const QString &b);

    static QHash<int, QByteArray> s_roleNames;

    int findIndex(const QString &fileName) const;
    int getIndex(int row) const;
    QString getFileName(int row) const;
    Section getSection(int row) const;
    void emitFavoriteChanged(int row);

    bool searching() const;
    std::function<bool(int, int)> searchCompareFunction() const;
    void filterSearch();
    void clearSearch();
    void resetSearch();
    void emitChangedEntries(int oldCount, int newCount);
    MatchedBy matchedBy(int row) const;

    QVector<QString> m_games;
    QVector<int> m_lastPlayed;
    QStringList m_favorites;
    QString m_searchedText;
    QVector<int> m_results;

    MGConfItem m_favoriteConf;
};

#endif // GAMELIST_H
