include(config.pri)

TEMPLATE    = subdirs
SUBDIRS     = \
 #   portmidi \
    common \
    vstboard \

vstsdk {
    SUBDIRS += \
    vstdll \
 #   dllLoader
}

CONFIG += ordered
