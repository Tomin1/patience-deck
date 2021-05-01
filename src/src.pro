TEMPLATE = app
TARGET = $$(NAME)
QT += svg
CONFIG += link_pkgconfig sailfishapp
PKGCONFIG += guile-2.2 mlite5
DEFINES += DATADIR=/usr/share/$$TARGET
DEFINES += VERSION=$(VERSION)
SOURCES += *.cpp
HEADERS += *.h
