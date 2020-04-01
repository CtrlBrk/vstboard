#    Copyright 2010 Raphaël François
#    Contact : ctrlbrk76@gmail.com
#
#    This file is part of VstBoard.
#
#    VstBoard is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    VstBoard is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with VstBoard.  If not, see <http://www.gnu.org/licenses/>.

include(../config.pri)

CONFIG -= qt
QT -= core gui
TEMPLATE = lib
CONFIG += staticlib

DEFINES -= UNICODE

INCLUDEPATH += $$PORTAUDIO_PATH/include

linux-g++ {
    DEFINES += PA_USE_ALSA
    DEFINES += PA_USE_ASIHPI
    DEFINES += PA_USE_JACK
    DEFINES += PA_USE_OSS

    DEFINES += KSAUDIO_SPEAKER_DIRECTOUT=0
    DEFINES += METHOD_NEITHER=3
    DEFINES += FILE_ANY_ACCESS=0

    INCLUDEPATH += $$PORTAUDIO_PATH/src/os/unix

    HEADERS += $$PORTAUDIO_PATH/include/pa_jack.h
    HEADERS += $$PORTAUDIO_PATH/include/pa_linux_alsa.h

    SOURCES += $$PORTAUDIO_PATH/src/hostapi/alsa/pa_linux_alsa.c
    SOURCES += $$PORTAUDIO_PATH/src/hostapi/asihpi/pa_linux_asihpi.c
    SOURCES += $$PORTAUDIO_PATH/src/hostapi/jack/pa_jack.c
    #SOURCES += $$PORTAUDIO_PATH/src/hostapi/oss/pa_unix_oss.c
    SOURCES += $$PORTAUDIO_PATH/src/os/unix/pa_unix_hostapis.c
    SOURCES += $$PORTAUDIO_PATH/src/os/unix/pa_unix_util.c
}

#LIBS += -lwinmm
#LIBS += -luser32
#LIBS += -ladvapi32
#LIBS += -lole32
#LIBS += -lsetupapi
#LIBS += -ldsound

