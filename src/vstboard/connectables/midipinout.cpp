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

#include "midipinout.h"
#include "pin.h"
#include "object.h"

using namespace Connectables;

MidiPinOut::MidiPinOut(Object *parent, int number, bool bridge)
    :Pin(parent,PinType::Midi,PinDirection::Output,number,bridge),
    vuVal(.0f)
{
    falloff = 0.05f;

    setObjectName(QString("MidiOut%1").arg(number));
    visible=true;
}

void MidiPinOut::SendMsg(int msgType,void *data) {
    Pin::SendMsg(msgType,data);
    value=1.0f;
}

float MidiPinOut::GetValue()
{
    if(vuVal!=value)
        valueChanged=true;

    vuVal=value;
    value=.0f;
    return vuVal;
}
