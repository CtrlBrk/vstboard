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
#include "listaudiointerfacesmodel.h"
#include "connectables/objectinfo.h"

ListAudioInterfacesModel::ListAudioInterfacesModel(MsgController *msgCtrl, int objId, QObject *parent) :
    QStandardItemModel(parent),
    MsgHandler(msgCtrl, objId)
{
    QStringList headerLabels;
    headerLabels << "Name";
    headerLabels << "In";
    headerLabels << "Out";
    headerLabels << "InUse";
    setHorizontalHeaderLabels(  headerLabels );

//    QBrush my_brush;
//    QColor red(Qt::red);
//    setHeaderData(1,Qt::Horizontal,my_brush,Qt::BackgroundRole);
//    setHeaderData(1,Qt::Horizontal,my_brush,Qt::ForegroundRole);
}

Qt::ItemFlags ListAudioInterfacesModel::flags ( const QModelIndex & index ) const
{
    if(!index.parent().isValid()  || !index.data(UserRoles::objInfo).isValid())
        return Qt::ItemIsEnabled;

    return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;

}

QMimeData  * ListAudioInterfacesModel::mimeData ( const QModelIndexList  & indexes ) const
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
        data->setData("application/x-audiointerface",b);
        return data;
    }
    return 0;
}

void ListAudioInterfacesModel::ReceiveMsg(const MsgObject &msg)
{
    if(msg.prop.contains(MsgObject::State)) {
        for(int i=0; i<rowCount(); i++) {
            if(index(i,0).data(UserRoles::value).toInt() == msg.prop[MsgObject::Group].toInt()) {
                QModelIndex apiIdx = index(i,0);
                for(int j=0; j<rowCount(apiIdx); j++) {
                    if(index(j,0,apiIdx).data(UserRoles::value).toInt() == msg.prop[MsgObject::Id].toInt()) {
                        if(msg.prop[MsgObject::State].toBool())
                            itemFromIndex(index(j,3,apiIdx))->setCheckState(Qt::Checked);
                        else
                            itemFromIndex(index(j,3,apiIdx))->setCheckState(Qt::Unchecked);

                        return;
                    }
                }
            }
        }
        return;
    }

    if(msg.prop.contains(MsgObject::Update)) {
        invisibleRootItem()->removeRows(0, rowCount());
//        expandedIndex.clear();

        foreach(const MsgObject &msgApi, msg.children) {
            QStandardItem *apiItem = new QStandardItem(msgApi.prop[MsgObject::Name].toString());
            apiItem->setData( msgApi.objIndex, UserRoles::value );
            apiItem->setData( "api", UserRoles::type );
            apiItem->setDragEnabled(false);
            apiItem->setSelectable(false);
//            if(msgApi.prop.contains("expand"))
//                expandedIndex << apiItem->index();

            foreach(const MsgObject &msgDevice, msgApi.children) {
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

                QStandardItem *inUseItem = new QStandardItem( false );
                inUseItem->setEditable(false);
                if(msgDevice.prop[MsgObject::State].toBool())
                    inUseItem->setCheckState(Qt::Checked);
                else
                    inUseItem->setCheckState(Qt::Unchecked);
                listItems << inUseItem;

                apiItem->appendRow( listItems );

            }

            invisibleRootItem()->appendRow(apiItem);
        }
    }
}

void ListAudioInterfacesModel::Update()
{
    MSGOBJ();
    msg.prop[MsgObject::GetUpdate]=1;
    msgCtrl->SendMsg(msg);
}

void ListAudioInterfacesModel::Rescan()
{
    MSGOBJ();
    msg.prop[MsgObject::Rescan]=1;
    msgCtrl->SendMsg(msg);
}

void ListAudioInterfacesModel::ApiDisabled(const QModelIndex &api)
{
    MSGOBJ();
    msg.prop[MsgObject::State]=api.data(UserRoles::value).toInt();
    msgCtrl->SendMsg(msg);
}

void ListAudioInterfacesModel::ResetApis()
{
    MSGOBJ();
    msg.prop[MsgObject::State]=-1;
    msgCtrl->SendMsg(msg);
}

void ListAudioInterfacesModel::ConfigDevice(const QModelIndex &dev)
{
    MSGOBJ();
    ObjectInfo info = dev.data(UserRoles::objInfo).value<ObjectInfo>();
    msg.prop[MsgObject::Setup]=QVariant::fromValue(info);
    msgCtrl->SendMsg(msg);
}
