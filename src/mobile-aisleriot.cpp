#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

#include <QGuiApplication>
#include <QQuickView>
#include <QScopedPointer>
#include <QtQml>
#include <sailfishapp.h>
#include "aisleriot.h"
#include "board.h"

int main(int argc, char *argv[])
{
    qputenv("GUILE_AUTO_COMPILE", "0");
    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));
    QScopedPointer<QQuickView> view(SailfishApp::createView());
    qmlRegisterSingletonType<Aisleriot>("Aisleriot", 1, 0, "Aisleriot", &Aisleriot::instance);
    qmlRegisterType<Board>("Aisleriot", 1, 0, "Board");
    view->setSource(SailfishApp::pathToMainQml());
    view->show();
    return app->exec();
}
