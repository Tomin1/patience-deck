# NOTICE:
#
# Application name defined in TARGET has a corresponding QML filename.
# If name defined in TARGET is changed, the following needs to be done
# to match new name:
#   - corresponding QML filename must be changed
#   - desktop icon filename must be changed
#   - desktop filename must be changed
#   - icon definition filename in desktop file must be changed
#   - translation filenames have to be changed

# The name of your application
TARGET = mobile-aisleriot

QT += svg
CONFIG += link_pkgconfig
PKGCONFIG += guile-2.2

CONFIG += sailfishapp

SOURCES += src/mobile-aisleriot.cpp \
    src/gamelist.cpp \
    src/engine.cpp \
    src/interface.cpp \
    src/aisleriot.cpp \
    src/board.cpp \
    src/drag.cpp \
    src/card.cpp \
    src/slot.cpp \
    src/logging.cpp

HEADERS += src/aisleriot.h \
    src/constants.h \
    src/gamelist.h \
    src/engine.h \
    src/engine_p.h \
    src/interface.h \
    src/board.h \
    src/drag.h \
    src/card.h \
    src/slot.h \
    src/logging.h

DISTFILES += qml/mobile-aisleriot.qml \
    qml/cover/CoverPage.qml \
    qml/pages/FirstPage.qml \
    qml/pages/SecondPage.qml \
    rpm/mobile-aisleriot.spec \
    translations/*.ts \
    mobile-aisleriot.desktop

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172

games.files = $$files(aisleriot/games/*.scm)
games.files -= aisleriot/games/api.scm
games.files -= aisleriot/games/card-monkey.scm
games.files -= aisleriot/games/template.scm
games.files -= aisleriot/games/test.scm
games.path = /usr/share/$$TARGET/games/

api.files = aisleriot/games/api.scm
api.path = /usr/share/$$TARGET/games/aisleriot/

data.files = aisleriot/cards/anglo.svg
data.path = /usr/share/$$TARGET/data/

INSTALLS += games api data

#CONFIG += sailfishapp_i18n
#TRANSLATIONS += translations/mobile-aisleriot-de.ts
