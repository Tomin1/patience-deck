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

#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

#include <QCommandLineParser>
#include <QGuiApplication>
#include <QQuickView>
#include <QScopedPointer>
#include <QtQml>
#include <sailfishapp.h>
#include "constants.h"
#include "engine.h"
#include "patience.h"
#include "table.h"
#include "gamelist.h"
#include "gameoptionmodel.h"

int main(int argc, char *argv[])
{
    qputenv("GUILE_AUTO_COMPILE", "0");
    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));
    QScopedPointer<QQuickView> view(SailfishApp::createView());

    QCommandLineParser parser;
    parser.setApplicationDescription("A collection of patience games.");
    auto helpOption = parser.addHelpOption();
    Engine::addArguments(&parser);

    parser.process(*app);
    if (parser.isSet(helpOption))
        parser.showHelp();
    Engine::setArguments(&parser);

    qmlRegisterSingletonType<Patience>("Patience", 1, 0, "Patience", &Patience::instance);
    qmlRegisterType<Table>("Patience", 1, 0, "Table");
    qmlRegisterType<GameList>("Patience", 1, 0, "GameList");
    qmlRegisterType<GameOptionModel>("Patience", 1, 0, "GameOptions");
    app->setApplicationVersion(QUOTE(VERSION));
    view->setSource(SailfishApp::pathToMainQml());
    view->show();
    return app->exec();
}