win32 {

    INCLUDEPATH += $$PORTAUDIO_PATH/src/os/win
    INCLUDEPATH += $$PORTAUDIO_PATH/src/hostapi/asio/ASIOSDK/common
    INCLUDEPATH += $$PORTAUDIO_PATH/src/hostapi/asio/ASIOSDK/host
    INCLUDEPATH += $$PORTAUDIO_PATH/src/hostapi/asio/ASIOSDK/host/pc

    HEADERS += $$PORTAUDIO_PATH/include/pa_win_waveformat.h
    HEADERS += $$PORTAUDIO_PATH/include/pa_win_wmme.h
    HEADERS += $$PORTAUDIO_PATH/include/pa_asio.h


    SOURCES += $$PORTAUDIO_PATH/src/hostapi/wmme/pa_win_wmme.c
    SOURCES += $$PORTAUDIO_PATH/src/os/win/pa_win_waveformat.c
    SOURCES += $$PORTAUDIO_PATH/src/os/win/pa_win_util.c
    SOURCES += $$PORTAUDIO_PATH/src/os/win/pa_win_hostapis.c
    SOURCES += $$PORTAUDIO_PATH/src/hostapi/asio/pa_asio.cpp
    SOURCES += $$PORTAUDIO_PATH/src/hostapi/asio/ASIOSDK/common/asio.cpp
    SOURCES += $$PORTAUDIO_PATH/src/hostapi/asio/ASIOSDK/host/ASIOConvertSamples.cpp
    SOURCES += $$PORTAUDIO_PATH/src/hostapi/asio/ASIOSDK/host/asiodrivers.cpp
    SOURCES += $$PORTAUDIO_PATH/src/hostapi/asio/ASIOSDK/host/pc/asiolist.cpp

    win32-g++ {
        DEFINES += PA_USE_WMME
        DEFINES += PA_USE_ASIO

        DEFINES += KSAUDIO_SPEAKER_DIRECTOUT=0
        DEFINES += METHOD_NEITHER=3
        DEFINES += FILE_ANY_ACCESS=0

        INCLUDEPATH += $$PORTAUDIO_PATH/src/hostapi/wasapi/mingw-include

        SOURCES += $$PORTAUDIO_PATH/src/hostapi/asio/iasiothiscallresolver.cpp

    }

    win32-msvc* {

        LIBS += -lwinmm
        LIBS += -luser32
        LIBS += -ladvapi32
        LIBS += -lole32
        LIBS += -lsetupapi
        LIBS += -ldsound

        DEFINES += PA_USE_WMME
        DEFINES += PA_USE_DS
        DEFINES += PA_USE_ASIO
        DEFINES += PA_USE_WASAPI
        DEFINES += PA_USE_WDMKS

        DEFINES += PAWIN_USE_DIRECTSOUNDFULLDUPLEXCREATE

        HEADERS += $$PORTAUDIO_PATH/include/pa_win_ds.h
        HEADERS += $$PORTAUDIO_PATH/include/pa_win_wasapi.h
        HEADERS += $$PORTAUDIO_PATH/src/os/win/pa_win_wdmks_utils.h
        HEADERS += $$PORTAUDIO_PATH/src/os/win/pa_x86_plain_converters.h

        SOURCES += $$PORTAUDIO_PATH/src/os/win/pa_win_coinitialize.c
        SOURCES += $$PORTAUDIO_PATH/src/os/win/pa_win_wdmks_utils.c
        SOURCES += $$PORTAUDIO_PATH/src/os/win/pa_x86_plain_converters.c

        SOURCES += $$PORTAUDIO_PATH/src/hostapi/wdmks/pa_win_wdmks.c
        SOURCES += $$PORTAUDIO_PATH/src/hostapi/dsound/pa_win_ds.c
        SOURCES += $$PORTAUDIO_PATH/src/hostapi/dsound/pa_win_ds_dynlink.c
        SOURCES += $$PORTAUDIO_PATH/src/hostapi/wasapi/pa_win_wasapi.c

    }



    INCLUDEPATH += $$PORTAUDIO_PATH/src/common
    INCLUDEPATH += $$PORTAUDIO_PATH/src/

    HEADERS += $$PORTAUDIO_PATH/include/portaudio.h

    SOURCES += $$PORTAUDIO_PATH/src/common/pa_allocation.c
    SOURCES += $$PORTAUDIO_PATH/src/common/pa_converters.c
    SOURCES += $$PORTAUDIO_PATH/src/common/pa_cpuload.c
    SOURCES += $$PORTAUDIO_PATH/src/common/pa_debugprint.c
    SOURCES += $$PORTAUDIO_PATH/src/common/pa_dither.c
    SOURCES += $$PORTAUDIO_PATH/src/common/pa_front.c
    SOURCES += $$PORTAUDIO_PATH/src/common/pa_process.c
    SOURCES += $$PORTAUDIO_PATH/src/common/pa_ringbuffer.c
    SOURCES += $$PORTAUDIO_PATH/src/common/pa_stream.c
    SOURCES += $$PORTAUDIO_PATH/src/common/pa_trace.c
}

mac {
    DEFINES += PA_USE_COREAUDIO

    INCLUDEPATH += $PORTAUDIO_PATH/src/os/mac_osx
    INCLUDEPATH += $$PORTAUDIO_PATH/src/hostapi/asio/ASIOSDK/common
    INCLUDEPATH += $$PORTAUDIO_PATH/src/hostapi/asio/ASIOSDK/host
    INCLUDEPATH += $$PORTAUDIO_PATH/src/hostapi/asio/ASIOSDK/host/mac

    HEADERS += $$PORTAUDIO_PATH/include/pa_mac_core.h

    SOURCES += $$PORTAUDIO_PATH/src/hostapi/coreaudio/pa_mac_core.c

    SOURCES += $$PORTAUDIO_PATH/src/hostapi/asio/pa_asio.cpp \
    $$PORTAUDIO_PATH/src/hostapi/asio/ASIOSDK/common/asio.cpp \
    $$PORTAUDIO_PATH/src/hostapi/asio/ASIOSDK/host/ASIOConvertSamples.cpp \
    $$PORTAUDIO_PATH/src/hostapi/asio/ASIOSDK/host/asiodrivers.cpp \
    $$PORTAUDIO_PATH/src/hostapi/asio/ASIOSDK/host/mac/asioshlib.cpp \
    $$PORTAUDIO_PATH/src/hostapi/asio/ASIOSDK/host/mac/codefragements.cpp
}
