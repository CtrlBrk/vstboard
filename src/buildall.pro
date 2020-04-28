include(config.pri)

TEMPLATE    = subdirs
SUBDIRS     = \
    common \
    vstboard \

vstsdk {
    SUBDIRS += vstdll
    win32 {
        SUBDIRS += dllLoader
    }
}

CONFIG += ordered
