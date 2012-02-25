#-------------------------------------------------
#
# Project created by QtCreator 2011-08-26T11:01:04
#
#-------------------------------------------------

QT       += testlib

#QT       -= gui

DEFINES += TESTING

CONFIG(debug, debug|release) {
    POST =
    build_postfix=debug
    DEFINES += DEBUG
} else {
    POST =
    build_postfix=release
    DEFINES += RELEASE
}

TARGET = tst_testtest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH += $$PWD/../../src/common
DEPENDPATH += $$PWD/../../src/common

SOURCES += tst_testtest.cpp

DEFINES += SRCDIR=\\\"$$PWD/\\\"

PRECOMPILED_HEADER = precomp.h

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../common/release/ -lcommon
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../common/debug/ -lcommon
else:unix:!symbian: LIBS += -L$$OUT_PWD/../common/ -lcommon

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../common/release/libcommon.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../common/debug/libcommon.a
else:win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../common/release/common.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../common/debug/common.lib
else:unix:!symbian: PRE_TARGETDEPS += $$OUT_PWD/../common/libcommon.a

#to add symbols :
    QMAKE_CXXFLAGS_RELEASE +=  -Zi
    QMAKE_LFLAGS_RELEASE += /DEBUG
