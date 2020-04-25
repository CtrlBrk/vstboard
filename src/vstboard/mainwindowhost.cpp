/**************************************************************************
#    Copyright 2010-2020 Raphaël François
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
#include "mainwindowhost.h"
#include "mainhosthost.h"
#include "views/configdialoghost.h"
#include "views/splash.h"
#include "audiodevices.h"
#include "mididevices.h"
#include "programmanager.h"

MainWindowHost::MainWindowHost(Settings *settings, MainHostHost * myHost, QWidget *parent) :
    MainWindow(settings,myHost,parent),
	listAudioDevModel(0),
	listMidiDevModel(0)
{
    myHost->mainWindow=this;

    setWindowTitle("VstBoard");
}

void MainWindowHost::Kill()
{
    delete this;
}

void MainWindowHost::Init()
{
    MainWindow::Init();

    //audio devices
    listAudioDevModel = new ListAudioInterfacesModel(this,FixedObjId::audioDevices,this);
    ui->treeAudioInterfaces->setModel(listAudioDevModel);
    ui->treeAudioInterfaces->header()->setSectionResizeMode(0,QHeaderView::Stretch);
    ui->treeAudioInterfaces->header()->setSectionResizeMode(1,QHeaderView::Fixed);
    ui->treeAudioInterfaces->header()->setSectionResizeMode(2,QHeaderView::Fixed);
    ui->treeAudioInterfaces->header()->setSectionResizeMode(3,QHeaderView::Fixed);
    ui->treeAudioInterfaces->header()->resizeSection(1,30);
    ui->treeAudioInterfaces->header()->resizeSection(2,30);
    ui->treeAudioInterfaces->header()->resizeSection(3,40);

    connect(ui->treeAudioInterfaces, SIGNAL(UpdateList()),
            this, SLOT(UpdateAudioDevices()));
    listAudioDevModel->Update();

    //midi devices
    listMidiDevModel = new ListMidiInterfacesModel(this,FixedObjId::midiDevices,this);
    ui->treeMidiInterfaces->setModel(listMidiDevModel);
    ui->treeMidiInterfaces->header()->setSectionResizeMode(0,QHeaderView::Stretch);
    ui->treeMidiInterfaces->header()->setSectionResizeMode(1,QHeaderView::Fixed);
    ui->treeMidiInterfaces->header()->setSectionResizeMode(2,QHeaderView::Fixed);
    ui->treeMidiInterfaces->header()->setSectionResizeMode(3,QHeaderView::Fixed);
    ui->treeMidiInterfaces->header()->resizeSection(1,30);
    ui->treeMidiInterfaces->header()->resizeSection(2,30);
    ui->treeMidiInterfaces->header()->resizeSection(3,40);

    QAction *updateMidiList = new QAction( QIcon::fromTheme("view-refresh") , tr("Refresh list"), ui->treeMidiInterfaces);
    updateMidiList->setShortcut(Qt::Key_F5);
    updateMidiList->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->treeMidiInterfaces->addAction( updateMidiList );

    connect( updateMidiList, SIGNAL(triggered()),
             this, SLOT(UpdateMidiDevices()));

    listMidiDevModel->Update();

    BuildListTools();

    myHost->SetSampleRate( ConfigDialog::defaultSampleRate(settings) );
}

void MainWindowHost::closeEvent(QCloseEvent *event)
{
    if(!myHost->programManager->userWantsToUnloadSetup()) {
        event->ignore();
        return;
    }

    if(!myHost->programManager->userWantsToUnloadProject()) {
        event->ignore();
        return;
    }

    writeSettings();
    event->accept();

    qApp->exit(0);

}

void MainWindowHost::readSettings()
{
    MainWindow::readSettings();

    QSettings settings;
    if( !settings.value("splashHide",false).toBool() ) {
        Splash *spl = new Splash(this);
        spl->show();
    }

    ui->menuView->addAction(ui->dockMidiDevices->toggleViewAction());
    ui->mainToolBar->addAction(ui->dockMidiDevices->toggleViewAction());
    ui->dockMidiDevices->toggleViewAction()->setShortcut(viewConfig->keyBinding->GetMainShortcut(KeyBind::midiDevices));

    ui->menuView->addAction(ui->dockAudioDevices->toggleViewAction());
    ui->mainToolBar->addAction(ui->dockAudioDevices->toggleViewAction());
    ui->dockAudioDevices->toggleViewAction()->setShortcut(viewConfig->keyBinding->GetMainShortcut(KeyBind::audioDevices));
}

void MainWindowHost::resetSettings()
{
    MainWindow::resetSettings();

//    QList<QDockWidget*>listDocksVisible;
//    listDocksVisible << ui->dockMidiDevices;
//    listDocksVisible << ui->dockAudioDevices;
//    foreach(QDockWidget *dock, listDocksVisible) {
//        dock->setFloating(false);
//        dock->setVisible(true);
//    }

    addDockWidget(Qt::LeftDockWidgetArea,  ui->dockMidiDevices);
//    addDockWidget(Qt::LeftDockWidgetArea,  ui->dockAudioDevices);

    tabifyDockWidget(ui->dockTools,ui->dockMidiDevices);
    tabifyDockWidget(ui->dockMidiDevices,ui->dockAudioDevices);
}

void MainWindowHost::UpdateAudioDevices()
{
    listAudioDevModel->Rescan();
}

void MainWindowHost::UpdateMidiDevices()
{
    listMidiDevModel->Rescan();
}

void MainWindowHost::on_actionRefresh_Audio_devices_triggered()
{
    UpdateAudioDevices();
}

void MainWindowHost::on_actionRefresh_Midi_devices_triggered()
{
    UpdateMidiDevices();
}

void MainWindowHost::on_actionConfig_triggered()
{
    ConfigDialogHost conf(settings,myHost,this);
    conf.exec();
}

void MainWindowHost::SendMsg(const MsgObject &msg)
{
    emit SendMsgSignal(msg);
//    myHost->ReceiveMsg(msg);
}

void MainWindowHost::ReceiveMsgSignal(const MsgObject &msg)
{
    ReceiveMsg(msg);
}
