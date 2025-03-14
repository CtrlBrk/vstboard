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
#include "listmidiinterfacesmodel.h"
#include "connectables/objectinfo.h"
#ifdef RTAUDIO
#include "../mididevices_rm.h"
#endif
#ifdef PORTAUDIO
#include "../mididevices_pm.h"
#endif


ListMidiInterfacesModel::ListMidiInterfacesModel(MsgController *msgCtrl, int objId, QObject *parent):
    QStandardItemModel(parent),
    MsgHandler(msgCtrl, objId)
{
    QStringList headerLabels;
    headerLabels << "Name";
    headerLabels << "In";
    headerLabels << "Out";
    headerLabels << "InUse";
    setHorizontalHeaderLabels(  headerLabels );
}

Qt::ItemFlags ListMidiInterfacesModel::flags ( const QModelIndex & index ) const
{
    if(!index.data(UserRoles::objInfo).isValid())
        return Qt::ItemIsEnabled;

    return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;

}

QMimeData  * ListMidiInterfacesModel::mimeData ( const QModelIndexList  & indexes ) const
{
    QByteArray b;
    QDataStream stream(&b,QIODevice::WriteOnly);

    foreach(QModelIndex idx, indexes) {
        if(!idx.isValid() || !idx.data(UserRoles::objInfo).isValid())
            continue;
        stream << idx.data(UserRoles::objInfo).value<ObjectInfo>();
    }

    if(b.length()>0) {
        QMimeData  *data = new QMimeData();
        data->setData("application/x-midiinterface",b);
        return data;
    }
    return 0;
}

void ListMidiInterfacesModel::ReceiveMsg(const MsgObject &msg)
{
    if(msg.prop.contains(MsgObject::State)) {
        ObjectInfo objInfo = msg.prop[MsgObject::ObjInfo].value<ObjectInfo>();
        for(int i=0; i<rowCount(); i++) {
            ObjectInfo rowInfo = index(i,0).data(UserRoles::objInfo).value<ObjectInfo>();
            if(MidiDevices::GetUid(rowInfo) == msg.prop[MsgObject::Id].toInt()) {
//            if(index(i,0).data(UserRoles::value).toInt() == msg.prop[MsgObject::Id].toInt()) {
                if(msg.prop[MsgObject::State].toBool())
                    item(i,3)->setCheckState(Qt::Checked);
                else
                    item(i,3)->setCheckState(Qt::Unchecked);

                return;
            }
        }
        return;
    }

    if(msg.prop.contains(MsgObject::Update)) {
        invisibleRootItem()->removeRows(0, rowCount());

        foreach(const MsgObject &msgDevice, msg.children) {
            QList<QStandardItem *> listItems;
            QStandardItem *devItem = new QStandardItem(msgDevice.prop[MsgObject::Name].toString());
            devItem->setEditable(false);
            ObjectInfo obj = msgDevice.prop[MsgObject::ObjInfo].value<ObjectInfo>();
            devItem->setData(QVariant::fromValue(obj), UserRoles::objInfo);
            devItem->setData(obj.id, UserRoles::value);
            devItem->setDragEnabled(true);
            listItems << devItem;

            QStandardItem *inputItem = new QStandardItem( QString::number(obj.inputs));
            inputItem->setEditable(false);
            listItems << inputItem;

            QStandardItem *outputItem = new QStandardItem( QString::number(obj.outputs));
            outputItem->setEditable(false);
            listItems << outputItem;

            QStandardItem *inUseItem = new QStandardItem(false);
            inUseItem->setEditable(false);
            if(msgDevice.prop[MsgObject::State].toBool())
                inUseItem->setCheckState(Qt::Checked);
            else
                inUseItem->setCheckState(Qt::Unchecked);

            listItems << inUseItem;

            invisibleRootItem()->appendRow(listItems);
        }
    }
}


void ListMidiInterfacesModel::Update()
{
    MSGOBJ();
    msg.prop[MsgObject::GetUpdate]=1;
    msgCtrl->SendMsg(msg);
}

void ListMidiInterfacesModel::Rescan()
{
    MSGOBJ();
    msg.prop[MsgObject::Rescan]=1;
    msgCtrl->SendMsg(msg);
}
