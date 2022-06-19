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

#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

#include <libintl.h>
#include <locale.h>
#include <QCommandLineParser>
#include <QGuiApplication>
#include <QQuickView>
#include <QScopedPointer>
#include <QtQml>
#include <sailfishapp.h>
#include "constants.h"
#include "engine.h"
#include "feedbackevent.h"
#include "gamelist.h"
#include "gameoptionmodel.h"
#include "helpmodel.h"
#include "patience.h"
#include "table.h"
#include "texturerenderer.h"

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
    qputenv("GUILE_AUTO_COMPILE", "0");
    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));
    QScopedPointer<QQuickView> view(SailfishApp::createView());

    // Add Qt translations
    QTranslator *eeTranslator = new QTranslator();
    eeTranslator->load("patience-deck", SailfishApp::pathTo("translations").toLocalFile(), ".qm");
    app->installTranslator(eeTranslator);
    QTranslator *translator = new QTranslator();
    translator->load(QLocale::system(), "patience-deck", "-", SailfishApp::pathTo("translations").toLocalFile(), ".qm");
    app->installTranslator(translator);

    // Add Aisleriot translations
    bindtextdomain("aisleriot", SailfishApp::pathTo("translations").toLocalFile().toLocal8Bit().constData());
    bind_textdomain_codeset("aisleriot", "UTF-8");
    textdomain("aisleriot");

    QCommandLineParser parser;
    parser.setApplicationDescription("A collection of patience games.");
    auto helpOption = parser.addHelpOption();
    Engine::addArguments(&parser);
    Table::addArguments(&parser);
    TextureRenderer::addArguments(&parser);
    Patience::addArguments(&parser);

    parser.process(*app);
    if (parser.isSet(helpOption))
        parser.showHelp();
    Engine::setArguments(&parser);
    Table::setArguments(&parser);
    TextureRenderer::setArguments(&parser);
    Patience::setArguments(&parser);

    qmlRegisterSingletonType<Patience>("Patience", 1, 0, "Patience", &Patience::instance);
    qmlRegisterType<Table>("Patience", 1, 0, "Table");
    qmlRegisterUncreatableType<FeedbackEvent>("Patience", 1, 0, "FeedbackEvent", "This is an attached property to Table");
    qmlRegisterType<GameList>("Patience", 1, 0, "GameList");
    qmlRegisterType<GameOptionModel>("Patience", 1, 0, "GameOptions");
    qmlRegisterType<HelpModel>("Patience", 1, 0, "HelpModel");
    app->setApplicationVersion(QUOTE(VERSION));
    view->setSource(SailfishApp::pathToMainQml());
    view->show();
    return app->exec();
}
