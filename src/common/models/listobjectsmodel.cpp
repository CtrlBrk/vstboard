/**************************************************************************
#    Copyright 2010-2025 Raphaël François
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
#include "listobjectsmodel.h"

ListObjectsModel::ListObjectsModel(MsgController *msgCtrl, int objId, QObject *parent) :
    QStandardItemModel(parent),
    MsgHandler(msgCtrl, objId)
{

}

QStandardItem* ListObjectsModel::AddItem(const MsgObject &msg, QStandardItem *parent)
{
    int newId = msg.prop[MsgObject::Id].toInt();

    if(listItems.contains(newId)) {
        return listItems[ newId ];
    }
    QStandardItem *i = new QStandardItem( msg.prop[MsgObject::Name].toString() );
    listItems[ newId ] = i;
    parent->appendRow(i);
    return i;
}

void ListObjectsModel::ReceiveMsg(const MsgObject &msg)
{
    if(msg.prop.contains(MsgObject::Add)) {
        QStandardItem *parent = invisibleRootItem();
        if(listItems.contains(msg.objIndex)) {
            parent = listItems[msg.objIndex];
        }
        QStandardItem *newObj = AddItem(msg, parent);

        foreach(const MsgObject &cMsg, msg.children) {
            AddItem(cMsg, newObj);
        }
    }
    if(msg.prop.contains(MsgObject::Remove)) {

    }
    if(msg.prop.contains(MsgObject::Clear)) {
        clear();
    }
}
