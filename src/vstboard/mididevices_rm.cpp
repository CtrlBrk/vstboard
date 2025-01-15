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
#include "mididevices_rm.h"
#include "connectables/objectinfo.h"
#include "mainhosthost.h"

#include "RtMidi.h"

//QList< QSharedPointer<Connectables::MidiDevice> >MidiDevices::listOpenedMidiDevices;
//QMutex MidiDevices::mutexListMidi;

MidiDevices::MidiDevices(MainHostHost *myHost, MsgController *msgCtrl, int objId) :
    QObject(myHost),
    MsgHandler(msgCtrl, objId),
    pmOpened(false),
    myHost(myHost)
{
    OpenDevices();
}

MidiDevices::~MidiDevices()
{
//    if(Pt_Started())
//        Pt_Stop();

//    if(pmOpened)
//        Pm_Terminate();

}

void MidiDevices::OpenDevices()
{
    mutexListMidi.lock();
    foreach(Connectables::MidiDevice* md, listOpenedMidiDevices) {
        md->SetSleep(true);
        md->CloseStream();
    }
    mutexListMidi.unlock();

    pmOpened=true;

    if(MsgEnabled())
        BuildModel();

    foreach(QSharedPointer<Connectables::Object>obj, myHost->objFactory->GetListObjects()) {
        if(!obj)
            continue;

        if(obj->info().objType == ObjType::MidiInterface) {
            obj->Open();
        }
    }

    mutexListMidi.lock();
    foreach(Connectables::MidiDevice* md, listOpenedMidiDevices) {
        if(md->OpenStream())
            md->SetSleep(false);
    }
    mutexListMidi.unlock();

    myHost->UpdateView();
}

void MidiDevices::OpenDevice(Connectables::MidiDevice* objPtr)
{
    mutexListMidi.lock();
    listOpenedMidiDevices << objPtr;
    mutexListMidi.unlock();

    //distinguish in & out by building a uid :
    listOpenedDevices << GetUid( objPtr->info() );

    MSGOBJ();
    msg.prop[MsgObject::State]=true;
    msg.prop[MsgObject::Id]=GetUid(objPtr->info());
    msgCtrl->SendMsg(msg);
}

void MidiDevices::CloseDevice(Connectables::MidiDevice* objPtr)
{
    listOpenedDevices.removeAll( GetUid(objPtr->info()) );

    MSGOBJ();
    msg.prop[MsgObject::State]=false;
    msg.prop[MsgObject::Id]=GetUid(objPtr->info());
    msgCtrl->SendMsg(msg);

    mutexListMidi.lock();
    listOpenedMidiDevices.removeAll( objPtr );
    mutexListMidi.unlock();
}

bool MidiDevices::GetDeviceInfo(ObjectInfo &obj,MsgObject &msg)
{
    try {
        RtMidiIn midiIn( (RtMidi::Api)obj.api);
        RtMidiOut midiOut( (RtMidi::Api)obj.api);

        RtMidi *dev = 0;
        if(obj.inputs) {

            dev = &midiIn;
        } else {

            dev = &midiOut;
        }
        unsigned int nPorts = dev->getPortCount();
        std::string portName;
        for ( unsigned int j=0; j<nPorts; j++ ) {
            obj.nodeType = NodeType::object;
            obj.objType = ObjType::MidiInterface;
            obj.id = j;
            obj.name = QString::fromStdString( dev->getPortName(j) );

            MsgObject msgDevice;
            msgDevice.prop[MsgObject::Name]=obj.name;
            msgDevice.prop[MsgObject::ObjInfo]=QVariant::fromValue(obj);
            msgDevice.prop[MsgObject::State]=(bool)listOpenedDevices.contains( GetUid(obj) );
            msg.children << msgDevice;
        }

    }
    catch ( RtMidiError &error ) {
      error.printMessage();
      return false;
    }

    return true;
}

void MidiDevices::BuildModel()
{
    MSGOBJ();
    msg.prop[MsgObject::Update]=1;

    std::vector<RtMidi::Api> apis;
    RtMidi::getCompiledApi( apis );

    for (size_t i = 0; i < apis.size() ; ++i) {
        _MSGOBJ(msgApi,apis[i]);
        QString apiName = QString::fromStdString( RtMidi::getApiName(apis[i]) );
        msgApi.prop[MsgObject::Name] = apiName;

        ObjectInfo obj;
        obj.api = apis[i];
        obj.apiName = apiName;

        obj.inputs = 1;
        obj.outputs = 0;
        GetDeviceInfo( obj, msg );

        obj.inputs = 0;
        obj.outputs = 1;
        GetDeviceInfo( obj, msg );
    }
    msgCtrl->SendMsg(msg);
}

int MidiDevices::GetDevIdByName(RtMidi::Api apiId, const std::string &devName, bool input)
{
    try {
        if(input) {
            RtMidiIn rm( apiId );
            for(uint i=0; i<rm.getPortCount(); i++) {
                if(devName == rm.getPortName(i)) {
                    return i;
                }
            }
        } else {
            RtMidiOut rm( apiId );
            for(uint i=0; i<rm.getPortCount(); i++) {
                std::string name = rm.getPortName(i);
                if(devName == name) {
                    return i;
                }
            }
        }
    }
    catch ( RtMidiError &error ) {
      error.printMessage();
    }
    return -1;
}

void MidiDevices::ReceiveMsg(const MsgObject &msg)
{
    SetMsgEnabled(true);

    if(msg.prop.contains(MsgObject::Rescan)) {
        OpenDevices();
        return;
    }
    if(msg.prop.contains(MsgObject::GetUpdate)) {
        BuildModel();
        return;
    }
}

//void MidiDevices::MidiReceive_poll(PtTimestamp timestamp, void *userData)
//{
//    PmEvent buffer;
//    PmError result = pmNoError;

//    MidiDevices *devices = static_cast<MidiDevices*>(userData);

//    QMutexLocker l(&devices->mutexListMidi);

//    foreach(Connectables::MidiDevice* device, devices->listOpenedMidiDevices) {
//        if(device->GetSleep())
//            continue;

//        if(!device->stream || !device->queue)
//           continue;

//        device->Lock();

//        //it's a midi input
//        if(device->devInfo->input) {
//            do {
//                result = Pm_Poll(device->stream);
//                if (result) {
//                    PmError rslt = (PmError)Pm_Read(device->stream, &buffer, 1);
//                    if (rslt == pmBufferOverflow) {
//                        LOG("midi buffer overflow on"<<device->GetIndex()<<device->objectName());
//                        continue;
//                    }
//                    if(rslt == 1 ) {
//                        Pm_Enqueue(device->queue, &buffer);
//                    } else {
//                        LOG("midi in error on %1 %2"<<device->GetIndex()<<device->objectName());
//                        continue;
//                    }
//                }
//            } while (result);
//        }

//        //it's a midi output
//        if(device->devInfo->output) {
//            while (!Pm_QueueEmpty(device->queue)) {
//                result = Pm_Dequeue(device->queue, &buffer);
//                Pm_Write(device->stream, &buffer, 1);
//            }
//        }

//        device->Unlock();
//    }
//}
