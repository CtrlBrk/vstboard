include(../config.pri)

QT += core gui

TEMPLATE = lib
CONFIG += staticlib

vstsdk {
    HEADERS += vst/cvsthost.h \
        connectables/objects/vstplugin.h \
        vst/ceffect.h \
        vst/const.h \
        vst/vstbank.h \
        views/vstpluginwindow.h \
        views/vstshellselect.h

    SOURCES += vst/cvsthost.cpp \
        connectables/objects/vstplugin.cpp \
        vst/ceffect.cpp \
        vst/vstbank.cpp \
        views/vstpluginwindow.cpp \
        views/vstshellselect.cpp \
        $$VSTSDK_PATH/public.sdk/source/vst2.x/audioeffectx.cpp \
        $$VSTSDK_PATH/public.sdk/source/vst2.x/audioeffect.cpp

    FORMS += views/vstpluginwindow.ui \
        views/vstshellselect.ui
}

scriptengine {
    SOURCES += connectables/objects/script.cpp \
        views/scripteditor.cpp \

    HEADERS += connectables/objects/script.h \
        views/scripteditor.h \

    FORMS += views/scripteditor.ui
}

PRECOMPILED_HEADER = precomp.h

SOURCES += \
    mainwindow.cpp \
    mainhost.cpp \
    audiobuffer.cpp \
    objectinfo.cpp \
    viewconfig.cpp \
    projectfile/projectfile.cpp \
    models/listtoolsmodel.cpp \
    models/programsmodel.cpp \
    vst/vstbankbase.cpp \
    vst/vstprogram.cpp \
    renderer/pathsolver.cpp \
    renderer/renderer.cpp \
    renderer/renderthread.cpp \
    renderer/optimizerstep.cpp \
    renderer/optimizer.cpp \
    renderer/optimizestepthread.cpp \
    renderer/renderernode.cpp \
    renderer/node.cpp \
    renderer/solvernode.cpp \
    connectables/cable.cpp \
    connectables/objectparameter.cpp \
    connectables/objectfactory.cpp \
    connectables/pinslist.cpp \
    connectables/objectprogram.cpp \
    connectables/containerprogram.cpp \
    connectables/objects/object.cpp \
    connectables/objects/container.cpp \
    connectables/objects/bridge.cpp \
    connectables/objects/miditoautomation.cpp \
    connectables/objects/midisender.cpp \
    connectables/objects/hostcontroller.cpp \
    connectables/pins/pin.cpp \
    connectables/pins/parameterpin.cpp \
    connectables/pins/audiopin.cpp \
    connectables/pins/midipinin.cpp \
    connectables/pins/midipinout.cpp \
    connectables/pins/bridgepinin.cpp \
    connectables/pins/bridgepinout.cpp \
    commands/comdisconnectpin.cpp \
    commands/comaddobject.cpp \
    commands/comaddcable.cpp \
    commands/comremoveobject.cpp \
    commands/comchangeprogram.cpp \
    commands/comaddprogram.cpp \
    commands/comremoveprogram.cpp \
    commands/comremovegroup.cpp \
    commands/comaddgroup.cpp \
    commands/comchangeprogramitem.cpp \
    commands/comdiscardchanges.cpp \
    commands/comchangeautosave.cpp \
    commands/comremovepin.cpp \
    commands/comaddpin.cpp \
    models/scenemodel.cpp \
    commands/threadsafecom.cpp

SOURCES += \
    views/configdialog.cpp \
    views/maingraphicsview.cpp \
    views/programlist.cpp \
    views/filebrowser.cpp \
    views/grouplistview.cpp \
    views/proglistview.cpp \
    views/aboutdialog.cpp \
    views/splash.cpp \
    views/viewconfigdialog.cpp \
    views/listaudiodevicesview.cpp \
    views/gradientwidget.cpp \
    views/gradientwidgethue.cpp

