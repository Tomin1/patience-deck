/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2022 Tomi Leppänen
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

#ifndef PATIENCEDECK_H
#define PATIENCEDECK_H

#include <QObject>
#include <QString>

class QJSEngine;
class QQmlEngine;
class PatienceDeck : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString aisleriotAuthors READ aisleriotAuthors CONSTANT)
    Q_PROPERTY(QString aisleriotTranslatorInfo READ aisleriotTranslatorInfo CONSTANT)
    Q_PROPERTY(QString translators READ translators CONSTANT)
    Q_PROPERTY(bool showLibraryLicenses READ showLibraryLicenses CONSTANT)
    Q_PROPERTY(int gamesCount READ gamesCount CONSTANT)
    Q_PROPERTY(bool showAllGames READ showAllGames WRITE setShowAllGames NOTIFY showAllGamesChanged)

public:
    static QObject* instance(QQmlEngine *engine, QJSEngine *scriptEngine);
    ~PatienceDeck();

    // QML API
    Q_INVOKABLE QString getIconPath(int size) const;

    // Properties
    QString aisleriotAuthors() const;
    QString aisleriotTranslatorInfo() const;
    QString translators() const;
    bool showLibraryLicenses() const;
    int gamesCount() const;
    bool showAllGames() const;
    void setShowAllGames(bool show);

signals:
    void showAllGamesChanged();

private:
    static QString appName();

    explicit PatienceDeck(QObject *parent = nullptr);
    QString readFile(const QString &path) const;

    static PatienceDeck *s_app;
};

#endif // PATIENCEDECK_H
