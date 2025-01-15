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
    }
}

CONFIG += ordered
