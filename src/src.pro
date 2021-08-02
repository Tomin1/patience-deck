TEMPLATE = app
TARGET = $$(NAME)
QT += svg
CONFIG += link_pkgconfig sailfishapp
equals(TARGET, "harbour-patience-deck") {
    INCLUDEPATH += $(CACHE)/built/usr/share/harbour-patience-deck/include
    INCLUDEPATH += $(CACHE)/built/usr/share/harbour-patience-deck/include/guile/2.2
    LIBS += -L$(CACHE)/built/usr/share/harbour-patience-deck/lib -lguile-2.2 -lgc
} else {
    PKGCONFIG += guile-2.2
}
PKGCONFIG += mlite5
DEFINES += DATADIR=/usr/share/$$TARGET
DEFINES += VERSION=$(VERSION)
SOURCES += \
    card.cpp \
    drag.cpp \
    engine.cpp \
    gamelist.cpp \
    gameoptionmodel.cpp \
    interface.cpp \
    logging.cpp \
    manager.cpp \
    patience.cpp \
    patience-deck.cpp \
    queue.cpp \
    slot.cpp \
    table.cpp \
    texturerenderer.cpp \
    timer.cpp

HEADERS += \
    card.h \
    constants.h \
    drag.h \
    enginedata.h \
    engine.h \
    engine_p.h \
    gamelist.h \
    gameoptionmodel.h \
    interface.h \
    logging.h \
    manager.h \
    patience.h \
    queue.h \
    slot.h \
    table.h \
    texturerenderer.h \
    timer.h
