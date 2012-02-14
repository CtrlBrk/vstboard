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
