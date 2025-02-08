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

#include <fstream>
#include "objectfactory.h"
#include "midisender.h"
#include "miditoautomation.h"
#include "hostcontroller.h"
#include "container.h"
#include "bridge.h"
#include "mainhost.h"
#include "buffer.h"

#ifdef VST24SDK
    #include "vstplugin.h"
    #include "../vst/cvsthost.h"
#endif
#ifdef VSTSDK
    #include "vst3plugin.h"
#endif

#ifdef SCRIPTENGINE
    #include "script.h"
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
    QScriptValue scriptObj = myHost->scriptEngine.newQObject(this);
    myHost->scriptEngine.globalObject().setProperty("ObjectFactory", scriptObj);
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

QSharedPointer<Object> ObjectFactory::GetObjectFromId(int id) {
    if(id==0) {
#ifdef DEBUG_OBJECTS
LOG(QString("obj %1 not defined").arg(id));
#endif
        return QSharedPointer<Object>();
    }

    if(!listObjects.contains(id)) {
#ifdef DEBUG_OBJECTS
LOG(QString("obj %1 not defined").arg(id));
#endif
        return QSharedPointer<Object>();
    }

    QSharedPointer<Object> o = listObjects.value(id);
    return listObjects.value(id);
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

WORD ObjectFactory::Check32bits(const ObjectInfo &info) {
    IMAGE_DOS_HEADER dosHead;
    IMAGE_FILE_HEADER fileHead;

    std::ifstream fsrteam(info.filename.toStdString(), std::ios_base::in | std::ios_base::binary);
    fsrteam.read(PCHAR(&dosHead), sizeof(dosHead));
    fsrteam.seekg(dosHead.e_lfanew + 4, std::ios_base::beg);
    fsrteam.read(PCHAR(&fileHead), sizeof(fileHead));
    fsrteam.close();
    return fileHead.Machine;
}

QSharedPointer<Object> ObjectFactory::NewObject(const ObjectInfo &info, int containerId)
{
    int objId = -1;

    //check 32bits dll
    if(info.nodeType == NodeType::object && info.objType==ObjType::VstPlugin) {
        if(Check32bits(info)==IMAGE_FILE_MACHINE_I386) {
            ObjectInfo i = info;
            i.objType = ObjType::VstPlugin32;
            return NewObject(i,containerId);
        }
    }

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
                #ifdef VST24SDK
                    case ObjType::VstPlugin:
                        obj = new VstPlugin(myHost,objId, info);
                        break;
                #endif
                    case ObjType::Vst3Plugin:
                        obj = new Vst3Plugin(myHost,objId, info);
                        break;
                    case ObjType::VstPlugin32:
                        // obj = new VstPlugin32(myHost,objId, info);
                        obj = new Object(myHost, objId, info);
                        obj->SetErrorMessage("32bit not implemented");
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

//    uint16 maxId = -1;
//    if(objId>maxId) {
//        LOG("objId overflow " << objId << maxId)
//        return sharedObj;
//    }
#ifdef DEBUG_OBJECTS
    LOG("add obj " << objId << obj->objectName())
#endif
    listObjects.insert(objId,sharedObj.toWeakRef());
    obj->SetSleep(false);

    if(info.forcedObjId) {
        obj->ResetSavedIndex(info.forcedObjId);
    } else {
//        cptListObjects++;
    }

    return sharedObj;

}
