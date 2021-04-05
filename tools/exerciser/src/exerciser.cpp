/*
 * Exerciser for Patience Deck engine class.
 * Copyright (C) 2021  Tomi Leppänen
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

#include <QCoreApplication>
#include <QQmlApplicationEngine>
#include "engine.h"
#include "helper.h"

int main(int argc, char *argv[])
{
    qputenv("GUILE_AUTO_COMPILE", "0");
    qputenv("LC_ALL", "C");
    QCoreApplication app(argc, argv);
    qmlRegisterUncreatableType<Engine>("Patience", 1, 0, "Engine", QStringLiteral("Use EngineHelper.engine"));
    qmlRegisterType<EngineHelper>("Patience", 1, 0, "EngineHelper");
    QQmlApplicationEngine qmlEngine("exerciser.qml");
    QObject::connect(&qmlEngine, &QQmlApplicationEngine::quit, &app, &QCoreApplication::quit);
    return app.exec();
}
