TEMPLATE = subdirs
SUBDIRS = src translations
TARGET=$$(NAME)

CONFIG += sailfishapp

DISTFILES += \
    qml/*.qml \
    qml/*/*.qml \
    qml/*/*/*.qml \
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

manual.files = aisleriot/help/C/*.xml
manual.path = /usr/share/$$(NAME)/help/

figures.files = aisleriot/help/C/figures/*.png
figures.path = /usr/share/$$(NAME)/help/figures/

data.files = aisleriot/cards/anglo.svg
data.path = /usr/share/$$(NAME)/data/
data.path = /usr/share/$$(NAME)/data/

copying.files = COPYING.GPL3 \
    aisleriot/COPYING.GFDL \
    aisleriot/COPYING.GFDL1.3 \
    data/COPYING.README
copying.path = /usr/share/$$(NAME)/

INSTALLS += games api data manual figures copying
