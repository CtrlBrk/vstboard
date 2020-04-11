DEFINES += APP_NAME=\\\"VstBoard\\\"
DEFINES += APP_VERSION_MAJOR=0
DEFINES += APP_VERSION_MINOR=6
DEFINES += APP_VERSION_BUILD=0
DEFINES += APP_VERSION_PATCH=0

#CONFIG += object_parallel_to_source

#Release:DESTDIR = $${OUT_PWD}/release
DESTDIR = $${OUT_PWD}/../

OBJECTS_DIR= $${OUT_PWD}
RCC_DIR = $${OUT_PWD}
MOC_DIR = $${OUT_PWD}
UI_DIR = $${DESTDIR}/ui

INCLUDEPATH += $${UI_DIR}

PORTAUDIO_PATH 	= ../../libs/portaudio
PORTMIDI_PATH 	= ../../libs/portmidi

win32|macx {
    CONFIG += vstsdk
    DEFINES += VSTSDK

    VST3SDK_PATH = "../../libs/VST_SDK/VST3_SDK"
    INCLUDEPATH += $$VST3SDK_PATH

}

#deprecated
#CONFIG += scriptengine

#use ring buffers to sync multiple audio inputs
DEFINES += CIRCULAR_BUFFER

#limit buffers range from -1.0 to 1.0
#DEFINES += BUFFER_ZERODB_CLIPPING

#use lines instead of curves
#DEFINES += SIMPLE_CABLES

#DEFINES += DEBUG_RENDERER
#DEFINES += DEBUG_BUFFERS
#DEFINES += DEBUG_OBJECTS
#DEFINES += DEBUG_DEVICES
#DEFINES += DEBUG_MESSAGES

#scriptengine {
#    DEFINES += SCRIPTENGINE
#    QT += script
#}

CONFIG(debug, debug|release) {
    POST =
    build_postfix=debug
    DEFINES += DEBUG
    DEFINES += _DEBUG
} else {
    POST =
    build_postfix=release
    DEFINES += RELEASE
}

win32-g++ {
    DEFINES += WINVER=0x0501
    DEFINES += _WIN32_WINNT=0x0501
    LIBPREFIX = lib
    LIBEXT = a
#    LIBS += -L$$quote($$MINGW_PATH/lib)
#    INCLUDEPATH += $$quote($$MINGW_PATH/include)
}

win32-msvc* {
    DEFINES += _CRT_SECURE_NO_WARNINGS
 #   INCLUDEPATH += $$quote($$(INCLUDE))
    LIBS += -L$$quote($$(LIB))
 #   QMAKE_CFLAGS += -Fd$$top_destdir/$$TARGET

    LIBPREFIX = ""
    LIBEXT = lib

#to add symbols :
#    QMAKE_CXXFLAGS_RELEASE +=  -Zi
#    QMAKE_LFLAGS_RELEASE += /DEBUG

#ltcg
    QMAKE_CXXFLAGS_RELEASE +=  -GL
    QMAKE_LFLAGS_RELEASE += /LTCG
}


#unix {
#    LIBPREFIX = lib
#    LIBEXT = a
#}

contains(QT_ARCH, i386) {
    VSTLIB = build
} else {
    VSTLIB = build64
}

CONFIG(debug, debug|release) {
    MSBUILDDIR = Debug
} else {
    MSBUILDDIR = Release
}
