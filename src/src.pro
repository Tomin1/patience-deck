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
    engine/engine.cpp \
    engine/interface.cpp \
    common/logging.cpp \
    manager/manager.cpp \
    manager/queue.cpp \
    models/gamelist.cpp \
    models/gameoptionmodel.cpp \
    models/helpmodel.cpp \
    patience/timer.cpp \
    patience/patience.cpp \
    table/card.cpp \
    table/countableid.cpp \
    table/drag.cpp \
    table/feedbackevent.cpp \
    table/slot.cpp \
    table/table.cpp \
    table/texturerenderer.cpp \
    patience-deck.cpp

HEADERS += \
    common/constants.h \
    common/logging.h \
    engine/enginedata.h \
    engine/engine.h \
    engine/engine_p.h \
    engine/interface.h \
    manager/manager.h \
    manager/queue.h \
    models/gamelist.h \
    models/gameoptionmodel.h \
    models/helpmodel.h \
    patience/patience.h \
    patience/timer.h \
    table/card.h \
    table/countableid.h \
    table/drag.h \
    table/feedbackevent.h \
    table/slot.h \
    table/table.h \
    table/texturerenderer.h

INCLUDEPATH += \
    common \
    engine \
    manager \
    models \
    patience \
    table
