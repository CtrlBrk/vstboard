/**************************************************************************
#    Copyright 2010-2012 RaphaÃ«l FranÃ§ois
#    Contact : ctrlbrk76@gmail.com
#
#    This file is part of VstBoard.
#
#    VstBoard is free software: you can redistribute it and/or modify
#    it under the terms of the under the terms of the GNU Lesser General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    VstBoard is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    under the terms of the GNU Lesser General Public License for more details.
#
#    You should have received a copy of the under the terms of the GNU Lesser General Public License
#    along with VstBoard.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#ifndef KEYBIND_H
#define KEYBIND_H

#include "precomp.h"
#include "settings.h"

class KeyBind : public QObject
{
    Q_OBJECT
    Q_ENUMS(MainShortcuts)
    Q_ENUMS(MoveInputs)
    Q_ENUMS(MovesBindings)

public:

    enum MainShortcuts {
        openProject,
        saveProject,
        saveProjectAs,
        newProject,
        openSetup,
        saveSetup,
        saveSetupAs,
        newSetup,
        hostPanel,
        projectPanel,
        programPanel,
        groupPanel,
        defaultLayout,
        toolBar,
        tools,
        vstPlugins,
        browser,
        programs,
        midiDevices,
        audioDevices,
        solverModel,
        hostModel,
        undoHistory,
        configuration,
        appearence,
        undo,
        redo,
        refreashAudioDevices,
        refreashMidiDevices,
        deleteObject,
        deleteObjectWithCables,
        toggleEditor,
        toggleLearnMode,
        toggleBypass,
        unplugPin,
        deletePin,
        saveBank,
        saveBankAs,
        saveProgram,
        saveProgramAs,
        zoomIn,
        zoomOut,
        zoomReset,
        hideAllEditors,
        autoOpenEditors
        /*
        increaseValue,
        decreaseValue

        */
    };

    enum MoveInputs {
        none,
        mouse,
        mouseWheel
    };

    enum MovesBindings {
        zoom,
        zoomResetMouse,
        moveView,
        moveObject,
        createCable,
        changeValue,
        changeCursorValue
    };

    struct MoveBind {
        KeyBind::MoveInputs input;
        Qt::MouseButtons buttons;
        Qt::KeyboardModifiers modifier;
    };

    KeyBind(Settings *settings);
    const QString GetMainShortcut(const MainShortcuts id) const;
    const MoveBind GetMoveSortcuts(const MovesBindings id) const;
    QStandardItemModel * GetMainBindingModel();
    void SetMainBindingModel(QStandardItemModel *model);
    QStandardItemModel * GetModesModel();
    void SetModesModel(QStandardItemModel *model);

    void SaveInRegistry();
    void LoadFromRegistry();
    void SetDefaults();
    void SetCurrentMode(const QString mode) {currentMode=mode;}

private:
    Settings *settings;
    QMap<MainShortcuts, QString>mapMainShortcuts;
    QMap<QString, QMap<MovesBindings, MoveBind> >mapModes;
    QStandardItemModel mainModel;
    QStandardItemModel modesModel;
    QString currentMode;

signals:
    void BindingChanged();
};

#endif // KEYBIND_H
