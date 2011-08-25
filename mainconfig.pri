#===========================================================

DEFINES += APP_NAME=\\\"VstBoard\\\"

PORTAUDIO_PATH 	= $$PWD/libs/portaudio
PORTMIDI_PATH 	= $$PWD/libs/portmidi
VSTSDK_PATH	= $$PWD/libs/vstsdk2.4
VLD_PATH        = "C:/Program Files (x86)/Visual Leak Detector"

CONFIG += vstsdk                        #enable the vst SDK
CONFIG += scriptengine                  #enable the script engine
#CONFIG += ringBuffers                  #enable ring buffers to sync multiple input devices
#CONFIG += zeroDbClipping               #clip internal buffers at 0db
#CONFIG += simpleCables                 #draw cables as lines instead of curves

CONFIG(debug, debug|release) {          #only when debugging
#    CONFIG += vld                      #enable VLD memory leak detector
#    CONFIG += debugMutex                #log mutex activity
#    CONFIG += debugUnusedMutex         #report unused mutex
#    CONFIG += debugSemaphore            #log semaphore activity
#    CONFIG += debugUnusedSemaphore     #report unused semaphore
#    CONFIG += debugReadWriteLock        #log rwlocks activity
#    CONFIG += debugUnusedReadWriteLock  #report unused rwlocks
}

#===========================================================

CONFIG(debug, debug|release) {
    POST =
    build_postfix=debug
    DEFINES += DEBUG
} else {
    POST =
    build_postfix=release
}

win32-g++ {
    DEFINES += WINVER=0x0501
    DEFINES += _WIN32_WINNT=0x0501
 #   LIBS += -L$$quote($$MINGW_PATH/lib)
 #   INCLUDEPATH += $$quote($$MINGW_PATH/include)
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
