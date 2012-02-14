include(../config.pri)

QT += core gui

TEMPLATE = lib
CONFIG += staticlib

vstsdk {
    HEADERS += vst/cvsthost.h \
        connectables/vstplugin.h \
        vst/ceffect.h \
        vst/const.h \
        vst/vstbank.h \
        vst/vstbankbase.h \
        vst/vstprogram.h \
        views/vstpluginwindow.h \
        views/vstshellselect.h

    SOURCES += vst/cvsthost.cpp \
        connectables/vstplugin.cpp \
        vst/ceffect.cpp \
        vst/vstbank.cpp \
        vst/vstbankbase.cpp \
        vst/vstprogram.cpp \
        views/vstpluginwindow.cpp \
        views/vstshellselect.cpp \
        $$VSTSDK_PATH/public.sdk/source/vst2.x/audioeffectx.cpp \
        $$VSTSDK_PATH/public.sdk/source/vst2.x/audioeffect.cpp \

    SOURCES += vst/vst3host.cpp \
        connectables/vst3plugin.cpp \
        $$VSTSDK_PATH/base/source/fobject.cpp \
        $$VSTSDK_PATH/base/source/fstring.cpp \
        $$VSTSDK_PATH/base/source/fatomic.cpp \
        $$VSTSDK_PATH/base/source/updatehandler.cpp \
        $$VSTSDK_PATH/base/source/fdebug.cpp \
        $$VSTSDK_PATH/base/source/fthread.cpp \
        $$VSTSDK_PATH/base/source/baseiids.cpp \
        $$VSTSDK_PATH/pluginterfaces/base/ustring.cpp \
        $$VSTSDK_PATH/pluginterfaces/base/funknown.cpp \
        $$VSTSDK_PATH/pluginterfaces/base/conststringtable.cpp \
        $$VSTSDK_PATH/public.sdk/source/main/pluginfactoryvst3.cpp \
        $$VSTSDK_PATH/public.sdk/source/common/pluginview.cpp \
        $$VSTSDK_PATH/public.sdk/source/common/memorystream.cpp \
        $$VSTSDK_PATH/public.sdk/source/vst/vstaudioeffect.cpp \
        $$VSTSDK_PATH/public.sdk/source/vst/vstcomponent.cpp \
        $$VSTSDK_PATH/public.sdk/source/vst/vstcomponentbase.cpp \
        $$VSTSDK_PATH/public.sdk/source/vst/vstparameters.cpp \
        $$VSTSDK_PATH/public.sdk/source/vst/vstbus.cpp \
        $$VSTSDK_PATH/public.sdk/source/vst/vsteditcontroller.cpp \
        $$VSTSDK_PATH/public.sdk/source/vst/vstinitiids.cpp \
        $$VSTSDK_PATH/public.sdk/source/vst/hosting/hostclasses.cpp \
        $$VSTSDK_PATH/public.sdk/source/vst/hosting/processdata.cpp \
        $$VSTSDK_PATH/public.sdk/source/vst/hosting/parameterchanges.cpp


    FORMS += views/vstpluginwindow.ui \
        views/vstshellselect.ui
}

scriptengine {
    QT += script

    SOURCES += connectables/script.cpp \
        views/scripteditor.cpp \

    HEADERS += connectables/script.h \
        views/scripteditor.h \

    FORMS += views/scripteditor.ui
}

PRECOMPILED_HEADER = precomp.h

SOURCES += \
    mainhost.cpp \
    audiobuffer.cpp \
    mainwindow.cpp \
    circularbuffer.cpp \
    connectables/midipinin.cpp \
    connectables/midipinout.cpp \
    connectables/bridgepinin.cpp \
    connectables/bridgepinout.cpp \
    connectables/cable.cpp \
    connectables/objectfactory.cpp \
    connectables/object.cpp \
    connectables/container.cpp \
    connectables/connectioninfo.cpp \
    connectables/objectinfo.cpp \
    connectables/pin.cpp \
    connectables/parameterpin.cpp \
    connectables/parameterpinin.cpp \
    connectables/parameterpinout.cpp \
    connectables/bridge.cpp \
    connectables/miditoautomation.cpp \
    connectables/midisender.cpp \
    connectables/pinslist.cpp \
    connectables/hostcontroller.cpp \
    connectables/audiopin.cpp \
    connectables/buffer.cpp \
    renderer/pathsolver.cpp \
    renderer/renderer.cpp \
    renderer/solvernode.cpp \
    renderer/renderthread.cpp \
    renderer/optimizerstep.cpp \
    renderer/optimizer.cpp \
    renderer/optimizestepthread.cpp \
    renderer/renderernode.cpp \
    renderer/node.cpp \
    connectables/objectparameter.cpp \
    connectables/objectprogram.cpp \
    connectables/containerprogram.cpp \
    projectfile/projectfile.cpp \
    models/listtoolsmodel.cpp \
    models/hostmodel.cpp \
    commands/comdisconnectpin.cpp \
    commands/comaddobject.cpp \
    commands/comaddcable.cpp \
    commands/comremoveobject.cpp \
    commands/comaddprogram.cpp \
    commands/comremoveprogram.cpp \
    commands/comremovegroup.cpp \
    commands/comaddgroup.cpp \
    commands/comchangeautosave.cpp \
    commands/comremovepin.cpp \
    commands/comaddpin.cpp \
    views/keybindingdialog.cpp \
    views/buttonswidget.cpp \
    views/modifierswidget.cpp \
    renderer/updatedelays.cpp \
    settings.cpp \
    msgobject.cpp \
    msghandler.cpp \
    msgcontroller.cpp \
    connectables/cursor.cpp \
    programmanager.cpp \
    models/groupsprogramsmodel.cpp \
    models/parkingmodel.cpp \
    commands/comprogramstate.cpp


