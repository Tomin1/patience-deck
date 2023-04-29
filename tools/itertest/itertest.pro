TEMPLATE = app
TARGET = itertest
CONFIG -= qt

INCLUDEPATH += ../../src/common

SOURCES = \
    itertest.cpp \
    ../../src/common/itertools.cpp

HEADERS += \
    ../../src/common/itertools.h
