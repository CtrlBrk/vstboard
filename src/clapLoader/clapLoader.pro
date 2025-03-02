#-------------------------------------------------
#
# Project created by QtCreator 2011-02-03T20:52:43
#
#-------------------------------------------------

include(../config.pri)

CONFIG += dll

DEFINES -= AS_INSTRUMENT
TARGET = VstBoardClap
TEMPLATE = lib
QMAKE_EXTENSION_SHLIB = clap

INCLUDEPATH += ../../libs/clap/include
INCLUDEPATH += ../../libs/clap-helpers/include

QT       -= core gui
LIBS += -ladvapi32
LIBS += -luser32
LIBS += -lcomdlg32

SOURCES += \
    main.cpp \
