/**************************************************************************
#    Copyright 2010-2011 Raphaël François
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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "globals.h"
#include "projectfile/projectfile.h"
#include "views/configdialog.h"
#include "views/aboutdialog.h"
#include "connectables/objectinfo.h"
#include "views/viewconfigdialog.h"
//#include "models/programsmodel.h"
#include "views/keybindingdialog.h"
#include "msgobject.h"

MainWindow::MainWindow(Settings *settings, MainHost * myHost, QWidget *parent) :
    QMainWindow(parent),
    mySceneView(0),
    listToolsModel(0),
    listVstPluginsModel(0),
    listVstBanksModel(0),
    ui(new Ui::MainWindow),
    myHost(myHost),
    viewConfig( new View::ViewConfig(settings,this)),
    viewConfigDlg(0),
    actUndo(0),
    actRedo(0),
    settings(settings),
    progModel(new GroupsProgramsModel(this,this)),
    groupParking(0),
    programParking(0)
{
}

void MainWindow::Init()
{
    //myHost->mainWindow=this;
//    connect(myHost,SIGNAL(programParkingModelChanged(QStandardItemModel*)),
//            this,SLOT(programParkingModelChanges(QStandardItemModel*)));
//    connect(myHost,SIGNAL(groupParkingModelChanged(QStandardItemModel*)),
//            this,SLOT(groupParkingModelChanges(QStandardItemModel*)));

    ui->setupUi(this);
    ui->statusBar->hide();
    ui->dockUndo->hide();

    connect(ui->mainToolBar, SIGNAL(visibilityChanged(bool)),
            ui->actionTool_bar, SLOT(setChecked(bool)));

    //programs
//    ui->Programs->SetModel( myHost->programsModel );
    ui->Programs->SetModel(progModel);
    progModel->Update();

    SetupBrowsersModels( ConfigDialog::defaultVstPath(settings), ConfigDialog::defaultBankPath(settings));

    mySceneView = new View::SceneView(this, this, FixedObjId::mainContainer, ui->hostView, ui->projectView, ui->programView, ui->groupView, this);
    mySceneView->SetParkings(ui->programParkList, ui->groupParkList);
//    mySceneView->setModel(myHost->GetModel());

    groupParking = new ParkingModel(this, FixedObjId::groupParking);
    ui->groupParkList->setModel(groupParking);
    programParking = new ParkingModel(this, FixedObjId::programParking);
    ui->programParkList->setModel(programParking);

//    ui->solverView->setModel(myHost->GetRendererModel());
//    connect(myHost->GetRendererModel(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
//            ui->solverView, SLOT(resizeColumnsToContents()));
//    connect(myHost->GetRendererModel(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
//            ui->solverView, SLOT(resizeRowsToContents()));

//    ui->treeHostModel->setModel(myHost->GetModel());

    setPalette( viewConfig->GetPaletteFromColorGroup( ColorGroups::Window, palette() ));
    connect( viewConfig, SIGNAL(ColorChanged(ColorGroups::Enum,Colors::Enum,QColor)),
             progModel, SLOT(UpdateColor(ColorGroups::Enum,Colors::Enum,QColor)) );
    connect( viewConfig, SIGNAL(ColorChanged(ColorGroups::Enum,Colors::Enum,QColor)),
             this, SLOT(UpdateColor(ColorGroups::Enum,Colors::Enum,QColor)));

    connect(viewConfig->keyBinding, SIGNAL(BindingChanged()),
            this, SLOT(UpdateKeyBinding()));

//    actUndo = myHost->undoStack.createUndoAction(ui->mainToolBar);
//    actUndo->setIcon(QIcon(":/img16x16/undo.png"));
//    actUndo->setShortcutContext(Qt::ApplicationShortcut);
//    ui->mainToolBar->addAction( actUndo );

//    actRedo = myHost->undoStack.createRedoAction(ui->mainToolBar);
//    actRedo->setIcon(QIcon(":/img16x16/redo.png"));
//    actRedo->setShortcutContext(Qt::ApplicationShortcut);
//    ui->mainToolBar->addAction( actRedo );

//    ui->listUndo->setStack(&myHost->undoStack);

    UpdateKeyBinding();
}

void MainWindow::ReceiveMsg(const MsgObject &msg)
{
    if(msg.objIndex==FixedObjId::mainWindow) {
        if(msg.prop.contains(MsgObject::Setup) && msg.prop.contains(MsgObject::Project)) {
            QFileInfo setup( msg.prop[MsgObject::Setup].toString() );

            if(setup.fileName().isEmpty()) {
                ui->actionSave_Setup_As->setEnabled( false );
            } else {
                ui->actionSave_Setup_As->setEnabled( true );
                settings->SetSetting("lastSetupDir",setup.absolutePath());
                ConfigDialog::AddRecentSetupFile(setup.absoluteFilePath(),settings);
            }

            QFileInfo project( msg.prop[MsgObject::Project].toString() );
            if(project.fileName().isEmpty()) {
                ui->actionSave_Project_As->setEnabled( false );
            } else {
                ui->actionSave_Project_As->setEnabled( true );
                settings->SetSetting("lastProjectDir",project.absolutePath());
                ConfigDialog::AddRecentProjectFile(project.absoluteFilePath(),settings);
            }

            setWindowTitle(QString("VstBoard %1:%2").arg( setup.baseName() ).arg( project.baseName() ));
            updateRecentFileActions();
        }
        return;
    }

    if(!listObj.contains(msg.objIndex)) {
        LOG("obj not found"<<msg.objIndex<<msg.prop)
        return;
    }

    if(msg.prop.contains(MsgObject::Remove)) {
        if(!listObj.contains(msg.prop[MsgObject::Remove].toInt())) {
            LOG("obj not found"<<msg.prop)
            return;
        }
        delete listObj[msg.prop[MsgObject::Remove].toInt()];
        return;
    }

    listObj[msg.objIndex]->ReceiveMsg(msg);

    //some objects take care of the children
    if(msg.objIndex==FixedObjId::programsManager ||
        msg.objIndex==FixedObjId::audioDevices ||
        msg.objIndex==FixedObjId::midiDevices) {
        return;
    }

    foreach(const MsgObject &cMsg, msg.children) {
        ReceiveMsg(cMsg);
    }


//    if(msg.objIndex == FixedObjId::audioDevices) {
//        if(listObj.contains(msg.objIndex)) {
//            listObj[msg.objIndex]->ReceiveMsg(msg);
//        }
//        return;
//    }

//    if(msg.objIndex == FixedObjId::midiDevices) {
//        if(listObj.contains(msg.objIndex)) {
//            listObj[msg.objIndex]->ReceiveMsg(msg);
//        }
//        return;
//    }

//    if(progModel && msg.objIndex==FixedObjId::programsManager) {
//        progModel->ReceiveMsg(msg);
//        return;
//    }

//    if(groupParking && msg.objIndex==FixedObjId::groupParking) {
//        groupParking->ReceiveMsg(msg);
//        return;
//    }

//    if(programParking && msg.objIndex==FixedObjId::programParking) {
//        programParking->ReceiveMsg(msg);
//        return;
//    }

//    if(!mySceneView)
//        return;
////    LOG("addobj"<<msg.objIndex<<msg.parentIndex<<msg.prop)
//    mySceneView->ReceiveMsg(msg);
//    if(!msg.children.isEmpty())
//        ProcessMsg(msg.children);
}

//void MainWindow::ReceiveMsg(const QString &type, const QVariant &data)
//{
//    ProcessMsg( data.value< ListMsgObj >() );
//}

//void MainWindow::ProcessMsg(const ListMsgObj &lstMsg)
//{
//    foreach(MsgObject msg, lstMsg) {
//        ReceiveMsg(msg);
//    }
//}

void MainWindow::showEvent(QShowEvent *event)
{
//    LOG("window show event")

    MsgObject msg;
    msg.prop[MsgObject::Update]=1;
    msg.objIndex=FixedObjId::hostContainer;
    SendMsg(msg);
    msg.objIndex=FixedObjId::projectContainer;
    SendMsg(msg);
    msg.objIndex=FixedObjId::groupContainer;
    SendMsg(msg);
    msg.objIndex=FixedObjId::programContainer;
    SendMsg(msg);
}

void MainWindow::SetupBrowsersModels(const QString &vstPath, const QString &browserPath)
{
#if !defined(__GNUC__)
    //vst plugins browser
    //sse2 crash with gcc ?

    listVstPluginsModel = new QFileSystemModel(this);
    listVstPluginsModel->setReadOnly(true);
    listVstPluginsModel->setResolveSymlinks(true);
    QStringList fileFilter;
    fileFilter << "*.dll";
    listVstPluginsModel->setNameFilters(fileFilter);
    listVstPluginsModel->setNameFilterDisables(false);
    listVstPluginsModel->setRootPath(vstPath);
    ui->VstBrowser->setModel(listVstPluginsModel);

    //bank file browser
    listVstBanksModel = new QFileSystemModel(this);
    listVstBanksModel->setReadOnly(false);
    listVstBanksModel->setResolveSymlinks(true);
    //    QStringList bankFilter;
    //    bankFilter << "*.fxb";
    //    bankFilter << "*.fxp";
    //    listVstBanksModel->setNameFilters(bankFilter);
    //    listVstBanksModel->setNameFilterDisables(false);
    listVstBanksModel->setRootPath(browserPath);
    ui->BankBrowser->setModel(listVstBanksModel);
#endif
}

MainWindow::~MainWindow()
{
    if(mySceneView) {
        delete mySceneView;
        mySceneView=0;
    }
    if(ui)
        delete ui;
    if(progModel)
        delete progModel;
    if(groupParking)
        delete groupParking;
    if(programParking)
        delete programParking;
}

void MainWindow::UpdateColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color)
{
    if(groupId!=ColorGroups::Window)
        return;

    QPalette::ColorRole role = viewConfig->GetPaletteRoleFromColor(colorId);

    QPalette pal=palette();
    pal.setColor(role, color);
    setPalette(pal);
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::BuildListTools()
{
    QStringList headerLabels;
    headerLabels << "Name";

    QStandardItem *parentItem=0;
    QStandardItem *item=0;
    ObjectInfo info;

    listToolsModel = new ListToolsModel(this,FixedObjId::tools,this);
    listToolsModel->setHorizontalHeaderLabels(headerLabels);
    parentItem = listToolsModel->invisibleRootItem();

#ifdef SCRIPTENGINE
    //script
    item = new QStandardItem(tr("Script"));
    info.nodeType = NodeType::object;
    info.objType = ObjType::Script;
    item->setData(QVariant::fromValue(info), UserRoles::objInfo);
    parentItem->appendRow(item);
#endif

    //midi parameters
    item = new QStandardItem(tr("Midi to parameter"));
    info.nodeType = NodeType::object;
    info.objType = ObjType::MidiToAutomation;
    item->setData(QVariant::fromValue(info), UserRoles::objInfo);
    parentItem->appendRow(item);

    //midi sender
    item = new QStandardItem(tr("Midi sender"));
    info.nodeType = NodeType::object;
    info.objType = ObjType::MidiSender;
    item->setData(QVariant::fromValue(info),UserRoles::objInfo);
    parentItem->appendRow(item);

    //host controller
    item = new QStandardItem(tr("Host Controller"));
    info.nodeType = NodeType::object;
    info.objType = ObjType::HostController;
    item->setData(QVariant::fromValue(info),UserRoles::objInfo);
    parentItem->appendRow(item);

    //delay buffer
    item = new QStandardItem(tr("Delay Buffer"));
    info.nodeType = NodeType::object;
    info.objType = ObjType::Buffer;
    info.inputs = 2000;//myHost->GetBufferSize()*2;
    item->setData(QVariant::fromValue(info),UserRoles::objInfo);
    parentItem->appendRow(item);

    ui->treeTools->setModel(listToolsModel);
    ui->treeTools->header()->setResizeMode(0,QHeaderView::Stretch);
}

void MainWindow::on_actionLoad_triggered()
{
    MsgObject msg(FixedObjId::mainHost);
    msg.prop[MsgObject::Project]=MsgObject::Load;
    SendMsg(msg);
}

void MainWindow::on_actionNew_triggered()
{
    MsgObject msg(FixedObjId::mainHost);
    msg.prop[MsgObject::Project]=MsgObject::Clear;
    SendMsg(msg);
}

void MainWindow::on_actionSave_triggered()
{
    MsgObject msg(FixedObjId::mainHost);
    msg.prop[MsgObject::Project]=MsgObject::Save;
    SendMsg(msg);
}

void MainWindow::on_actionSave_Project_As_triggered()
{
    MsgObject msg(FixedObjId::mainHost);
    msg.prop[MsgObject::Project]=MsgObject::SaveAs;
    SendMsg(msg);
}

void MainWindow::on_actionLoad_Setup_triggered()
{
    MsgObject msg(FixedObjId::mainHost);
    msg.prop[MsgObject::Setup]=MsgObject::Load;
    SendMsg(msg);
}

void MainWindow::on_actionNew_Setup_triggered()
{
    MsgObject msg(FixedObjId::mainHost);
    msg.prop[MsgObject::Setup]=MsgObject::Clear;
    SendMsg(msg);
}

void MainWindow::on_actionSave_Setup_triggered()
{
    MsgObject msg(FixedObjId::mainHost);
    msg.prop[MsgObject::Setup]=MsgObject::Save;
    SendMsg(msg);
}

void MainWindow::on_actionSave_Setup_As_triggered()
{
    MsgObject msg(FixedObjId::mainHost);
    msg.prop[MsgObject::Setup]=MsgObject::SaveAs;
    SendMsg(msg);
}

void MainWindow::UpdateKeyBinding()
{
    ui->actionLoad->setShortcut(viewConfig->keyBinding->GetMainShortcut(KeyBind::openProject));
    ui->actionSave->setShortcut(viewConfig->keyBinding->GetMainShortcut(KeyBind::saveProject));
    ui->actionSave_Project_As->setShortcut(viewConfig->keyBinding->GetMainShortcut(KeyBind::saveProjectAs));
    ui->actionNew->setShortcut(viewConfig->keyBinding->GetMainShortcut(KeyBind::newProject));
    ui->actionLoad_Setup->setShortcut(viewConfig->keyBinding->GetMainShortcut(KeyBind::openSetup));
    ui->actionSave_Setup->setShortcut(viewConfig->keyBinding->GetMainShortcut(KeyBind::saveSetup));
    ui->actionSave_Setup_As->setShortcut(viewConfig->keyBinding->GetMainShortcut(KeyBind::saveSetupAs));
    ui->actionNew_Setup->setShortcut(viewConfig->keyBinding->GetMainShortcut(KeyBind::newSetup));
    ui->actionHost_panel->setShortcut(viewConfig->keyBinding->GetMainShortcut(KeyBind::hostPanel));
    ui->actionProject_panel->setShortcut(viewConfig->keyBinding->GetMainShortcut(KeyBind::projectPanel));
    ui->actionProgram_panel->setShortcut(viewConfig->keyBinding->GetMainShortcut(KeyBind::programPanel));
    ui->actionGroup_panel->setShortcut(viewConfig->keyBinding->GetMainShortcut(KeyBind::groupPanel));
    ui->actionRestore_default_layout->setShortcut(viewConfig->keyBinding->GetMainShortcut(KeyBind::defaultLayout));
    ui->actionTool_bar->setShortcut(viewConfig->keyBinding->GetMainShortcut(KeyBind::toolBar));
    ui->actionConfig->setShortcut(viewConfig->keyBinding->GetMainShortcut(KeyBind::configuration));
    ui->actionAppearance->setShortcut(viewConfig->keyBinding->GetMainShortcut(KeyBind::appearence));
    ui->dockTools->toggleViewAction()->setShortcut(viewConfig->keyBinding->GetMainShortcut(KeyBind::tools));
    ui->dockVstBrowser->toggleViewAction()->setShortcut(viewConfig->keyBinding->GetMainShortcut(KeyBind::vstPlugins));
    ui->dockBankBrowser->toggleViewAction()->setShortcut(viewConfig->keyBinding->GetMainShortcut(KeyBind::browser));
    ui->dockPrograms->toggleViewAction()->setShortcut(viewConfig->keyBinding->GetMainShortcut(KeyBind::programs));
//    ui->dockUndo->toggleViewAction()->setShortcut(viewConfig->keyBinding->GetMainShortcut(KeyBind::undoHistory));
    ui->dockSolver->toggleViewAction()->setShortcut(viewConfig->keyBinding->GetMainShortcut(KeyBind::solverModel));
    ui->dockHostModel->toggleViewAction()->setShortcut(viewConfig->keyBinding->GetMainShortcut(KeyBind::hostModel));
    ui->actionHide_all_editors->setShortcut(viewConfig->keyBinding->GetMainShortcut(KeyBind::hideAllEditors));
    ui->actionAutoShowGui->setShortcut(viewConfig->keyBinding->GetMainShortcut(KeyBind::autoOpenEditors));
    ui->actionUndo->setShortcut(viewConfig->keyBinding->GetMainShortcut(KeyBind::undo));
    ui->actionRedo->setShortcut(viewConfig->keyBinding->GetMainShortcut(KeyBind::redo));
    ui->actionRefresh_Audio_devices->setShortcut(viewConfig->keyBinding->GetMainShortcut(KeyBind::refreashAudioDevices));
    ui->actionRefresh_Midi_devices->setShortcut(viewConfig->keyBinding->GetMainShortcut(KeyBind::refreashMidiDevices));
}

void MainWindow::writeSettings()
{
    settings->SetSetting("MainWindow/geometry", saveGeometry());
    settings->SetSetting("MainWindow/state", saveState());
//    settings->SetSetting("MainWindow/statusBar", ui->statusBar->isVisible());
    settings->SetSetting("MainWindow/splitPan", ui->splitterPanels->saveState());
    settings->SetSetting("MainWindow/splitProg", ui->splitterProg->saveState());
    settings->SetSetting("MainWindow/splitGroup", ui->splitterGroup->saveState());

    settings->SetSetting("MainWindow/planelHost", ui->actionHost_panel->isChecked());
    settings->SetSetting("MainWindow/planelProject", ui->actionProject_panel->isChecked());
    settings->SetSetting("MainWindow/planelProgram", ui->actionProgram_panel->isChecked());
    settings->SetSetting("MainWindow/planelGroup", ui->actionGroup_panel->isChecked());

    settings->SetSetting("lastVstPath", ui->VstBrowser->path());
    settings->SetSetting("lastBankPath", ui->BankBrowser->path());
    ui->Programs->writeSettings(settings);
    //settings.sync();
}

void MainWindow::readSettings()
{
    ui->menuView->addAction(ui->dockTools->toggleViewAction());
    ui->mainToolBar->addAction(ui->dockTools->toggleViewAction());

    ui->menuView->addAction(ui->dockVstBrowser->toggleViewAction());
    ui->mainToolBar->addAction(ui->dockVstBrowser->toggleViewAction());

    ui->menuView->addAction(ui->dockBankBrowser->toggleViewAction());
    ui->mainToolBar->addAction(ui->dockBankBrowser->toggleViewAction());

    ui->menuView->addAction(ui->dockPrograms->toggleViewAction());
    ui->mainToolBar->addAction(ui->dockPrograms->toggleViewAction());

//    ui->menuView->addAction(ui->dockUndo->toggleViewAction());
//    ui->mainToolBar->addAction(ui->dockUndo->toggleViewAction());

    ui->menuView->addAction(ui->dockSolver->toggleViewAction());

    ui->menuView->addAction(ui->dockHostModel->toggleViewAction());

    //recent setups
    for(int i=0; i<NB_RECENT_FILES; i++) {
        QAction *act = new QAction(this);
        act->setVisible(false);
        connect(act, SIGNAL(triggered()),
                this, SLOT(openRecentSetup()));

        ui->menuRecent_Setups->addAction(act);

        listRecentSetups << act;
    }

    //recent projects
    for(int i=0; i<NB_RECENT_FILES; i++) {
        QAction *act = new QAction(this);
        act->setVisible(false);
        connect(act, SIGNAL(triggered()),
                this, SLOT(openRecentProject()));

        ui->menuRecent_Projects->addAction(act);

        listRecentProjects << act;
    }

    //window state

    if(settings->SettingDefined("MainWindow/geometry")) {
        restoreGeometry(settings->GetSetting("MainWindow/geometry").toByteArray());
        restoreState(settings->GetSetting("MainWindow/state").toByteArray());
//        bool statusb = myHost->GetSetting("MainWindow/statusBar",false).toBool();
//        ui->actionStatus_bar->setChecked( statusb );
//        ui->statusBar->setVisible(statusb);
    } else {
        resetSettings();
    }

    ui->dockUndo->setVisible(false);

    ui->splitterProg->setStretchFactor(0,100);
    ui->splitterGroup->setStretchFactor(0,100);

    if(settings->SettingDefined("MainWindow/splitPan"))
        ui->splitterPanels->restoreState(settings->GetSetting("MainWindow/splitPan").toByteArray());
    if(settings->SettingDefined("MainWindow/splitProg"))
        ui->splitterProg->restoreState(settings->GetSetting("MainWindow/splitProg").toByteArray());
    if(settings->SettingDefined("MainWindow/splitGroup"))
        ui->splitterGroup->restoreState(settings->GetSetting("MainWindow/splitGroup").toByteArray());

    ui->actionHost_panel->setChecked( settings->GetSetting("MainWindow/planelHost",true).toBool() );
    ui->actionProject_panel->setChecked( settings->GetSetting("MainWindow/planelProject",false).toBool() );
    ui->actionProgram_panel->setChecked( settings->GetSetting("MainWindow/planelProgram",true).toBool() );
    ui->actionGroup_panel->setChecked( settings->GetSetting("MainWindow/planelGroup",true).toBool() );

    ui->Programs->readSettings(settings);

    viewConfig->LoadFromRegistry();

    LoadProgramsFont();
}

void MainWindow::LoadDefaultFiles()
{
    //load default files
    QString file = ConfigDialog::defaultSetupFile(settings);
    if(!file.isEmpty())
        myHost->LoadSetupFile( file );

    file = ConfigDialog::defaultProjectFile(settings);
    if(!file.isEmpty())
        myHost->LoadProjectFile( file );

    updateRecentFileActions();
}

void MainWindow::currentFileChanged()
{
    QFileInfo set(myHost->currentSetupFile);
    QFileInfo proj(myHost->currentProjectFile);
    setWindowTitle(QString("VstBoard %1:%2").arg( set.baseName() ).arg( proj.baseName() ));

    ui->actionSave_Setup_As->setEnabled(!myHost->currentSetupFile.isEmpty());
    ui->actionSave_Project_As->setEnabled(!myHost->currentProjectFile.isEmpty());

    updateRecentFileActions();
}

void MainWindow::resetSettings()
{
    QList<QDockWidget*>listDocksVisible;
    listDocksVisible << ui->dockTools;
    listDocksVisible << ui->dockVstBrowser;
    listDocksVisible << ui->dockBankBrowser;
    listDocksVisible << ui->dockPrograms;
//    listDocksVisible << ui->dockUndo;
    foreach(QDockWidget *dock, listDocksVisible) {
        dock->setFloating(false);
        dock->setVisible(true);
    }

    QList<QDockWidget*>listDocksHidden;
    listDocksHidden << ui->dockSolver;
    listDocksHidden << ui->dockHostModel;
    foreach(QDockWidget *dock, listDocksHidden) {
        dock->setFloating(false);
        dock->setVisible(false);
    }

    ui->Programs->resetSettings();

    addDockWidget(Qt::LeftDockWidgetArea,  ui->dockTools);
    addDockWidget(Qt::LeftDockWidgetArea,  ui->dockVstBrowser);
    addDockWidget(Qt::LeftDockWidgetArea,  ui->dockBankBrowser);

    addDockWidget(Qt::RightDockWidgetArea,  ui->dockPrograms);
//    addDockWidget(Qt::RightDockWidgetArea,  ui->dockUndo);
    addDockWidget(Qt::RightDockWidgetArea,  ui->dockSolver);
    addDockWidget(Qt::RightDockWidgetArea,  ui->dockHostModel);

    tabifyDockWidget(ui->dockHostModel,ui->dockSolver);

    ui->actionHost_panel->setChecked(true);
    ui->actionProject_panel->setChecked(false);
    ui->actionProgram_panel->setChecked(true);
    ui->actionGroup_panel->setChecked(true);

    ui->actionTool_bar->setChecked(true);
//    ui->actionStatus_bar->setChecked(false);
    ui->statusBar->setVisible(false);
    ui->dockUndo->setVisible(false);

    int h = ui->splitterPanels->height()/4;
    QList<int>heights;
    heights << h << h << h << h;
    ui->splitterPanels->setSizes(heights);

    QList<int> szProg;
    szProg << 1000 << 100;
    ui->splitterProg->setSizes(szProg);

    QList<int> szGrp;
    szGrp << 1000 << 100;
    ui->splitterGroup->setSizes(szGrp);
}

void MainWindow::on_actionAbout_triggered()
{
    AboutDialog about;
    about.exec();
}

void MainWindow::updateRecentFileActions()
{
    {
        QStringList files = settings->GetSetting("recentSetupFiles").toStringList();

        int numRecentFiles = qMin(files.size(), (int)NB_RECENT_FILES);

        for (int i = 0; i < numRecentFiles; ++i) {
            listRecentSetups[i]->setText(QFileInfo(files[i]).fileName());
            listRecentSetups[i]->setData(files[i]);
            listRecentSetups[i]->setVisible(true);
        }
        for (int j = numRecentFiles; j < NB_RECENT_FILES; ++j)
            listRecentSetups[j]->setVisible(false);
    }

    {
        QStringList files = settings->GetSetting("recentProjectFiles").toStringList();

        int numRecentFiles = qMin(files.size(), (int)NB_RECENT_FILES);

        for (int i = 0; i < numRecentFiles; ++i) {
            listRecentProjects[i]->setText(QFileInfo(files[i]).fileName());
            listRecentProjects[i]->setData(files[i]);
            listRecentProjects[i]->setVisible(true);
        }
        for (int j = numRecentFiles; j < NB_RECENT_FILES; ++j)
            listRecentProjects[j]->setVisible(false);
    }
}

void MainWindow::openRecentSetup()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if(!action)
        return;

    myHost->LoadSetupFile( action->data().toString() );
}

void MainWindow::openRecentProject()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if(!action)
        return;

    myHost->LoadProjectFile( action->data().toString() );
}

//void MainWindow::programParkingModelChanges(QStandardItemModel *model)
//{
//    ui->programParkList->setModel(model);
//}

//void MainWindow::groupParkingModelChanges(QStandardItemModel *model)
//{
//    ui->groupParkList->setModel(model);
//}

void MainWindow::on_actionRestore_default_layout_triggered()
{
    resetSettings();
}

void MainWindow::on_solverView_clicked(const QModelIndex &index)
{
    myHost->OptimizeRenderer();
}

void MainWindow::on_actionAppearance_toggled(bool arg1)
{
    if(arg1) {
        if(viewConfigDlg)
            return;
        viewConfigDlg = new View::ViewConfigDialog(myHost,this);
        connect(viewConfigDlg, SIGNAL(destroyed()),
                this, SLOT(OnViewConfigClosed()));
        viewConfigDlg->setAttribute( Qt::WA_DeleteOnClose, true );
        viewConfigDlg->show();
    } else {
        if(!viewConfigDlg)
            return;
        viewConfigDlg->close();
    }
}

void MainWindow::OnViewConfigClosed()
{
    viewConfigDlg=0;
    ui->actionAppearance->setChecked(false);
}

void MainWindow::SetProgramsFont(const QFont &f)
{
    ui->Programs->setFont(f);
}

void MainWindow::LoadProgramsFont()
{
    QFont f(ui->Programs->font());

    QString fam( settings->GetSetting("fontProgFamily","Default").toString() );
    if(fam!="Default")
        f.setFamily( fam );
    else
        f.setFamily( ui->dockPrograms->font().family() );

    int s = settings->GetSetting("fontProgSize",0).toInt();
    if(s>0)
        f.setPointSize( s );
    else
        f.setPointSize( ui->dockPrograms->font().pointSize() );

    f.setStretch( settings->GetSetting("fontProgStretch",100).toInt() );
    f.setBold( settings->GetSetting("fontProgBold",false).toBool() );
    f.setItalic( settings->GetSetting("fontProgItalic",false).toBool() );
    ui->Programs->setFont(f);
}

void MainWindow::on_actionCable_toggled(bool arg1)
{
    ui->actionValue->setChecked(!arg1);
    if(arg1)
        viewConfig->keyBinding->SetCurrentMode("Cable");
}

void MainWindow::on_actionValue_toggled(bool arg1)
{
    ui->actionCable->setChecked(!arg1);
    if(arg1)
        viewConfig->keyBinding->SetCurrentMode("Value");
}

void MainWindow::on_actionKeyBinding_triggered()
{
    KeyBindingDialog bind(viewConfig->keyBinding,this);
    bind.exec();
}

void MainWindow::on_actionHide_all_editors_triggered(bool checked)
{
    if(checked) {
        listClosedEditors.clear();
        foreach(QSharedPointer<Connectables::Object>obj, myHost->objFactory->GetListObjects()) {
            if(obj->ToggleEditor(false))
                listClosedEditors << obj;
        }
    } else {
        foreach(QSharedPointer<Connectables::Object>obj, listClosedEditors) {
            if(obj || obj->parkingId!=FixedObjId::ND)
                listClosedEditors.removeAll(obj);
            else
                obj->ToggleEditor(true);
        }
        listClosedEditors.clear();
    }
}

void MainWindow::on_actionAutoShowGui_triggered(bool checked)
{
    viewConfig->AutoOpenGui=checked;
}

void MainWindow::on_actionUndo_triggered()
{
    MsgObject msg(FixedObjId::mainHost);
    msg.prop[MsgObject::Undo]=1;
    SendMsg(msg);
}

void MainWindow::on_actionRedo_triggered()
{
    MsgObject msg(FixedObjId::mainHost);
    msg.prop[MsgObject::Redo]=1;
    SendMsg(msg);
}
