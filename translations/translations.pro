TEMPLATE = aux

TS_FILE = $$(NAME).ts
EE_QM = $$(NAME).qm

ts.commands += lupdate $$PWD/../qml -ts $$TS_FILE
ts.CONFIG += no_check_exist
ts.output = $$TS_FILE
ts.input = ..

engineering_english.commands += lrelease -idbased $$TS_FILE -qm $$EE_QM
engineering_english.CONFIG += no_check_exist
engineering_english.depends = ts
engineering_english.input = $$TS_FILE
engineering_english.output = $$EE_QM

engineering_english_install.path = /usr/share/$$(NAME)/translations
engineering_english_install.files = $$EE_QM
engineering_english_install.CONFIG += no_check_exist

QMAKE_EXTRA_TARGETS += ts engineering_english
PRE_TARGETDEPS += ts engineering_english
INSTALLS += engineering_english_install
