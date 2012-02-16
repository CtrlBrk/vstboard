#-------------------------------------------------
#
# Project created by QtCreator 2011-08-26T11:01:04
#
#-------------------------------------------------

QT       += testlib

#QT       -= gui

DEFINES += TESTING

TARGET = tst_testtest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH += $$PWD/../../src/common
DEPENDPATH += $$PWD/../../src/common

SOURCES += tst_testtest.cpp \
    audiobuffer.cpp \
    circularbuffer.cpp \
    renderer/optimizernode.cpp \
    renderer/optimizemap.cpp \
    renderer/node.cpp

DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    audiobuffer.h \
    circularbuffer.h \
    precomp.h

PRECOMPILED_HEADER = precomp.h

#to add symbols :
    QMAKE_CXXFLAGS_RELEASE +=  -Zi
    QMAKE_LFLAGS_RELEASE += /DEBUG
