include(../config.pri)

QT += core gui widgets

QMAKE_LFLAGS+="/DEF:$${_PRO_FILE_PWD_}/vstboard.def"

TARGET = "VstBoardPlugin"
TEMPLATE = lib

PRECOMPILED_HEADER = ../common/precomp.h

#CONFIG(debug, debug|release) {
#    LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/" -lvld
#    INCLUDEPATH += "C:/Program Files (x86)/Visual Leak Detector/include/"
#}

win32 {
    LIBS += -lwinmm
    LIBS += -ladvapi32
    LIBS += -lws2_32
    LIBS += -lole32
}

win32-g++ {
    QMAKE_LFLAGS += -static-libgcc
}

win32-msvc* {
    DEFINES += _CRT_SECURE_NO_WARNINGS
    RC_FILE = vstdll.rc
}

INCLUDEPATH += ../common

#CONFIG += qtwinmigrate-uselib
include($${_PRO_FILE_PWD_}/../../libs/qtwinmigrate/src/qtwinmigrate.pri)

SOURCES += \
    main.cpp \
    gui.cpp \
    connectables/vstaudiodevicein.cpp \
    connectables/vstaudiodeviceout.cpp \
    vst.cpp \
    connectables/vstautomation.cpp \
    connectables/vstmididevice.cpp \
    mainwindowvst.cpp \
    connectables/objectfactoryvst.cpp \
    views/configdialogvst.cpp \
    resizehandle.cpp \
    factory.cpp \
    vstboardcontroller.cpp \
    vstboardprocessor.cpp \
    vsttest.cpp \
    $$VSTSDK_PATH/base/source/timer.cpp \
    $$VSTSDK_PATH/base/source/fstreamer.cpp \
    $$VSTSDK_PATH/base/source/fbuffer.cpp \
    $$VSTSDK_PATH/public.sdk/source/vst/hosting/eventlist.cpp \
    $$VSTSDK_PATH/public.sdk/source/vst/vst2wrapper/vst2wrapper.cpp \
    vst2shell.cpp \
    myvst2wrapper.cpp

HEADERS  += \
    gui.h \
    connectables/vstaudiodevicein.h \
    connectables/vstaudiodeviceout.h \
    vst.h \
    connectables/vstautomation.h \
    connectables/vstmididevice.h \
    mainwindowvst.h \
    connectables/objectfactoryvst.h \
    views/configdialogvst.h \
    resizehandle.h \
    ids.h \
    vstboardcontroller.h \
    vstboardprocessor.h \
    vsttest.h \
    vst2shell.h \
    myvst2wrapper.h

RESOURCES += ../resources/resources.qrc

#TRANSLATIONS = ../resources/translations/vstdll_fr.ts

win32-g++:LIBS += -L$$DESTDIR -llibcommon
else:win32:LIBS += -L$$DESTDIR -lcommon
else:unix:LIBS += -L$$DESTDIR -lcommon

INCLUDEPATH += $$DESTDIR/common
DEPENDPATH += $$DESTDIR/common

win32-g++:PRE_TARGETDEPS += $$DESTDIR/libcommon.a
else:win32:PRE_TARGETDEPS += $$DESTDIR/common.lib
else:unix:!symbian: PRE_TARGETDEPS += $$DESTDIR/libcommon.a

OTHER_FILES += \
    vstboard.def
































