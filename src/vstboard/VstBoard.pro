include(../config.pri)

QT += core gui widgets

TEMPLATE = app
TARGET = "VstBoard"

win32 {
    LIBS += -lwinmm
    LIBS += -luser32
    LIBS += -ladvapi32
    LIBS += -lole32
    LIBS += -lsetupapi
    LIBS += -ldsound
}

INCLUDEPATH += $$PORTAUDIO_PATH/include
INCLUDEPATH += $$PORTMIDI_PATH/porttime
INCLUDEPATH += $$PORTMIDI_PATH/pm_common

INCLUDEPATH += ../common

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
    views/directxconfigdialog.cpp \
    views/mmeconfigdialog.cpp \
    views/wasapiconfigdialog.cpp

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
    views/directxconfigdialog.h \
    views/mmeconfigdialog.h \
    views/wasapiconfigdialog.h


FORMS += \
    views/directxconfigdialog.ui \
    views/mmeconfigdialog.ui \
    views/wasapiconfigdialog.ui

#TRANSLATIONS = ../resources/translations/vstboard_fr.ts

RESOURCES += ../resources/resources.qrc

win32-msvc* {
    RC_FILE = vstboard.rc
}

#common
win32-g++:LIBS += -L$$DESTDIR -llibcommon
else:win32:LIBS += -L$$DESTDIR -lcommon
else:unix:LIBS += -L$$DESTDIR -lcommon

INCLUDEPATH += $$DESTDIR/common
DEPENDPATH += $$DESTDIR/common

win32-g++:PRE_TARGETDEPS += $$DESTDIR/libcommon.a
else:win32:PRE_TARGETDEPS += $$DESTDIR/common.lib
else:unix:PRE_TARGETDEPS += $$DESTDIR/libcommon.a

#portaudio
win32-g++:LIBS += -L$$DESTDIR -llibportaudio
else:win32:LIBS += -L$$DESTDIR -lportaudio
else:unix:LIBS += -L$$DESTDIR -lportaudio

INCLUDEPATH += $$DESTDIR/portaudio
DEPENDPATH += $$DESTDIR/portaudio

win32-g++:PRE_TARGETDEPS += $$DESTDIR/libportaudio.a
else:win32:PRE_TARGETDEPS += $$DESTDIR/portaudio.lib
else:unix:PRE_TARGETDEPS += $$DESTDIR/libportaudio.a

#portmidi
win32-g++:LIBS += -L$$DESTDIR -llibportmidi
else:win32:LIBS += -L$$DESTDIR -lportmidi
else:unix:LIBS += -L$$DESTDIR -lportmidi

INCLUDEPATH += $$DESTDIR/portmidi
DEPENDPATH += $$DESTDIR/portmidi

win32-g++:PRE_TARGETDEPS += $$DESTDIR/libportmidi.a
else:win32:PRE_TARGETDEPS += $$DESTDIR/portmidi.lib
else:unix:PRE_TARGETDEPS += $$DESTDIR/libportmidi.a
