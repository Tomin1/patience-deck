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

buttons.files = $$files(data/buttons/*.svg)
buttons.files -= data/buttons/icon-m-blank.svg
buttons.path = /usr/share/$$(NAME)/buttons/

copying.files = COPYING.GPL3 \
    aisleriot/COPYING.GFDL \
    aisleriot/COPYING.GFDL1.3 \
    data/COPYING.README
copying.path = /usr/share/$$(NAME)/

INSTALLS += games api data buttons manual figures copying

# Translations
TS_FILE = $$PWD/translations/patience-deck.ts
EE_QM = $$shadowed($$PWD/translations/patience-deck.qm)

qtPrepareTool(LUPDATE, lupdate)
ts_first.commands = $$LUPDATE $$PWD/qml -ts $$TS_FILE
ts_first.input = $$DISTFILES
ts_first.output = FORCE
ts_first.target = ts-make_first
PRE_TARGETDEPS += ts ts_first ts-install_subtargets
QMAKE_EXTRA_TARGETS += ts ts_first ts-install_subtargets
translations.depends = ts

qtPrepareTool(LRELEASE, lrelease)
engineering_english.commands = $$LRELEASE -idbased $$TS_FILE -qm $$EE_QM
engineering_english.depends = ts-make_first
engineering_english.input = $$TS_FILE

engineering_english_install.CONFIG = no_check_exist
engineering_english_install.depends = engineering_english
engineering_english_install.files = $$EE_QM
engineering_english_install.path = /usr/share/$$(NAME)/translations

QMAKE_EXTRA_TARGETS += engineering_english
PRE_TARGETDEPS += engineering_english
INSTALLS += engineering_english_install
