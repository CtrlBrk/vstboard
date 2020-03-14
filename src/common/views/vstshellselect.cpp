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

#include "vstshellselect.h"
#include "ui_vstshellselect.h"
using namespace View;

VstShellSelect::VstShellSelect(MsgController *msgCtrl, int objId, QWidget *parent) :
    QDialog(parent),
    MsgHandler(msgCtrl,objId),
    ui(new Ui::VstShellSelect),
    senderObjId(0)
{
    setWindowFlags(Qt::Tool);
    ui->setupUi(this);
//    setAttribute( Qt::WA_DeleteOnClose );
}

VstShellSelect::~VstShellSelect()
{
    delete ui;
}

void VstShellSelect::ReceiveMsg(const MsgObject &msg)
{
    senderObjId = msg.prop[MsgObject::Id].toInt();
    info = msg.prop[MsgObject::ObjInfo].value<ObjectInfo>();

    foreach(const MsgObject &plug, msg.children) {
        QListWidgetItem *item = new QListWidgetItem( plug.prop[MsgObject::Name].toString());
        item->setData(Qt::UserRole, plug.prop[MsgObject::Id].toUInt());
        ui->listPlugins->addItem(item);
    }

    show();
    window()->setFocus();
    ui->listPlugins->setFocus();
}

void View::VstShellSelect::accept()
{
    info.id = ui->listPlugins->currentIndex().data(Qt::UserRole).toUInt();
    ui->listPlugins->clear();
    QDialog::accept();

    MsgObject msg(senderObjId);
    msg.prop[MsgObject::ObjInfo] = QVariant::fromValue(info);
    msgCtrl->SendMsg(msg);

    senderObjId=0;
}

void View::VstShellSelect::reject()
{
    ui->listPlugins->clear();
    QDialog::reject();

//    MsgObject msg(senderObjId);
//    msg.prop[MsgObject::Remove]=RemoveType::RemoveWithCables;
//    msgCtrl->SendMsg(msg);
    MsgObject msg(senderObjId);
    msg.prop[MsgObject::ObjInfo] = QVariant::fromValue(info);
    msgCtrl->SendMsg(msg);

    senderObjId=0;
}

