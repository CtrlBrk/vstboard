
include(../config.pri)

TEMPLATE = app
TARGET = "VstBoard"

LIBS += -ladvapi32
LIBS += -luser32
LIBS += -lcomdlg32

SOURCES += \
    main.cpp \

HEADERS += \
    loaderhelpers.h

win32-msvc* {
    RC_FILE = launcher.rc
}
