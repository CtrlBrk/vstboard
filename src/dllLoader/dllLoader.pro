#-------------------------------------------------
#
# Project created by QtCreator 2011-02-03T20:52:43
#
#-------------------------------------------------

include(../config.pri)

QMAKE_LFLAGS+="/DEF:$${_PRO_FILE_PWD_}/vstboard.def"

DEFINES -= AS_INSTRUMENT
TARGET = VstBoardLoader
TEMPLATE = lib

QT       -= core gui
LIBS += -ladvapi32
LIBS += -luser32

SOURCES += \
    main.cpp \
    ../../libs/VST3SDK/pluginterfaces/base/conststringtable.cpp

HEADERS +=

win32-msvc* {
    RC_FILE = dllLoader.rc
}
#message($$TARGET)
#QMAKE_POST_LINK = copy $$TARGET.dll $$TARGET.vst3

