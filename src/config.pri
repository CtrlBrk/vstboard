DEFINES += APP_NAME=\\\"VstBoard\\\"

PORTAUDIO_PATH 	= ../../libs/portaudio
PORTMIDI_PATH 	= ../../libs/portmidi

#win32|macx {
#    VSTSDK_PATH	= ../../libs/VST3SDK
#    CONFIG += vstsdk
#    DEFINES += VSTSDK
#    INCLUDEPATH += $$VSTSDK_PATH \
#        $$VSTSDK_PATH/public.sdk/source/vst2.x
#}

#CONFIG += scriptengine
#DEFINES += SCRIPTENGINE

#use ring buffers to sync multiple audio inputs
#DEFINES += CIRCULAR_BUFFER

#limit buffers range from -1.0 to 1.0
#DEFINES += BUFFER_ZERODB_CLIPPING

#use lines instead of curves
#DEFINES += SIMPLE_CABLES

scriptengine {
    QT += script
}

CONFIG(debug, debug|release) {
    POST =
    build_postfix=debug
    DEFINES += DEBUG
} else {
    POST =
    build_postfix=release
    DEFINES += RELEASE
}

win32-g++ {
    DEFINES += WINVER=0x0501
    DEFINES += _WIN32_WINNT=0x0501
#    LIBS += -L$$quote($$MINGW_PATH/lib)
#    INCLUDEPATH += $$quote($$MINGW_PATH/include)
}

win32-msvc* {
    DEFINES += _CRT_SECURE_NO_WARNINGS
    INCLUDEPATH += $$quote($$(INCLUDE))
    LIBS += -L$$quote($$(LIB))
 #   QMAKE_CFLAGS += -Fd$$top_destdir/$$TARGET

#to add symbols :
#    QMAKE_CXXFLAGS_RELEASE +=  -Zi
#    QMAKE_LFLAGS_RELEASE += /DEBUG

#ltcg
    QMAKE_CXXFLAGS_RELEASE +=  -GL
    QMAKE_LFLAGS_RELEASE += /LTCG
}