SOURCES += \
    views/programlist.cpp \
    views/filebrowser.cpp \
    views/grouplistview.cpp \
    views/proglistview.cpp \
    views/aboutdialog.cpp \
    views/configdialog.cpp \
    views/maingraphicsview.cpp \
    views/splash.cpp \
    views/viewconfigdialog.cpp \
    views/viewconfig.cpp \
    views/listaudiodevicesview.cpp \
    views/gradientwidget.cpp \
    views/gradientwidgethue.cpp \
    views/keybind.cpp
	
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
    audiobuffer.h \
    mainwindow.h \
    circularbuffer.h \
    connectables/midisender.h \
    connectables/miditoautomation.h \
    connectables/hostcontroller.h \
    connectables/midipinout.h \
    connectables/bridgepinin.h \
    connectables/bridgepinout.h \
    connectables/cable.h \
    connectables/objectfactory.h \
    connectables/object.h \
    connectables/container.h \
    connectables/connectioninfo.h \
    connectables/objectinfo.h \
    connectables/pin.h \
    connectables/parameterpin.h \
    connectables/parameterpinin.h \
    connectables/parameterpinout.h \
    connectables/bridge.h \
    connectables/midipinin.h \
    connectables/pinslist.h \
    connectables/audiopin.h \
    connectables/buffer.h \
    renderer/solvernode.h \
    renderer/pathsolver.h \
    renderer/renderer.h \
    renderer/renderthread.h \
    renderer/optimizerstep.h \
    renderer/optimizer.h \
    renderer/optimizestepthread.h \
    renderer/renderernode.h \
    renderer/node.h \
    connectables/objectparameter.h \
    connectables/objectprogram.h \
    connectables/containerprogram.h \
    projectfile/projectfile.h \
    projectfile/fileversion.h \
    models/listtoolsmodel.h \
    models/hostmodel.h \
    commands/comdisconnectpin.h \
    commands/comaddobject.h \
    commands/comaddcable.h \
    commands/comremoveobject.h \
    commands/comaddprogram.h \
    commands/comremoveprogram.h \
    commands/comremovegroup.h \
    commands/comaddgroup.h \
    commands/comchangeautosave.h \
    commands/comremovepin.h \
    commands/comaddpin.h \
    views/keybindingdialog.h \
    views/buttonswidget.h \
    views/modifierswidget.h \
    renderer/updatedelays.h \
    settings.h \
    msgobject.h \
    msghandler.h \
    msgcontroller.h \
    connectables/cursor.h \
    programmanager.h \
    models/groupsprogramsmodel.h \
    models/parkingmodel.h \
    connectables/vst3plugin.h \
    vst/vst3host.h \
    commands/comprogramstate.h

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
    views/viewconfig.h \
    views/listaudiodevicesview.h \
    views/gradientwidget.h \
    views/gradientwidgethue.h \
    views/keybind.h
	
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
    sceneview/objectdropzone.h
    
FORMS += \
    mainwindow.ui \
    views/configdialog.ui \
    views/filebrowser.ui \
    views/programlist.ui \
    views/aboutdialog.ui \
    views/splash.ui \
    views/viewconfigdialog.ui \
    views/keybindingdialog.ui \
    views/buttonswidget.ui \
    views/modifierswidget.ui


PRECOMPILED_HEADER = precomp.h

TRANSLATIONS = ../resources/translations/common_fr.ts

RESOURCES += \
    ../resources/resources.qrc























