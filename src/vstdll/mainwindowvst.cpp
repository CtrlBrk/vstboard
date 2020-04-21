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
#include "mainwindowvst.h"
#include "views/configdialogvst.h"
#include "vstboardcontroller.h"
#include "ids.h"

MainWindowVst::MainWindowVst(Vst::EditController *controller,Settings *settings, QWidget *parent) :
    MainWindow(settings,0,parent),
    controller(controller)
{

}

MainWindowVst::~MainWindowVst()
{

}

void MainWindowVst::Init()
{
    MainWindow::Init();

#ifdef QT_NO_DEBUG
    if(qtTranslator.load("qt_" + QLocale::system().name(), ":/translations/"))
        qApp->installTranslator(&qtTranslator);
    if(commonTranslator.load("common_" + QLocale::system().name(), ":/translations/"))
        qApp->installTranslator(&commonTranslator);
    if(myappTranslator.load("vstboard_" + QLocale::system().name(), ":/translations/"))
        qApp->installTranslator(&myappTranslator);
#endif

    ui->actionRefresh_Audio_devices->setDisabled(true);
    ui->actionRefresh_Midi_devices->setDisabled(true);
    setWindowTitle("VstBoard");
    BuildListTools();
    setAcceptDrops(false);

    readSettings();
}

void MainWindowVst::SendMsg(const MsgObject &msg)
{
    if(msg.prop.contains(MsgObject::Type)) {
        if(msg.prop[MsgObject::Type].toInt() == ObjType::VstAutomation) {
            int32 tag = msg.prop[MsgObject::Id].toInt();
            controller->beginEdit(tag);
            controller->performEdit(tag, msg.prop[MsgObject::Value].toFloat());
            controller->endEdit(tag);
//            return;
        }
    }

    OPtr<Vst::IMessage> message = controller->allocateMessage();
    if (message)
    {
        message->setMessageID("msg");

        QByteArray br;
        QDataStream str(&br, QIODevice::WriteOnly);
        str << msg;
        message->getAttributes ()->setBinary ("data", br.data(), br.size());
        controller->sendMessage(message);
    }
}

void MainWindowVst::closeEvent(QCloseEvent *event)
{
//    writeSettings();
    event->accept();
}

void MainWindowVst::BuildListTools()
{
    MainWindow::BuildListTools();

    QStandardItem *parentItem=0;
    QStandardItem *item=0;
    ObjectInfo info;

    parentItem = listToolsModel->invisibleRootItem();

    //audio devices (vst in/out)
    //================================

    //vst audio in
    item = new QStandardItem(tr("Vst audio input"));
    ObjectInfo infoai;
    infoai.nodeType = NodeType::object;
    infoai.objType = ObjType::AudioInterfaceIn;
    infoai.name = "Vst audio In";
    item->setData(QVariant::fromValue(infoai), UserRoles::objInfo);
    parentItem->appendRow(item);

    //vst audio out
    item = new QStandardItem(tr("Vst audio output"));
    ObjectInfo infoao;
    infoao.nodeType = NodeType::object;
    infoao.objType = ObjType::AudioInterfaceOut;
    infoao.name = "Vst audio Out";
    item->setData(QVariant::fromValue(infoao), UserRoles::objInfo);
    parentItem->appendRow(item);

    //midi devices (vst in/out)
    //================================

    //vst midi in
    item = new QStandardItem(tr("Vst midi input"));
    ObjectInfo infomi;
    infomi.nodeType = NodeType::object;
    infomi.objType = ObjType::MidiInterface;
    infomi.outputs = 1;
    infomi.name = "Vst midi In";
    item->setData(QVariant::fromValue(infomi), UserRoles::objInfo);
    parentItem->appendRow(item);

    //vst midi out
    item = new QStandardItem(tr("Vst midi output"));
    ObjectInfo infomo;
    infomo.nodeType = NodeType::object;
    infomo.objType = ObjType::MidiInterface;
    infomo.inputs = 1;
    infomo.name = "Vst midi Out";
    item->setData(QVariant::fromValue(infomo), UserRoles::objInfo);
    parentItem->appendRow(item);

    //vst automation
    item = new QStandardItem(tr("Vst Automation"));
    info.nodeType = NodeType::object;
    info.objType = ObjType::VstAutomation;
    item->setData(QVariant::fromValue(info), UserRoles::objInfo);
    parentItem->appendRow(item);
}

void MainWindowVst::readSettings()
{
    MainWindow::readSettings();
    ui->dockAudioDevices->hide();
    ui->dockMidiDevices->hide();
}

void MainWindowVst::resetSettings()
{
    MainWindow::resetSettings();
    ui->dockAudioDevices->hide();
    ui->dockMidiDevices->hide();
}

void MainWindowVst::on_actionConfig_triggered()
{
    ConfigDialogVst conf(settings,0,this);
    conf.exec();
}
