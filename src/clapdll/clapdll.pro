include(../config.pri)

QT += core gui widgets

TARGET = "ClapBlib"
TEMPLATE = lib

QMAKE_EXTENSION_SHLIB = dllclap

LIBS += -L$$VST3SDK_LIB -lbase
LIBS += -L$$VST3SDK_LIB -lpluginterfaces
LIBS += -L$$VST3SDK_LIB -lsdk
LIBS += -L$$VST3SDK_LIB -lsdk_common
LIBS += -L$$VST3SDK_LIB -lsdk_hosting



CONFIG += precompile_header
PRECOMPILED_HEADER = ../common/precomp.h

CONFIG += clap
INCLUDEPATH += ../../libs/clap/include
INCLUDEPATH += ../../libs/clap-helpers/include


win32 {
    DEFINES += QT_QTWINMIGRATE_IMPORT
    INCLUDEPATH += ../../libs/qtwinmigrate/src

    CONFIG(debug, debug|release) {
        LIBS += -L$${_PRO_FILE_PWD_}/../../libs/qtwinmigrate/lib -lQtSolutions_MFCMigrationFramework-headd
    } else {
        LIBS += -L$${_PRO_FILE_PWD_}/../../libs/qtwinmigrate/lib -lQtSolutions_MFCMigrationFramework-head
    }

}

win32 {
    LIBS += -lUser32
    LIBS += -lwinmm
    LIBS += -ladvapi32
    LIBS += -lws2_32
    LIBS += -lole32
    LIBS += -lcomdlg32
}

win32-g++ {
    QMAKE_LFLAGS += -static-libgcc
}

INCLUDEPATH += ../common

SOURCES += \
    clapmainhost.cpp \
    connectables/objectfactoryclap.cpp \
    main.cpp \
    gui.cpp \
    mainwindowclap.cpp

HEADERS  += \
    clapmainhost.h \
    connectables/objectfactoryclap.h \
    gui.h \
    mainwindowclap.h


RESOURCES += ../resources/resources.qrc


LIBDEPS = common
for(a, LIBDEPS) {
    LIBS += -L$$DESTDIR -l$${a}
    PRE_TARGETDEPS += $$DESTDIR/$${LIBPREFIX}$${a}.$${LIBEXT}
    INCLUDEPATH += $$DESTDIR/$${a}
    DEPENDPATH += $$DESTDIR/$${a}
}

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
