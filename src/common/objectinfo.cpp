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

#include "objectinfo.h"
#include "mainhost.h"
#include "events.h"

void MetaTransporter::ValueChanged( const MetaInfo & senderInfo, int type, const QVariant &value)
{
    if(!autoUpdate)
        return;

    Events::valChanged *e = new Events::valChanged(MetaInfo(senderInfo), (MetaInfos::Enum)type, value);
    PostEvent(e);
}

void MetaTransporter::PostEvent( QEvent * event) {
//#ifndef QT_NO_DEBUG
//    if(listeners.isEmpty()) {
//        LOG("no listener")
//    }
//#endif
    foreach(QObject *obj, listeners) {
        qApp->postEvent(obj,event);
    }
}

quint32 MetaInfo::nextId=50;
QMap<quint32,quint32> MetaInfo::savedIds;
DMutex MetaInfo::mutexListInfos(QMutex::Recursive);

MetaInfo::MetaInfo() :
    objType(MetaTypes::ND),
    objId(0),
    objName(""),
    parentId(0),
    containerId(0),
    parentObjectId(0),
    transporter(0)
{
    SET_MUTEX_NAME(mutexListInfos,"mutexListInfos");
}

MetaInfo::MetaInfo(const MetaInfo &c)
{
    *this=c;
    SET_MUTEX_NAME(mutexListInfos,"mutexListInfos");
}

MetaInfo::MetaInfo(const MetaTypes::Enum type) :
    objType(type),
    objId(0),
    objName(""),
    parentId(0),
    containerId(0),
    parentObjectId(0),
    transporter(0)
{
    SET_MUTEX_NAME(mutexListInfos,"mutexListInfos");
}

MetaInfo::MetaInfo(const QByteArray &b)
{
    QDataStream stream(b);
    fromStream(stream);
    SET_MUTEX_NAME(mutexListInfos,"mutexListInfos");
}

ObjectInfo::ObjectInfo() :
    MetaInfo(),
    parentInfo(0),
    containerInfo(0)
{
}

ObjectInfo::ObjectInfo(const ObjectInfo &c) :
    MetaInfo(c)
{
    parentInfo=c.parentInfo;
    containerInfo=c.containerInfo;
}

ObjectInfo::ObjectInfo(const MetaInfo &c, MetaTransporter *transporter) :
    MetaInfo(c),
    parentInfo(0),
    containerInfo(0)
{
    SetTransporter(transporter);
}

ObjectInfo::~ObjectInfo()
{
    if(parentInfo) {
        parentInfo->childrenInfo.removeAll(this);
        parentInfo=0;
    }
    foreach(ObjectInfo *info, childrenInfo) {
        info->SetParent(0);
    }
}

void ObjectInfo::AddToView()
{
    if(!ContainerId())
        return;

    if(Meta(MetaInfos::Hidden).toBool())
        return;

    Events::sendObj *event = new Events::sendObj(info(), Events::typeNewObj);
    if(transporter) {
        transporter->PostEvent(event);
    }  else {
        LOG("transporter not set"<<toStringFull());
    }

    foreach(ObjectInfo *info, childrenInfo) {
        info->AddToView();
    }
}

void ObjectInfo::AddToParkView()
{
    Events::sendObj *event = new Events::sendObj(info(), Events::typeParkObj);
    if(transporter)
        transporter->PostEvent(event);
    else {
        LOG("transporter not set"<<toStringFull());
    }
}

void ObjectInfo::RemoveFromView()
{
    Events::delObj *event = new Events::delObj(ObjId());
    if(transporter)
        transporter->PostEvent(event);
//    else {
//        LOG("transporter not set"<<toStringFull());
//    }
}

void ObjectInfo::UpdateView()
{
    if(!ContainerId())
        return;

    if(Meta(MetaInfos::Hidden).toBool())
        return;

    Events::sendObj *event = new Events::sendObj(info(), Events::typeUpdateObj);
    if(transporter) {
        transporter->PostEvent(event);
    } else {
        LOG("transporter not set"<<toStringFull());
    }
}

void ObjectInfo::SetContainer(ObjectInfo *container) {

    if(Meta(MetaInfos::Bridge).toBool())
        container = container->ContainerInfo();

    containerInfo = container;

    if(container)
        SetContainerId(container->ObjId());
    else
        SetContainerId(0);

    if(Type()!=MetaTypes::container) {
        foreach(ObjectInfo *info, childrenInfo) {
            info->SetContainer(container);
        }
    }
}

void ObjectInfo::SetParent(ObjectInfo *parent)
{
    if(parentInfo) {
        parentInfo->childrenInfo.removeAll(this);
    }

    parentInfo = parent;

    if(parentInfo) {
        SetParentId(parent->ObjId());
        parentInfo->childrenInfo << this;

        SetTransporter(parent->Transporter());

        if(parentInfo->Type()==MetaTypes::object || parentInfo->Type()==MetaTypes::container || parentInfo->Type()==MetaTypes::bridge)
            SetParentObjectId(ParentId());
        else
            SetParentObjectId(ParentInfo()->ParentObjectId());
    } else {
        SetParentId(0);
        SetTransporter(0);
        SetParentObjectId(0);
        SetContainerId(0);
        containerInfo=0;
    }

    foreach(ObjectInfo *info, childrenInfo) {
        info->SetParent(this);
    }
}