SOURCES += \
    sceneview/connectablepinview.cpp \
    sceneview/objectview.cpp \
    sceneview/pinview.cpp \
    sceneview/sceneview.cpp \
    sceneview/cableview.cpp \
    sceneview/maincontainerview.cpp \
    sceneview/connectableobjectview.cpp \
    sceneview/bridgepinview.cpp \
    sceneview/containercontent.cpp \
    sceneview/bridgeview.cpp \
    sceneview/listpinsview.cpp \
    sceneview/minmaxpinview.cpp \
    sceneview/cursorview.cpp \
    sceneview/vstpluginview.cpp \
    sceneview/objectdropzone.cpp

HEADERS += \
    _version.h \
    globals.h \
    mainhost.h \
    mainwindow.h \
    audiobuffer.h \
    objectinfo.h \
    events.h \
    viewconfig.h \
    projectfile/projectfile.h \
    projectfile/fileversion.h \
    vst/vstbankbase.h \
    vst/vstprogram.h \
    models/programsmodel.h \
    models/listtoolsmodel.h \
    renderer/solvernode.h \
    renderer/renderthread.h \
    renderer/optimizerstep.h \
    renderer/optimizer.h \
    renderer/optimizestepthread.h \
    renderer/renderernode.h \
    renderer/node.h \
    renderer/pathsolver.h \
    renderer/renderer.h \
    connectables/objectprogram.h \
    connectables/containerprogram.h \
    connectables/cable.h \
    connectables/pinslist.h \
    connectables/objectparameter.h \
    connectables/objectfactory.h \
    connectables/objects/object.h \
    connectables/objects/container.h \
    connectables/objects/bridge.h \
    connectables/objects/midisender.h \
    connectables/objects/miditoautomation.h \
    connectables/objects/hostcontroller.h \
    connectables/pins/pin.h \
    connectables/pins/audiopin.h \
    connectables/pins/midipinin.h \
    connectables/pins/midipinout.h \
    connectables/pins/parameterpin.h \
    connectables/pins/bridgepinin.h \
    connectables/pins/bridgepinout.h \
    commands/comdisconnectpin.h \
    commands/comaddobject.h \
    commands/comaddcable.h \
    commands/comremoveobject.h \
    commands/comchangeprogram.h \
    commands/comaddprogram.h \
    commands/comremoveprogram.h \
    commands/comremovegroup.h \
    commands/comaddgroup.h \
    commands/comchangeprogramitem.h \
    commands/comdiscardchanges.h \
    commands/comchangeautosave.h \
    commands/comremovepin.h \
    commands/comaddpin.h \
    models/scenemodel.h \
    connectables/mididefines.h \
    debugmutex.h \
    debugreadwritelock.h \
    debugsemaphore.h \
    commands/threadsafecom.h

HEADERS += \
    views/configdialog.h \
    views/programlist.h \
    views/filebrowser.h \
    views/grouplistview.h \
    views/proglistview.h \
    views/aboutdialog.h \
    views/maingraphicsview.h \
    views/splash.h \
    views/viewconfigdialog.h \
    views/listaudiodevicesview.h \
    views/gradientwidget.h \
    views/gradientwidgethue.h \

HEADERS += \
    sceneview/objectview.h \
    sceneview/pinview.h \
    sceneview/connectablepinview.h \
    sceneview/sceneview.h \
    sceneview/cableview.h \
    sceneview/maincontainerview.h \
    sceneview/connectableobjectview.h \
    sceneview/bridgepinview.h \
    sceneview/containercontent.h \
    sceneview/bridgeview.h \
    sceneview/listpinsview.h \
    sceneview/minmaxpinview.h \
    sceneview/cursorview.h \
    sceneview/vstpluginview.h \
    sceneview/objectdropzone.h \

FORMS += \
    mainwindow.ui \
    views/configdialog.ui \
    views/filebrowser.ui \
    views/programlist.ui \
    views/aboutdialog.ui \
    views/splash.ui \
    views/viewconfigdialog.ui \

PRECOMPILED_HEADER = precomp.h

TRANSLATIONS = ../resources/translations/common_fr.ts

RESOURCES += \
    ../resources/resources.qrc






