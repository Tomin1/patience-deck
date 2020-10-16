#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

#include <QGuiApplication>
#include <QQuickView>
#include <QScopedPointer>
#include <QtQml>
#include <sailfishapp.h>
#include "patience.h"
#include "table.h"
#include "gamelist.h"
#include "gameoptionmodel.h"

int main(int argc, char *argv[])
{
    qputenv("GUILE_AUTO_COMPILE", "0");
    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));
    QScopedPointer<QQuickView> view(SailfishApp::createView());
    qmlRegisterSingletonType<Patience>("Patience", 1, 0, "Patience", &Patience::instance);
    qmlRegisterType<Table>("Patience", 1, 0, "Table");
    qmlRegisterType<GameList>("Patience", 1, 0, "GameList");
    qmlRegisterType<GameOptionModel>("Patience", 1, 0, "GameOptions");
    view->setSource(SailfishApp::pathToMainQml());
    view->show();
    return app->exec();
}
