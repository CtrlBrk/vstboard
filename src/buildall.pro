include(config.pri)

TEMPLATE    = subdirs
SUBDIRS     = \
    portmidi \
    portaudio \
    common \
    vstboard

vstsdk {
    SUBDIRS += \
        vstdll \
        dllLoader
}

CONFIG += ordered
