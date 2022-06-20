TEMPLATE = app
TARGET = engine-exerciser
CONFIG += link_pkgconfig
PKGCONFIG += guile-2.2

QT += core qml

DEFINES += \
    DATADIR=games \
    ENGINE_EXERCISER=1

INCLUDEPATH += \
    ../../src/common \
    ../../src/engine \
    ../../src/manager \
    ../../src/models

DISTFILES += \
    qml/*.qml

SOURCES += \
    src/exerciser.cpp \
    src/checker.cpp \
    src/helper.cpp \
    ../../src/engine/engine.cpp \
    ../../src/engine/interface.cpp \
    ../../src/engine/recorder.cpp \
    ../../src/manager/queue.cpp \
    ../../src/common/logging.cpp

HEADERS += \
    src/checker.h \
    src/helper.h \
    ../../src/engine/engine.h \
    ../../src/engine/engineinternals.h \
    ../../src/engine/enginedata.h \
    ../../src/engine/interface.h \
    ../../src/engine/recorder.h \
    ../../src/manager/queue.h \
    ../../src/common/logging.h

games.files = $$files(../../aisleriot/games/*.scm)
games.files -= ../../aisleriot/games/api.scm
games.files -= ../../aisleriot/games/card-monkey.scm
games.files -= ../../aisleriot/games/template.scm
games.files -= ../../aisleriot/games/test.scm
games.path = games/

api.files = ../../aisleriot/games/api.scm
api.path = games/aisleriot/

INSTALLS += games api
