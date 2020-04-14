include(../config.pri)

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
#    $$VST3SDK_PATH/base/source/timer.cpp \
#    $$VST3SDK_PATH/base/source/fstreamer.cpp \
#    $$VST3SDK_PATH/base/source/fbuffer.cpp \
#    $$VST3SDK_PATH/public.sdk/source/vst/hosting/eventlist.cpp \
#    $$VST3SDK_PATH/public.sdk/source/vst/vst2wrapper/vst2wrapper.cpp \

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

OTHER_FILES += vstboard.def


LIBDEPS = common #portaudio portmidi
for(a, LIBDEPS) {
    LIBS += -L$$DESTDIR -l$${LIBPREFIX}$${a}
    PRE_TARGETDEPS += $$DESTDIR/$${LIBPREFIX}$${a}.$${LIBEXT}
    INCLUDEPATH += $$DESTDIR/$${a}
    DEPENDPATH += $$DESTDIR/$${a}
}


SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/hostclasses.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/pluginterfacesupport.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/common/memorystream.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/processdata.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/parameterchanges.cpp


LIBDEPS = sdk base pluginterfaces
for(a, LIBDEPS) {
    LIBS += -L$$VST3SDK_PATH/$${VSTLIB}/lib/$${MSBUILDDIR} -l$${LIBPREFIX}$${a}
    PRE_TARGETDEPS += $$VST3SDK_PATH/$${VSTLIB}/lib/$${MSBUILDDIR}/$${LIBPREFIX}$${a}.$${LIBEXT}
    DEPENDPATH += $$VST3SDK_PATH/$${VSTLIB}/lib/$${MSBUILDDIR}/$${a}
}


SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/hostclasses.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/pluginterfacesupport.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/common/memorystream.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/processdata.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/parameterchanges.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/module.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/module_win32.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/stringconvert.cpp

SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/plugprovider.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/connectionproxy.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/eventlist.cpp
#SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/vsteditcontroller.cpp

SOURCES += $$VST3SDK_PATH/public.sdk/source/vst2.x/audioeffect.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst2.x/audioeffectx.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/basewrapper/basewrapper.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/vst2wrapper/vst2wrapper.cpp

LIBDEPS = sdk base pluginterfaces
for(a, LIBDEPS) {
    LIBS += -L$$VST3SDK_PATH/$${VSTLIB}/lib/$${MSBUILDDIR} -l$${LIBPREFIX}$${a}
    PRE_TARGETDEPS += $$VST3SDK_PATH/$${VSTLIB}/lib/$${MSBUILDDIR}/$${LIBPREFIX}$${a}.$${LIBEXT}
    DEPENDPATH += $$VST3SDK_PATH/$${VSTLIB}/lib/$${MSBUILDDIR}/$${a}
}
