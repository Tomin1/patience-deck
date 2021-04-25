TEMPLATE = subdirs
SUBDIRS = src translations

CONFIG += sailfishapp

DISTFILES += \
    qml/*.qml \
    qml/cover/*.qml \
    qml/pages/*.qml \
    qml/images/*.svg \
    rpm/$$(NAME).spec \
    translations/*.ts \
    $$(NAME).desktop

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172

games.files = $$files(aisleriot/games/*.scm)
games.files -= aisleriot/games/api.scm
games.files -= aisleriot/games/card-monkey.scm
games.files -= aisleriot/games/template.scm
games.files -= aisleriot/games/test.scm
games.path = /usr/share/$$TARGET/games/

api.files = aisleriot/games/api.scm
api.path = /usr/share/$$TARGET/games/aisleriot/

manual.files = aisleriot/help/C/*.xml
manual.path = /usr/share/$$TARGET/help/

figures.files = aisleriot/help/C/figures/*.png
figures.path = /usr/share/$$TARGET/help/figures/

data.files = aisleriot/AUTHORS \
    aisleriot/cards/anglo.svg
data.path = /usr/share/$$TARGET/data/

INSTALLS += games api data manual figures
