/******************************************************************************
#    Copyright 2010 Rapha�l Fran�ois
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
******************************************************************************/

#include "audiodeviceout.h"
#include "audiodevice.h"
#include "../globals.h"
#include "../audiobuffer.h"
#include "../mainhost.h"
#include "../audiodevices.h"

using namespace Connectables;

AudioDeviceOut::AudioDeviceOut(int index, const ObjectInfo &info) :
    Object(index, info),
    parentDevice(0)
{
    listParameterPinOut.insert(0, new ParameterPinOut(this,0,0,true,"cpu%"));
}

AudioDeviceOut::~AudioDeviceOut()
{
    Close();
}

bool AudioDeviceOut::Close()
{
    if(!Object::Close())
        return false;

    if(parentDevice) {
        parentDevice->SetObjectOutput(0);
        parentDevice.clear();
    }
    return true;
}

void AudioDeviceOut::Render()
{
    listParameterPinOut.value(0)->ChangeValue(parentDevice->GetCpuUsage());
}

void AudioDeviceOut::SetBufferSize(long size)
{
    foreach(AudioPinIn *pin, listAudioPinIn) {
        pin->buffer->SetSize(size);
    }
}

bool AudioDeviceOut::Open()
{
    //can we find this device on this computer ?
    if(!AudioDevice::FindDeviceFromName(objInfo)) {
        debug("AudioDeviceOut::Open device not found")
        return false;
    }

    closed=false;

    //create the audiodevice if needed
    {
        QMutexLocker l(&AudioDevice::listDevMutex);

        if(!AudioDevices::listAudioDevices.contains(objInfo.id)) {
    //        parentDevice = new AudioDevice(objInfo);
            AudioDevice *dev = new AudioDevice(objInfo);
            parentDevice = QSharedPointer<AudioDevice>(dev);

    //        parentDevice = dev->GetSharedPointer().toStrongRef();
            if(!parentDevice->Open()) {
    //            delete parentDevice;
                parentDevice.clear();
                return false;
            }
            AudioDevices::listAudioDevices.insert(objInfo.id, parentDevice);
        } else {
            parentDevice = AudioDevices::listAudioDevices.value(objInfo.id);
        }
    }

    if(!parentDevice)
        return false;

    //if no output channels
    if(parentDevice->devInfo.maxOutputChannels==0) {
        parentDevice.clear();
        return false;
    }

    for(int i=0;i<parentDevice->devInfo.maxOutputChannels;i++) {
//        AudioPinIn *pin = new AudioPinIn(this,i,true);
        AudioPinIn *pin = new AudioPinIn(this,i);
        pin->buffer->SetSize(MainHost::Get()->GetBufferSize());
        pin->setObjectName(QString("Output %1").arg(i));
        listAudioPinIn << pin;
    }

    //device already has a child
    if(!parentDevice->SetObjectOutput(this)) {
//        parentDevice = 0;
        parentDevice.clear();
        return false;
    }

    Object::Open();
    return true;
}
