include(../common.pri)
TEMPLATE = app
TARGET = $$NAME
QT += svg
CONFIG += link_pkgconfig sailfishapp
PKGCONFIG += guile-2.2
DEFINES += DATADIR=/usr/share/$$TARGET
SOURCES += *.cpp
HEADERS += *.h
