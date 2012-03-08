/**************************************************************************
#    Copyright 2010-2012 Raphaël François
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
#include "mididevices.h"
#include "connectables/objectinfo.h"
#include "mainhosthost.h"

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
    if(Pt_Started())
        Pt_Stop();

    if(pmOpened)
        Pm_Terminate();

}

void MidiDevices::OpenDevices()
{
    mutexListMidi.lock();
    foreach(Connectables::MidiDevice* md, listOpenedMidiDevices) {
        md->SetSleep(true);
        md->CloseStream();
    }
    mutexListMidi.unlock();

    if(Pt_Started())
        Pt_Stop();

    if(pmOpened) {
        Pm_Terminate();
//        model->deleteLater();
        pmOpened=false;
    }

    PmError pmRet = Pm_Initialize();
    if(pmRet!=pmNoError) {
        QMessageBox msgBox;
        msgBox.setText(tr("Unable to initialize midi engine : %1").arg( Pm_GetErrorText(pmRet) ));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }

    PtError ptRet = Pt_Start(1, MidiDevices::MidiReceive_poll,this);
    if(ptRet!=ptNoError) {
        QMessageBox msgBox;
        msgBox.setText(tr("Unable to start midi engine"));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }
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

    listOpenedDevices << objPtr->info().id;

    MsgObject msg(GetIndex());
    msg.prop[MsgObject::State]=true;
    msg.prop[MsgObject::Id]=objPtr->info().id;
    msgCtrl->SendMsg(msg);
}

void MidiDevices::CloseDevice(Connectables::MidiDevice* objPtr)
{
    listOpenedDevices.removeAll(objPtr->info().id);

    MsgObject msg(GetIndex());
    msg.prop[MsgObject::State]=false;
    msg.prop[MsgObject::Id]=objPtr->info().id;
    msgCtrl->SendMsg(msg);

    mutexListMidi.lock();
    listOpenedMidiDevices.removeAll( objPtr );
    mutexListMidi.unlock();
}

void MidiDevices::BuildModel()
{
    MsgObject msg(GetIndex());
    msg.prop[MsgObject::Update]=1;

    QString lastName;
    int cptDuplicateNames=0;

    for(int i=0;i<Pm_CountDevices();i++) {
        const PmDeviceInfo *devInfo = Pm_GetDeviceInfo(i);

        QString devName= QString::fromLocal8Bit(devInfo->name);
        if(lastName == devName) {
            cptDuplicateNames++;
        } else {
            cptDuplicateNames=0;
        }
        lastName = devName;

        ObjectInfo obj;
        obj.nodeType = NodeType::object;
        obj.objType = ObjType::MidiInterface;
        obj.id = i;
        obj.name = devName;
        obj.apiName = QString::fromLocal8Bit(devInfo->interf );
        obj.duplicateNamesCounter = cptDuplicateNames;
        obj.inputs = devInfo->input;
        obj.outputs = devInfo->output;

        MsgObject msgDevice;
        msgDevice.prop[MsgObject::Name]=devName;
        msgDevice.prop[MsgObject::ObjInfo]=QVariant::fromValue(obj);
        msgDevice.prop[MsgObject::State]=(bool)listOpenedDevices.contains(obj.id);
        msg.children << msgDevice;
    }
    msgCtrl->SendMsg(msg);
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

void MidiDevices::MidiReceive_poll(PtTimestamp timestamp, void *userData)
{
    PmEvent buffer;
    PmError result = pmNoError;

    MidiDevices *devices = static_cast<MidiDevices*>(userData);

    QMutexLocker l(&devices->mutexListMidi);

    foreach(Connectables::MidiDevice* device, devices->listOpenedMidiDevices) {
        if(device->GetSleep())
            continue;

        if(!device->stream || !device->queue)
           continue;

        device->Lock();

        //it's a midi input
        if(device->devInfo->input) {
            do {
                result = Pm_Poll(device->stream);
                if (result) {
                    PmError rslt = (PmError)Pm_Read(device->stream, &buffer, 1);
                    if (rslt == pmBufferOverflow) {
                        LOG("midi buffer overflow on"<<device->GetIndex()<<device->objectName());
                        continue;
                    }
                    if(rslt == 1 ) {
                        Pm_Enqueue(device->queue, &buffer);
                    } else {
                        LOG("midi in error on %1 %2"<<device->GetIndex()<<device->objectName());
                        continue;
                    }
                }
            } while (result);
        }

        //it's a midi output
        if(device->devInfo->output) {
            while (!Pm_QueueEmpty(device->queue)) {
                result = Pm_Dequeue(device->queue, &buffer);
                Pm_Write(device->stream, &buffer, 1);
            }
        }

        device->Unlock();
    }
}
