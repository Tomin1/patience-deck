TEMPLATE = subdirs
SUBDIRS = src translations data
TARGET=$$(NAME)

CONFIG += sailfishapp

DISTFILES += \
    qml/*.qml \
    qml/*/*.qml \
    qml/*/*/*.qml \
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
chmod_games.depends = install_games
chmod_games.path = $$games.path
chmod_games.extra = chmod -x $(INSTALL_ROOT)/usr/share/$(NAME)/games/*.scm

api.files = aisleriot/games/api.scm
api.path = /usr/share/$$(NAME)/games/aisleriot/

manual.files = aisleriot/help/C/*.xml
manual.path = /usr/share/$$(NAME)/help/
manual_mv.depends = install_manual
manual_mv.path = $$manual.path
manual_mv.extra += mv $(INSTALL_ROOT)/usr/share/$(NAME)/help/eagle-wing.xml \
                      $(INSTALL_ROOT)/usr/share/$(NAME)/help/eagle_wing.xml

figures.files = aisleriot/help/C/figures/*.png
figures.path = /usr/share/$$(NAME)/help/figures/

copying.files = COPYING.GPL3 \
    aisleriot/COPYING.GFDL \
    aisleriot/COPYING.GFDL1.3
copying.path = /usr/share/$$(NAME)/

INSTALLS += games chmod_games api manual manual_mv figures copying

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

# Aisleriot translations
include(translations/translations.pri)
for (lang, LANGUAGES) {
    po_file = $$PWD/aisleriot/po/$${lang}.po
    mo_dir = $$shadowed(translations/mo/$${lang}/)
    mo_file = $${mo_dir}/aisleriot.mo

    aisleriot_mo_$${lang}.commands = mkdir -p $$mo_dir $$escape_expand(\n\t)
    aisleriot_mo_$${lang}.commands +=msggrep --msgid --regexp="translator-credits" --location="games/*.scm" --location="src/game-names.h" $$po_file | msgfmt -o $$mo_file /dev/stdin
    aisleriot_mo_$${lang}.depends = $$po_file
    aisleriot_mo_$${lang}.output = FORCE

    aisleriot_mo_$${lang}_install.CONFIG = no_check_exist
    aisleriot_mo_$${lang}_install.depends = aisleriot_mo_$${lang}
    aisleriot_mo_$${lang}_install.files = $$mo_file
    aisleriot_mo_$${lang}_install.path = /usr/share/$$(NAME)/translations/$${lang}/LC_MESSAGES

    QMAKE_EXTRA_TARGETS += aisleriot_mo_$${lang}
    PRE_TARGETDEPS += aisleriot_mo_$$lang
    INSTALLS += aisleriot_mo_$${lang}_install
}

translators_file.files = translations/TRANSLATORS
translators_file.path = /usr/share/$$(NAME)/data/
INSTALLS += translators_file
