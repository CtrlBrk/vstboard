include(../config.pri)

QT += core gui widgets

TEMPLATE = app
TARGET = "VstBoard"

LIBS += -lportmidi

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
    views/mmeconfigdialog.h \
    views/wasapiconfigdialog.h


FORMS += \
    views/mmeconfigdialog.ui \
    views/wasapiconfigdialog.ui

PRECOMPILED_HEADER = ../common/precomp.h

#TRANSLATIONS = ../resources/translations/vstboard_fr.ts

RESOURCES += ../resources/resources.qrc

win32-msvc* {
    RC_FILE = vstboard.rc
}

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../common/release/ -lcommon
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../common/debug/ -lcommon
else:unix:!symbian: LIBS += -L$$OUT_PWD/../common/ -lcommon

INCLUDEPATH += $$PWD/../common
DEPENDPATH += $$PWD/../common


win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../common/libcommon.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../common/libcommon.a
else:win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../common/release/common.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../common/debug/common.lib
else:unix:!symbian: PRE_TARGETDEPS += $$OUT_PWD/../common/libcommon.a

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../portaudio/release/ -lportaudio
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../portaudio/debug/ -lportaudio
else:unix:!symbian: LIBS += -L$$OUT_PWD/../portaudio/ -lportaudio

INCLUDEPATH += $$PWD/../portaudio
DEPENDPATH += $$PWD/../portaudio

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../portaudio/libportaudio.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../portaudio/libportaudio.a
else:win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../portaudio/release/portaudio.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../portaudio/debug/portaudio.lib
else:unix:!symbian: PRE_TARGETDEPS += $$OUT_PWD/../portaudio/libportaudio.a

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../portmidi/release/ -lportmidi
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../portmidi/debug/ -lportmidi
else:unix:!symbian: LIBS += -L$$OUT_PWD/../portmidi/ -lportmidi

INCLUDEPATH += $$PWD/../portmidi
DEPENDPATH += $$PWD/../portmidi

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../portmidi/libportmidi.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../portmidi/libportmidi.a
else:win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../portmidi/release/portmidi.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../portmidi/debug/portmidi.lib
else:unix:!symbian: PRE_TARGETDEPS += $$OUT_PWD/../portmidi/libportmidi.a
