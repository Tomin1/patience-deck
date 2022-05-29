TEMPLATE = aux

CARD_JSON = anglo.json anglo-optimized.json anglo-simplified.json

card_style.output = ${QMAKE_FILE_BASE}.svg
card_style.commands = ../tools/card_style_converter.py ${QMAKE_FILE_NAME} \
        ../aisleriot/cards/anglo.svg ${QMAKE_FILE_OUT}
card_style.input = CARD_JSON
card_style.depends = ../aisleriot/cards/anglo.svg
card_style.variable_out = CARD_SVG
card_style.CONFIG = target_predeps
QMAKE_EXTRA_COMPILERS += card_style

CARD_SVGS = $$replace(CARD_JSON, json, svg)

card_files.depends = $${CARD_SVGS}
card_files.files = $${CARD_SVGS}
card_files.path = /usr/share/$$(NAME)/data
card_files.CONFIG = no_check_exist
INSTALLS += card_files

buttons.files = $$files(buttons/*.svg)
buttons.files -= buttons/icon-m-blank.svg
buttons.path = /usr/share/$$(NAME)/buttons/
INSTALLS += buttons

copying.files = COPYING.README
copying.path = /usr/share/$$(NAME)/
INSTALLS += copying

authors.commands = ../tools/generate_authors.py \
        --authors=../aisleriot/AUTHORS \
        --manual=../aisleriot/help/C \
        --games=../aisleriot/games \
        --append=\"Aike Reyer\" $(INSTALL_ROOT)/usr/share/$$(NAME)/data/AUTHORS
authors.CONFIG = no_check_exist
authors.path = /usr/share/$$(NAME)/data
INSTALLS += authors
