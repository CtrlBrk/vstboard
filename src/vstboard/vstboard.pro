include(../config.pri)
#include(../vstsdk.pri)

QT += core gui widgets

TEMPLATE = app
TARGET = "VstBoard"

#LIBS += -lportaudio
LIBS += -lportmidi
LIBS += -lrtaudio

INCLUDEPATH += $$RTAUDIO

win32 {
    LIBS += -lwinmm
    LIBS += -luser32
    LIBS += -ladvapi32
    LIBS += -lole32
    LIBS += -lsetupapi
    LIBS += -ldsound
}
linux-g++ {
    LIBS += -ldl
    LIBS += -lstdc++fs
#    LIBS += -lasound
#    LIBS += -ljack
}

#INCLUDEPATH += $$PORTAUDIO_PATH/include
INCLUDEPATH += $$PORTMIDI_PATH/porttime
INCLUDEPATH += $$PORTMIDI_PATH/pm_common

INCLUDEPATH += ../common

CONFIG += precompile_header
PRECOMPILED_HEADER = ../common/precomp.h

#CONFIG(debug, debug|release) {
#    LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/" -lvld
#    INCLUDEPATH += "C:/Program Files (x86)/Visual Leak Detector/include/"
#}

SOURCES += \
    main.cpp \
    mainhosthost.cpp \
    connectables/objectfactoryhost.cpp \
    mainwindowhost.cpp \
    connectables/audiodevice.cpp \
    connectables/audiodevicein.cpp \
    connectables/audiodeviceout.cpp \
    models/listmidiinterfacesmodel.cpp \
    models/listaudiointerfacesmodel.cpp \
    connectables/mididevice.cpp \
    audiodevices.cpp \
    mididevices.cpp \
    views/configdialoghost.cpp \

HEADERS += \
    mainhosthost.h \
    mainwindowhost.h \
    connectables/objectfactoryhost.h \
    models/listaudiointerfacesmodel.h \
    connectables/audiodevice.h \
    connectables/audiodevicein.h \
    connectables/audiodeviceout.h \
    models/listmidiinterfacesmodel.h \
    connectables/mididevice.h \
    audiodevices.h \
    mididevices.h \
    views/configdialoghost.h \


#win32 {
#SOURCES += views/directxconfigdialog.cpp \
#    views/mmeconfigdialog.cpp \
#    views/wasapiconfigdialog.cpp

#HEADERS += views/directxconfigdialog.h \
#    views/mmeconfigdialog.h \
#    views/wasapiconfigdialog.h

#FORMS += \
#    views/directxconfigdialog.ui \
#    views/mmeconfigdialog.ui \
#    views/wasapiconfigdialog.ui
#}

#TRANSLATIONS = ../resources/translations/vstboard_fr.ts

RESOURCES += ../resources/resources.qrc

win32-msvc* {
    RC_FILE = vstboard.rc
}


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
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/module_mac.cpp
}
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/plugprovider.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/common/memorystream.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/connectionproxy.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/hostclasses.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/pluginterfacesupport.cpp

LIBDEPS = common base pluginterfaces sdk
for(a, LIBDEPS) {
    LIBS += -L$$DESTDIR -l$${a}
    PRE_TARGETDEPS += $$DESTDIR/$${LIBPREFIX}$${a}.$${LIBEXT}
    INCLUDEPATH += $$DESTDIR/$${a}
    DEPENDPATH += $$DESTDIR/$${a}
}
