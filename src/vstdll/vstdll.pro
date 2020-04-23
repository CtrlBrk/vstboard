include(../config.pri)
include(../vstsdk.pri)

QT += core gui widgets

QMAKE_LFLAGS+="/DEF:$${_PRO_FILE_PWD_}/vstboard.def"

TARGET = "VstBoardPlugin"
TEMPLATE = lib

CONFIG += precompile_header
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
    LIBS += -lcomdlg32
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
    vst2shell.cpp \
    myvst2wrapper.cpp

HEADERS  += \
    gui.h \
    connectables/vstaudiodevicein.h \
    connectables/vstaudiodeviceout.h \
    loaderhelpers.h \
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

OTHER_FILES += vstboard.def


LIBDEPS = common #portaudio portmidi
for(a, LIBDEPS) {
    LIBS += -L$$DESTDIR -l$${LIBPREFIX}$${a}
    PRE_TARGETDEPS += $$DESTDIR/$${LIBPREFIX}$${a}.$${LIBEXT}
    INCLUDEPATH += $$DESTDIR/$${a}
    DEPENDPATH += $$DESTDIR/$${a}
}

SOURCES += $$VST3SDK_PATH/public.sdk/source/vst2.x/audioeffect.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst2.x/audioeffectx.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/basewrapper/basewrapper.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/vst2wrapper/vst2wrapper.cpp
