include(config.pri)

TEMPLATE    = subdirs

SUBDIRS     = common

portaudio {
    SUBDIRS += portaudio
    SUBDIRS += portmidi
}

SUBDIRS += vstboard

vstsdk {
    SUBDIRS += vstdll

    win32 {
        SUBDIRS += dllLoader
        SUBDIRS += dllLoaderInst
    }
}

SUBDIRS += clapLoader
SUBDIRS += clapdll

SUBDIRS += launcher

CONFIG += ordered