bool MetaInfo::CanConnectTo(const MetaInfo &c) const
{
    //don't connect object to itself
//    if(objId == c.objId)
//        return false;

    if(Type()!=MetaTypes::pin)
        return false;
    if(c.Type()!=MetaTypes::pin)
        return false;

    //must be in the same container
    if(ContainerId() != c.ContainerId())
        return false;

    //must be opposite directions
    if(Meta(MetaInfos::Direction) == c.Meta(MetaInfos::Direction))
        return false;

    //must be the same type (audio/midi/automation) or a bridge pin
    if(Meta(MetaInfos::Media)!=MediaTypes::Bridge
        && c.Meta(MetaInfos::Media)!=MediaTypes::Bridge
        && Meta(MetaInfos::Media) != c.Meta(MetaInfos::Media))
        return false;

    return true;
}

QString MetaInfo::toString() const
{
    return QString("type:%1 id:%2 name:%3 parent:%4 container:%5 obj:%6")
            .arg(objType)
            .arg(objId)
            .arg(objName)
            .arg(parentId)
            .arg(containerId)
            .arg(parentObjectId);
}

QString MetaInfo::toStringFull() const
{
    QString str(toString());
    QMap<MetaInfos::Enum,QVariant>::iterator i = listInfos.begin();
    while(i != listInfos.end()) {
        str.append( QString(" %1:%2").arg(i.key()).arg(i.value().toString()));
        ++i;
    }
    return str;
}

QDataStream & MetaInfo::toStream(QDataStream& stream) const
{
    stream << (quint8)objType;
    stream << objId;
    stream << objName;
    stream << parentId;
    stream << containerId;
    stream << parentObjectId;

    mutexListInfos.lock();
    stream << (quint16)listInfos.size();
    QMap<MetaInfos::Enum,QVariant>::iterator i = listInfos.begin();
    while(i != listInfos.end()) {
        stream << (quint16)i.key();
        stream << i.value();
        ++i;
    }
    mutexListInfos.unlock();

//    stream << (quint16)childrenInfo.size();
//    foreach(ObjectInfo* o, childrenInfo) {
//        stream << o->ObjId();
//    }

//    stream << (quint8)metaType;
//    stream << (quint8)objType;
//    stream << id;
//    stream << name;
//    stream << filename;
//    stream << inputs;
//    stream << outputs;
//    stream << duplicateNamesCounter;
//    stream << apiName;
//    stream << api;
//    stream << objId;
    return stream;
}

QDataStream & MetaInfo::fromStream(QDataStream& stream)
{
    quint8 type;
    stream >> type;
    objType=(MetaTypes::Enum)type;
    stream >> objId;
    stream >> objName;
    stream >> parentId;
    stream >> containerId;
    stream >> parentObjectId;

    quint16 nb;
    stream >> nb;

    mutexListInfos.lock();
    for(int i=0; i<nb; i++) {
        quint16 id;
        QVariant val;
        stream >> id;
        stream >> val;
        listInfos.insert((MetaInfos::Enum)id,val);
    }
    mutexListInfos.unlock();

//    stream >> nb;
//    for(int i=0; i<nb; i++) {
//        quint32 id;
//        stream >> id;
//    }

//    stream >> (quint8&)metaType;
//    stream >> (quint8&)objType;
//    stream >> id;
//    stream >> name;
//    stream >> filename;
//    stream >> inputs;
//    stream >> outputs;
//    stream >> duplicateNamesCounter;
//    stream >> apiName;
//    stream >> api;
//    stream >> objId;
    return stream;
}

QDataStream & operator<< (QDataStream& stream, const MetaInfo& objInfo)
{
   return objInfo.toStream(stream);
}

QDataStream & operator>> (QDataStream& stream, MetaInfo& objInfo)
{
    return objInfo.fromStream(stream);
}

QDataStream & ObjectContainerAttribs::toStream (QDataStream& out) const
{
    out << position;
    out << editorVisible;
    out << editorPosition;
    out << editorSize;
    out << editorHScroll;
    out << editorVScroll;

    return out;
}

QDataStream & ObjectContainerAttribs::fromStream (QDataStream& in)
{
    in >> position;
    in >> editorVisible;
    in >> editorPosition;
    in >> editorSize;
    in >> editorHScroll;
    in >> editorVScroll;
    return in;
}

QDataStream & operator<< (QDataStream & out, const ObjectContainerAttribs& value)
{
    return value.toStream(out);
}

QDataStream & operator>> (QDataStream & in, ObjectContainerAttribs& value)
{
    return value.fromStream(in);
}
