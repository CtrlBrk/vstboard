include(../config.pri)

QT += core gui widgets
#QT += widgets-private
TEMPLATE = lib
CONFIG += staticlib

PRECOMPILED_HEADER = precomp.h

vstsdk {
    HEADERS += vst/vst3host.h \
        connectables/vst3plugin.h \
        vst/cvsthost.h \
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

#scriptengine {
#    SOURCES += connectables/script.cpp \
#        views/scripteditor.cpp \

#    HEADERS += connectables/script.h \
#        views/scripteditor.h \

#    FORMS += views/scripteditor.ui
#}

SOURCES += \
    connectables/objectcontainerattribs.cpp \
    msghandler.cpp \
    mainhost.cpp \
    audiobuffer.cpp \
    mainwindow.cpp \
    circularbuffer.cpp \
    projectfile/jsonreader.cpp \
    projectfile/jsonwriter.cpp \
    settings.cpp \
    msgobject.cpp \
    msgcontroller.cpp \
    programmanager.cpp \
    connectables/objectinfo.cpp \
    connectables/midipinin.cpp \
    connectables/midipinout.cpp \
    connectables/bridgepinin.cpp \
    connectables/bridgepinout.cpp \
    connectables/cable.cpp \
    connectables/objectfactory.cpp \
    connectables/object.cpp \
    connectables/container.cpp \
    connectables/connectioninfo.cpp \
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
    connectables/objectparameter.cpp \
    connectables/objectprogram.cpp \
    connectables/containerprogram.cpp \
    connectables/cursor.cpp \
    renderer/solvernode.cpp \
    renderer/node.cpp \
    renderer/updatedelays.cpp \
    renderer/renderer2.cpp \
    renderer/rendererthread2.cpp \
    renderer/renderernode2.cpp \
    renderer/optimizernode.cpp \
    renderer/optimizemap.cpp \
    renderer/semaphoreinverted.cpp \
    renderer/waitall.cpp \
    renderer/rendermap.cpp \
    renderer/solver.cpp \
    projectfile/projectfile.cpp \
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
    commands/comprogramstate.cpp \
    models/listtoolsmodel.cpp \
    models/groupsprogramsmodel.cpp \
    models/parkingmodel.cpp \
    renderer/pathsolver.cpp \
    views/keypressedwidget.cpp

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
    views/keybind.cpp \
    views/keybindingdialog.cpp \
    views/buttonswidget.cpp \
    views/modifierswidget.cpp \
	
SOURCES += \
    sceneview/objectview.cpp \
    sceneview/sceneview.cpp \
    sceneview/cableview.cpp \
    sceneview/maincontainerview.cpp \
    sceneview/connectableobjectview.cpp \
    sceneview/containercontent.cpp \
    sceneview/bridgeview.cpp \
    sceneview/listpinsview.cpp \
    sceneview/vstpluginview.cpp \
    sceneview/objectdropzone.cpp \
    sceneview/pinview.cpp \
    sceneview/minmaxpinview.cpp \
    sceneview/cursorview.cpp \
    sceneview/connectablepinview.cpp \
    sceneview/bridgepinview.cpp

HEADERS += \
    connectables/objectcontainerattribs.h \
    globals.h \
    mainhost.h \
    audiobuffer.h \
    mainwindow.h \
    circularbuffer.h \
    projectfile/jsonreader.h \
    projectfile/jsonwriter.h \
    settings.h \
    msgobject.h \
    msghandler.h \
    msgcontroller.h \
    programmanager.h \
    connectables/objectparameter.h \
    connectables/objectprogram.h \
    connectables/containerprogram.h \
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
    connectables/cursor.h \
    renderer/solvernode.h \
    renderer/node.h \
    renderer/optimizernode.h \
    renderer/optimizemap.h \
    renderer/semaphoreinverted.h \
    renderer/renderer2.h \
    renderer/rendererthread2.h \
    renderer/renderernode2.h \
    renderer/updatedelays.h \
    renderer/waitall.h \
    renderer/rendermap.h \
    renderer/solver.h \
    projectfile/projectfile.h \
    projectfile/fileversion.h \
    models/listtoolsmodel.h \
    models/groupsprogramsmodel.h \
    models/parkingmodel.h \
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
    commands/comprogramstate.h \
    renderer/pathsolver.h \
    views/keypressedwidget.h

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
    views/keybind.h \
    views/keybindingdialog.h \
    views/buttonswidget.h \
    views/modifierswidget.h \
	
HEADERS += \
    sceneview/objectview.h \
    sceneview/sceneview.h \
    sceneview/cableview.h \
    sceneview/maincontainerview.h \
    sceneview/connectableobjectview.h \
    sceneview/containercontent.h \
    sceneview/bridgeview.h \
    sceneview/listpinsview.h \
    sceneview/vstpluginview.h \
    sceneview/objectdropzone.h \
    sceneview/pinview.h \
    sceneview/minmaxpinview.h \
    sceneview/cursorview.h \
    sceneview/connectablepinview.h \
    sceneview/bridgepinview.h
    
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


#TRANSLATIONS = ../resources/translations/common_fr.ts

RESOURCES += \
    ../resources/resources.qrc























