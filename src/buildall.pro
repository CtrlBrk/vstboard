include(config.pri)

TEMPLATE    = subdirs
SUBDIRS     = \
    portaudio \
    portmidi \
    common \
    vstboard

vstsdk {
    SUBDIRS += \
    vstdll \
    dllLoader
}

CONFIG += ordered
