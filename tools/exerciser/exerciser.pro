TEMPLATE = app
TARGET = engine-exerciser
CONFIG += link_pkgconfig
PKGCONFIG += guile-2.2

QT += core qml

DEFINES += \
    DATADIR=games \
    ENGINE_EXERCISER=1

INCLUDEPATH += ../../src/

DISTFILES += \
    qml/*.qml

SOURCES += \
    src/exerciser.cpp \
    src/checker.cpp \
    src/helper.cpp \
    ../../src/engine.cpp \
    ../../src/interface.cpp \
    ../../src/queue.cpp \
    ../../src/logging.cpp

HEADERS += \
    src/checker.h \
    src/helper.h \
    ../../src/engine.h \
    ../../src/engine_p.h \
    ../../src/enginedata.h \
    ../../src/interface.h \
    ../../src/queue.h \
    ../../src/logging.h

games.files = $$files(../../aisleriot/games/*.scm)
games.files -= ../../aisleriot/games/api.scm
games.files -= ../../aisleriot/games/card-monkey.scm
games.files -= ../../aisleriot/games/template.scm
games.files -= ../../aisleriot/games/test.scm
games.path = games/

api.files = ../../aisleriot/games/api.scm
api.path = games/aisleriot/

INSTALLS += games api
