include(../config.pri)
#include(../vstsdk.pri)

QT += core gui widgets
win32 {
QMAKE_LFLAGS+="/DEF:$${_PRO_FILE_PWD_}/vstboard.def"
OTHER_FILES += vstboard.def
}
TARGET = "VstBoardPlugin"
TEMPLATE = lib

CONFIG += precompile_header
PRECOMPILED_HEADER = ../common/precomp.h

LIBS += -L$$VST3SDK_LIB -lbase
LIBS += -L$$VST3SDK_LIB -lpluginterfaces
LIBS += -L$$VST3SDK_LIB -lsdk

#CONFIG(debug, debug|release) {
#    LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/" -lvld
#    INCLUDEPATH += "C:/Program Files (x86)/Visual Leak Detector/include/"
#}

win32 {
#    CONFIG += qtwinmigrate-uselib
include($${_PRO_FILE_PWD_}/../../libs/qtwinmigrate/src/qtwinmigrate.pri)
}

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


LIBDEPS = common
for(a, LIBDEPS) {
    LIBS += -L$$DESTDIR -l$${a}
    PRE_TARGETDEPS += $$DESTDIR/$${LIBPREFIX}$${a}.$${LIBEXT}
    INCLUDEPATH += $$DESTDIR/$${a}
    DEPENDPATH += $$DESTDIR/$${a}
}


    SOURCES += $$VST3SDK_PATH/public.sdk/source/vst2.x/audioeffect.cpp
    SOURCES += $$VST3SDK_PATH/public.sdk/source/vst2.x/audioeffectx.cpp


SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/basewrapper/basewrapper.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/vst2wrapper/vst2wrapper.cpp

SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/eventlist.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/parameterchanges.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/processdata.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/module.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/stringconvert.cpp
win32 {
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/module_win32.cpp
}
linux-g++{
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/module_linux.cpp
}
macx{
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/module_mac.mm
}
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/plugprovider.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/common/memorystream.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/connectionproxy.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/hostclasses.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/pluginterfacesupport.cpp
