/**************************************************************************
#    Copyright 2010-2012 Raphaël François
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//#include "precomp.h"
#include "mainhost.h"
#include "models/listtoolsmodel.h"
#include "sceneview/sceneview.h"
#include "views/viewconfig.h"
#include "views/keybind.h"
#include "settings.h"
#include "msgcontroller.h"
#include "models/groupsprogramsmodel.h"
#include "models/parkingmodel.h"
#include "views/vstshellselect.h"

namespace Ui {
    class MainWindow;
}

namespace View {
    class ViewConfigDialog;
}

class MainWindow : public QMainWindow , public MsgController
{
    Q_OBJECT

public:
    MainWindow(Settings *settings, MainHost * myHost=0, QWidget *parent = 0);
    virtual ~MainWindow();
    virtual void Init();
    virtual void readSettings();
    void writeSettings();
    void ReceiveMsg(const MsgObject &msg);
//    void ReceiveMsg(const QString &type, const QVariant &data);
//    void ProcessMsg(const ListMsgObj &lstMsg);

    View::SceneView *mySceneView;
    View::ViewConfig *viewConfig;

    Settings *settings;

protected:
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);
    void changeEvent(QEvent *e);
    void SetupBrowsersModels(const QString &vstPath, const QString &browserPath);
    void currentFileChanged(const MsgObject &msg);
    void UpdateSolverMap(const MsgObject &msg);

    ListToolsModel *listToolsModel;

    QFileSystemModel *listVstPluginsModel;
    QFileSystemModel *listVstBanksModel;

    virtual void BuildListTools();
    virtual void resetSettings();
    void updateRecentFileActions();

    QList<QAction*>listRecentProjects;
    QList<QAction*>listRecentSetups;

    Ui::MainWindow *ui;
    MainHost *myHost;

    QAction *actUndo;
    QAction *actRedo;

    View::ViewConfigDialog *viewConfigDlg;

    QList<QWeakPointer<Connectables::Object> >listClosedEditors;
    GroupsProgramsModel *progModel;

    ParkingModel *groupParking;
    ParkingModel *programParking;

    View::VstShellSelect *shellSelect;

public slots:
    void OnViewConfigClosed();
    void LoadDefaultFiles();
    void SetProgramsFont(const QFont &f);
    void LoadProgramsFont();

private slots:
    void on_actionLoad_Setup_triggered();
    void on_actionRestore_default_layout_triggered();
    void on_actionSave_Setup_As_triggered();
    void on_actionSave_Project_As_triggered();
    void on_actionAbout_triggered();
    void on_actionNew_Setup_triggered();
    void on_actionSave_Setup_triggered();
    void on_actionNew_triggered();
    void on_actionLoad_triggered();
    void on_actionSave_triggered();
    void openRecentSetup();
    void openRecentProject();
    void UpdateColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color);
    void UpdateKeyBinding();

    void on_solverView_clicked(const QModelIndex &index);
    void on_actionAppearance_toggled(bool arg1);
    void on_actionCable_toggled(bool arg1);
    void on_actionValue_toggled(bool arg1);
    void on_actionKeyBinding_triggered();
    void on_actionHide_all_editors_triggered(bool checked);
    void on_actionAutoShowGui_triggered(bool checked);
    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
    void on_actionHost_panel_toggled(bool arg1);
    void on_actionGroup_panel_toggled(bool arg1);
    void on_actionProgram_panel_toggled(bool arg1);
    void on_actionProject_panel_toggled(bool arg1);
    void on_dockSolver_visibilityChanged(bool visible);
};

#endif // MAINWINDOW_H
