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


#include "objectfactory.h"
#include "midisender.h"
#include "miditoautomation.h"
#include "hostcontroller.h"
#include "container.h"
#include "bridge.h"
#include "mainhost.h"
#include "buffer.h"

#ifdef VSTSDK
    #include "vstplugin.h"
    #include "../vst/cvsthost.h"
    #include "vst3plugin.h"
#endif

/*!
  \namespace Connectables
  \brief classes used by the engine
*/
using namespace Connectables;

ObjectFactory::ObjectFactory(MainHost *myHost) :
    QObject(myHost),
    myHost(myHost)
{
    setObjectName("ObjectFactory");

#ifdef SCRIPTENGINE
    QScriptValue scriptObj = myHost->scriptEngine->newQObject(this);
    myHost->scriptEngine->globalObject().setProperty("ObjectFactory", scriptObj);
#endif
}

ObjectFactory::~ObjectFactory()
{
    Close();
}

void ObjectFactory::Close()
{
    listObjects.clear();
}

void ObjectFactory::ResetAllSavedId()
{
    hashObjects::iterator i = listObjects.begin();
    while(i != listObjects.end())
    {
        QSharedPointer<Object> objPtr = i.value().toStrongRef();
        if(!objPtr)
        {
            i=listObjects.erase(i);
        } else {
            //don't reset static ids
            if(i.key()>=FixedObjId::dynamicIdStart)
            {
                objPtr->ResetSavedIndex();
            }

            ++i;
        }
    }
}

int ObjectFactory::IdFromSavedId(int savedId)
{
    hashObjects::const_iterator i = listObjects.constBegin();
    while(i != listObjects.constEnd()) {
        QSharedPointer<Object> objPtr = i.value().toStrongRef();
        if(objPtr && objPtr->GetSavedIndex()==savedId) {
            return i.key();
        }
        ++i;
    }
#ifdef DEBUG_OBJECTS
    LOG("id not found"<<savedId);
#endif
    return -1;
}

Pin *ObjectFactory::GetPin(const ConnectionInfo &pinInfo)
{
    if(!listObjects.contains(pinInfo.objId)) {
#ifdef DEBUG_OBJECTS
        LOG("obj not found"<<pinInfo.objId);
#endif
        return 0;
    }

    QSharedPointer<Object> objPtr = listObjects.value(pinInfo.objId).toStrongRef();
    if(objPtr)
        return objPtr->GetPin(pinInfo);

    return 0;
}

QSharedPointer<Object> ObjectFactory::GetObj(const QModelIndex & index)
{
    //the object is not created, do it
    if(!index.data(UserRoles::value).isValid()) {
        return NewObject( index.data(UserRoles::objInfo).value<ObjectInfo>() );
    }

    //or return the existing object
    return GetObjectFromId(index.data(UserRoles::value).toInt());
}

QSharedPointer<Object> ObjectFactory::NewObject(const ObjectInfo &info, int containerId)
{
    int objId = -1;

    if(info.forcedObjId!=0) {
        objId = info.forcedObjId;
        if(listObjects.contains(objId)) {
#ifdef DEBUG_OBJECTS
            LOG("forcedId already exists"<<objId);
#endif
        }
    } else {
        objId = myHost->GetNewObjId();
    }

    Object *obj=0;

    if(containerId) {
//        MsgObject msg(containerId);
        _MSGOBJ(msg,containerId);
        msg.prop[MsgObject::Id]=objId;
        msg.prop[MsgObject::Add]=NodeType::tempObj;
        msg.prop[MsgObject::Name]=info.name;
        myHost->SendMsg(msg);
        qApp->processEvents();
    }

    obj=CreateOtherObjects(info, objId);

    if(!obj) {
        switch(info.nodeType) {

            case NodeType::container :
                obj = new Container(myHost,objId, info);
                break;

            case NodeType::bridge :
                obj = new Bridge(myHost,objId, info);
                break;

            case NodeType::object :

                switch(info.objType) {
#ifdef SCRIPTENGINE
                    case ObjType::Script:
                        obj = new Script(myHost,objId,info);
                        break;
#endif
                    case ObjType::MidiSender:
                        obj = new MidiSender(myHost,objId);
                        break;

                    case ObjType::MidiToAutomation:
                        obj = new MidiToAutomation(myHost,objId);
                        break;

                    case ObjType::HostController:
                        obj = new HostController(myHost,objId);
                        break;

            #ifdef VSTSDK
                    case ObjType::VstPlugin:
                        obj = new VstPlugin(myHost,objId, info);
                        break;
                    case ObjType::Vst3Plugin:
                        obj = new Vst3Plugin(myHost,objId, info);
                        break;
            #endif

                    case ObjType::dummy :
                        obj = new Object(myHost, objId, info);
                        obj->SetErrorMessage("Dummy object");
                        break;

                    case ObjType::Buffer :
                        obj = new Buffer(myHost, objId, info);
                        break;

                    default:
#ifdef DEBUG_OBJECTS
                        LOG("unknown object type"<<info.objType);
#endif
                        return QSharedPointer<Object>();
                }
                break;


            default :
#ifdef DEBUG_OBJECTS
                LOG("unknown nodeType"<<info.nodeType);
#endif
                return QSharedPointer<Object>();
        }
    }

#ifdef DEBUG_OBJECTS
//    LOG("Loading Obj:"<<objId<<" : "<<info.name);
#endif
    QSharedPointer<Object> sharedObj(obj);

    if(!obj->Open()) {
//        listObjects.remove(objId);
//        sharedObj.clear();
//        return QSharedPointer<Object>();
        return sharedObj;
    }

    uint16 maxId = -1;
    if(objId>maxId) {
        LOG("objId overflow " << objId << maxId)
        return sharedObj;
    }
    LOG("add obj " << objId << obj->objectName())
    listObjects.insert(objId,sharedObj.toWeakRef());
    obj->SetSleep(false);

    if(info.forcedObjId) {
        obj->ResetSavedIndex(info.forcedObjId);
    } else {
//        cptListObjects++;
    }

    return sharedObj;

}
