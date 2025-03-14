/**************************************************************************
#    Copyright 2010-2025 Raphaël François
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


#include "midipinout.h"
#include "pin.h"
#include "object.h"

using namespace Connectables;

MidiPinOut::MidiPinOut(const pinConstructArgs &conf) :
    Pin(conf)
{
    setObjectName(QString("MidiOut%1").arg(conf.pinNumber));
}

void MidiPinOut::SendMsg(const PinMessage::Enum msgType,void *data) {
    Pin::SendMsg(msgType,data);
    valueChanged=true;
}

float MidiPinOut::GetValue()
{
    if(valueChanged) {
        if(value==1.0f) value=0.99f;
        else value=1.0f;
    }
    return value;
}
