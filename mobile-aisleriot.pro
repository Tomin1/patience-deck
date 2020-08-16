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

CONFIG += link_pkgconfig
PKGCONFIG += guile-2.2

CONFIG += sailfishapp

SOURCES += src/mobile-aisleriot.cpp \
    src/aisleriot_scm.cpp \
    src/aisleriot.cpp \
    src/card.cpp \
    src/slot.cpp \
    src/logging.cpp

HEADERS += src/aisleriot.h \
    src/aisleriot_scm.h \
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

games.files = aisleriot/games/*.scm
games.path = /usr/share/$$TARGET/games/

INSTALLS += games

#CONFIG += sailfishapp_i18n
#TRANSLATIONS += translations/mobile-aisleriot-de.ts
