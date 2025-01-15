include(../config.pri)
#include(../vstsdk.pri)

QT += core gui widgets

TEMPLATE = app
TARGET = "VstBoard"


LIBS += -L$$VST3SDK_LIB -lbase
LIBS += -L$$VST3SDK_LIB -lpluginterfaces
LIBS += -L$$VST3SDK_LIB -lsdk
LIBS += -L$$VST3SDK_LIB -lsdk_common
LIBS += -L$$VST3SDK_LIB -lsdk_hosting

LIBDEPS = common

rtaudio {

    LIBS += -L$$RTMIDI_LIB -lrtmidi

    CONFIG(debug, debug|release) {
        LIBS += -L$$RTAUDIO_LIB -lrtaudiod
    } else {
        LIBS += -L$$RTAUDIO_LIB -lrtaudio
    }

    INCLUDEPATH += $$RTMIDI
    INCLUDEPATH += $$RTAUDIO

SOURCES += \
    audiodevices_rt.cpp \
    mididevices_rm.cpp \
    connectables/audiodevice_rt.cpp \
    connectables/mididevice_rm.cpp \

HEADERS += \
    audiodevices_rt.h \
    mididevices_rm.h \
    connectables/audiodevice_rt.h \
    connectables/mididevice_rm.h \

}

portaudio {
    LIBDEPS += portaudio portmidi

    INCLUDEPATH += $$PORTAUDIO_PATH/include
    INCLUDEPATH += $$PORTMIDI_PATH/porttime
    INCLUDEPATH += $$PORTMIDI_PATH/pm_common

    LIBS += -lportaudio
    LIBS += -lportmidi

SOURCES += \
    audiodevices_pa.cpp \
    mididevices_pm.cpp \
    connectables/audiodevice_pa.cpp \
    connectables/mididevice_pm.cpp \

HEADERS += \
    audiodevices_pa.h \
    mididevices_pm.h \
    connectables/audiodevice_pa.h \
    connectables/mididevice_pm.h \

win32 {
SOURCES += views/directxconfigdialog.cpp \
    views/mmeconfigdialog.cpp \
    views/wasapiconfigdialog.cpp

HEADERS += views/directxconfigdialog.h \
    views/mmeconfigdialog.h \
    views/wasapiconfigdialog.h

FORMS += \
    views/directxconfigdialog.ui \
    views/mmeconfigdialog.ui \
    views/wasapiconfigdialog.ui
}

}

for(a, LIBDEPS) {
    LIBS += -L$$DESTDIR -l$${a}
    PRE_TARGETDEPS += $$DESTDIR/$${LIBPREFIX}$${a}.$${LIBEXT}
    INCLUDEPATH += $$DESTDIR/$${a}
    DEPENDPATH += $$DESTDIR/$${a}
}


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
    connectables/audiodevicein.cpp \
    connectables/audiodeviceout.cpp \
    models/listmidiinterfacesmodel.cpp \
    models/listaudiointerfacesmodel.cpp \
    views/configdialoghost.cpp \

HEADERS += \
    mainhosthost.h \
    mainwindowhost.h \
    connectables/objectfactoryhost.h \
    models/listaudiointerfacesmodel.h \
    connectables/audiodevicein.h \
    connectables/audiodeviceout.h \
    models/listmidiinterfacesmodel.h \
    views/configdialoghost.h \



#TRANSLATIONS = ../resources/translations/vstboard_fr.ts

RESOURCES += ../resources/resources.qrc

win32-msvc* {
    RC_FILE = vstboard.rc
}

# SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/eventlist.cpp
# SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/parameterchanges.cpp
# SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/processdata.cpp
# SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/module.cpp
# SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/utility/stringconvert.cpp
win32 {
 SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/module_win32.cpp
# SOURCES += $$VST3SDK_PATH/public.sdk/source/common/threadchecker_win32.cpp
}
linux-g++{
 SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/module_linux.cpp
# SOURCES += $$VST3SDK_PATH/public.sdk/source/common/threadchecker_linux.cpp
}
macx{
 SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/module_mac.mm
# SOURCES += $$VST3SDK_PATH/public.sdk/source/common/threadchecker_mac.cpp
}
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/plugprovider.cpp
# SOURCES += $$VST3SDK_PATH/public.sdk/source/common/commoniids.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/common/memorystream.cpp
# SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/connectionproxy.cpp
# SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/hostclasses.cpp
# SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/pluginterfacesupport.cpp


