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


#include "bridgepinout.h"
#include "audiobuffer.h"
#include "mainhost.h"
using namespace Connectables;

BridgePinOut::BridgePinOut(Object *parent, int number, bool bridge)
    :Pin(parent,PinType::Bridge,PinDirection::Output,number,bridge),
    valueType(PinType::ND)
{
    setObjectName(QString("BOut%1").arg(number));
    visible=true;
}

void BridgePinOut::SendMsg(const PinMessage::Enum msgType,void *data)
{
    Pin::SendMsg(msgType,data);

    switch(msgType) {
        case PinMessage::AudioBuffer :
            if(static_cast<AudioBuffer*>(data)->GetCurrentVu() < 0.01)
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

    valueChanged=true;
}

float BridgePinOut::GetValue()
{
    if(valueChanged) {
        if(value==1.0f) value=0.99f;
        else value=1.0f;
//        parent->getHost()->GetModel()->setData(modelIndex, valueType, UserRoles::type);
    }
    return value;
}
