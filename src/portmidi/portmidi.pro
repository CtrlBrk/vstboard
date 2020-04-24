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

INCLUDEPATH += $$PORTMIDI_PATH/porttime
INCLUDEPATH += $$PORTMIDI_PATH/pm_common

SOURCES += $$PORTMIDI_PATH/pm_common/portmidi.c \
    $$PORTMIDI_PATH/pm_common/pmutil.c

win32:SOURCES += $$PORTMIDI_PATH/pm_win/pmwinmm.c \
    $$PORTMIDI_PATH/pm_win/pmwin.c \
    $$PORTMIDI_PATH/porttime/ptwinmm.c


linux-g++:SOURCES += $$PORTMIDI_PATH/pm_linux/pmlinux.c \
    $$PORTMIDI_PATH/pm_linux/finddefault.c \
    #$$PORTMIDI_PATH/pm_linux/pmlinuxalsa.c \
    $$PORTMIDI_PATH/porttime/ptlinux.c


