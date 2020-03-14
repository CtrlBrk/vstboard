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

#include "parkingmodel.h"

ParkingModel::ParkingModel(MsgController *msgCtrl, int objId, QObject *parent) :
    QStandardItemModel(parent),
    MsgHandler(msgCtrl, objId)
{
}

void ParkingModel::ReceiveMsg(const MsgObject &msg)
{
    if(msg.prop.contains(MsgObject::Add)) {
//        LOG("park"<<msg.prop[MsgObject::Add].toInt())
        ObjectInfo info = msg.prop[MsgObject::ObjInfo].value<ObjectInfo>();
        QStandardItem *item = new QStandardItem( info.name );
        item->setText( QString("%1(%2)").arg(info.name).arg(msg.prop[MsgObject::Add].toInt()) );
        item->setData(msg.prop[MsgObject::Add].toInt());
        item->setData(QVariant::fromValue(info),UserRoles::objInfo);
        invisibleRootItem()->appendRow(item);
        return;
    }

    if(msg.prop.contains(MsgObject::Remove)) {
//        LOG("unpark"<<msg.prop[MsgObject::Remove].toInt())
        int nb=rowCount();
        for(int i=0; i<nb; ++i) {
            if(invisibleRootItem()->child(i)->data().toInt()==msg.prop[MsgObject::Remove].toInt()) {
                invisibleRootItem()->removeRow(i);
                return;
            }
        }
        return;
    }

    if(msg.prop.contains(MsgObject::Clear)) {
        clear();
        return;
    }
}
