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

#include "mainhosthost.h"
#include "connectables/mididevice.h"
#include "globals.h"

using namespace Connectables;

MidiDevice::MidiDevice(MainHost *myHost,int index, const ObjectInfo &info) :
        Object(myHost,index, info),
        deviceIn(0),
        deviceOut(0),
        deviceOpened(false)
{
}

MidiDevice::~MidiDevice()
{
    Close();
}

void MidiDevice::Render()
{
    if(!deviceOpened)
        return;

    if(deviceIn) {
        std::vector<unsigned char> message;
        double stamp = deviceIn->getMessage( &message );
        int nBytes = message.size();

        for ( int i=0; i<nBytes; i++ ) {
          std::cout << "Byte " << i << " = " << (int)message[i] << ", ";
          foreach(Pin *pin,listMidiPinOut->listPins) {
              pin->SendMsg(PinMessage::MidiMsg,(void*)&message[i]);
          }
        }

//        if ( nBytes > 0 ) {
//          std::cout << "stamp = " << stamp << std::endl;
//        }

    }
}

void MidiDevice::MidiMsgFromInput(long msg) {
    if(deviceOut) {
        QMutexLocker objlock(&objMutex);
        unsigned char* c = (unsigned char*)&msg;
        deviceOut->sendMessage( c, 3);
    }
}

bool MidiDevice::OpenStream()
{
    if(deviceOpened)
        return true;

    QMutexLocker objlock(&objMutex);

    if(deviceOut) {
        deviceOut->openPort(objInfo.id);
    }

    if(deviceIn) {
        deviceIn->openPort(objInfo.id);
        deviceIn->ignoreTypes( false, false, false );
    }

    deviceOpened=true;
    return true;
}

bool MidiDevice::CloseStream()
{
    deviceOpened=false;
    return true;
}

bool MidiDevice::Close()
{
    MainHostHost *host = static_cast<MainHostHost*>(myHost);
    MidiDevices *devCtrl = host->midiDevices;
    if(devCtrl)
        devCtrl->CloseDevice(this);

    CloseStream();
    if(deviceIn) {
        delete deviceIn;
        deviceIn=0;
    }
    if(deviceOut) {
        delete deviceOut;
        deviceOut=0;
    }
    return true;
}

bool MidiDevice::Open()
{
    SetErrorMessage("");
    closed=false;

    RtMidi::Api apiId = MidiDevices::GetApiByName( objInfo.apiName.toStdString() );
    if(apiId == RtMidi::UNSPECIFIED) {
        SetErrorMessage( tr("Api %1 not found").arg(objInfo.apiName) );
        return true;
    }
    objInfo.api = apiId;

    int devId = MidiDevices::GetDevIdByName( objInfo );
    if(devId == -1) {
        SetErrorMessage( tr("Device %1:%2 not found").arg(objInfo.apiName).arg(objInfo.name) );
        return true;
    }
    objInfo.id = devId;

    if(objInfo.inputs>0) {
        deviceIn = new RtMidiIn( (RtMidi::Api)objInfo.api);
        listMidiPinOut->ChangeNumberOfPins(1);
    }
    if(objInfo.outputs>0) {
        deviceOut = new RtMidiOut( (RtMidi::Api)objInfo.api);
        listMidiPinIn->ChangeNumberOfPins(1);
    }

    //error while opening device, delete it now
    if(!OpenStream()) {
        return true;
    }

    Object::Open();

    MidiDevices *devCtrl = static_cast<MainHostHost*>(myHost)->midiDevices;
    if(devCtrl)
        devCtrl->OpenDevice(this);
    return true;
}
