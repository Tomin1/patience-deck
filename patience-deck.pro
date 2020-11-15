TEMPLATE = subdirs
SUBDIRS = src translations
TARGET=$$(NAME)

CONFIG += sailfishapp

DISTFILES += \
    qml/*.qml \
    qml/cover/*.qml \
    qml/pages/*.qml \
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
games.path = /usr/share/$$(NAME)/games/

api.files = aisleriot/games/api.scm
api.path = /usr/share/$$(NAME)/games/aisleriot/

data.files = aisleriot/AUTHORS \
    aisleriot/cards/anglo.svg
data.path = /usr/share/$$(NAME)/data/

INSTALLS += games api data
