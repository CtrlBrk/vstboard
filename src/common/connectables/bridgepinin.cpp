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


#include "bridgepinin.h"
#include "object.h"
#include "audiobuffer.h"
#include "mainhost.h"

using namespace Connectables;

BridgePinIn::BridgePinIn(Object *parent, int number, bool bridge) :
    Pin(parent,PinType::Bridge,PinDirection::Input,number,bridge),
    valueType(PinType::ND),
    msgCount(0)
{
    setObjectName(QString("BIn%1").arg(number));
    visible=true;
}

void BridgePinIn::ReceivePinMsg(const PinMessage::Enum msgType,void *data)
{
    if(msgCount==50) {
        return;
    }

    messagesData[msgCount]=data;
    messagesType[msgCount]=msgType;
    ++msgCount;

    valueChanged=true;
}

void BridgePinIn::NewRenderLoop()
{
    msgCount=0;
}

void BridgePinIn::SendMsgToOutput()
{
    if(msgCount==0)
        return;

    ConnectionInfo info = connectInfo;
    info.direction=PinDirection::Output;

    for(int i=0; i<msgCount; i++) {
        parent->GetPin(info)->SendMsg(messagesType[i],messagesData[i]);
    }

    switch(messagesType[msgCount-1]) {
        case PinMessage::AudioBuffer :
        if(static_cast<AudioBuffer*>(messagesData[msgCount-1])->GetCurrentVu() < 0.01)
                return;
            valueType=PinType::Audio;
            break;
        case PinMessage::ParameterValue :
            valueType=PinType::Parameter;
            break;
        case PinMessage::MidiMsg:
            valueType=PinType::Midi;
            break;
        default :
            valueType=PinType::ND;
    }
}

float BridgePinIn::GetValue()
{
    if(valueChanged) {
        if(value==1.0f) value=0.99f;
        else value=1.0f;
    }
    return value;
}
