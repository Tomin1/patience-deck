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
TARGET = patience-deck

QT += svg
CONFIG += link_pkgconfig
PKGCONFIG += guile-2.2

CONFIG += sailfishapp

DEFINES += DATADIR=/usr/share/$$TARGET

SOURCES += src/patience-deck.cpp \
    src/gamelist.cpp \
    src/gameoptionmodel.cpp \
    src/engine.cpp \
    src/interface.cpp \
    src/patience.cpp \
    src/table.cpp \
    src/drag.cpp \
    src/card.cpp \
    src/slot.cpp \
    src/logging.cpp

HEADERS += src/patience.h \
    src/constants.h \
    src/gamelist.h \
    src/gameoptionmodel.h \
    src/engine.h \
    src/engine_p.h \
    src/interface.h \
    src/table.h \
    src/drag.h \
    src/card.h \
    src/slot.h \
    src/logging.h

DISTFILES += qml/patience-deck.qml \
    qml/cover/CoverPage.qml \
    qml/pages/Game.qml \
    qml/pages/SelectGame.qml \
    qml/pages/GameOptions.qml \
    qml/pages/AboutPage.qml \
    qml/images/*.svg \
    rpm/patience-deck.spec \
    translations/*.ts \
    patience-deck.desktop

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172

games.files = $$files(aisleriot/games/*.scm)
games.files -= aisleriot/games/api.scm
games.files -= aisleriot/games/card-monkey.scm
games.files -= aisleriot/games/template.scm
games.files -= aisleriot/games/test.scm
games.path = /usr/share/$$TARGET/games/

api.files = aisleriot/games/api.scm
api.path = /usr/share/$$TARGET/games/aisleriot/

data.files = aisleriot/AUTHORS \
    aisleriot/cards/anglo.svg
data.path = /usr/share/$$TARGET/data/

INSTALLS += games api data

#CONFIG += sailfishapp_i18n
#TRANSLATIONS += translations/patience-deck-de.ts
