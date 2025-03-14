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


#include "midisender.h"
#include "../globals.h"

using namespace Connectables;

MidiSender::MidiSender(MainHost *myHost,int index) :
        Object(myHost,index, ObjectInfo(NodeType::object, ObjType::MidiSender, tr("MidiSender")) ),
    midiMsg(0),
    msgChanged(false)
{

    listMsgType << "Note off";
    listMsgType << "Note on";
    listMsgType << "Ctrl";
    listMsgType << "ProgChang";

    for(int i=0;i<128;i++) {
        listValues << i;
    }

    for(int i=0;i<16;i++) {
        listChannels << (i+1);
    }

    listMidiPinOut->ChangeNumberOfPins(1);

    listParameterPinIn->AddPin(Param_MsgType);
    listParameterPinIn->AddPin(Param_Value1);
    listParameterPinIn->AddPin(Param_Value2);
    listParameterPinIn->AddPin(Param_Channel);
}

Pin* MidiSender::CreatePin(const ConnectionInfo &info)
{
    Pin *newPin = Object::CreatePin(info);
    if(newPin)
        return newPin;

    if(info.type!=PinType::Parameter) {
        LOG("wrong PinType"<<info.type);
        return 0;
    }

    pinConstructArgs args(info);
    args.parent = this;

    if(info.type == PinType::Parameter) {
        switch(info.pinNumber) {
            case Param_MsgType:
                args.name = "MsgType";
                args.listValues = &listMsgType;
                args.defaultVariantValue = "Ctrl";
                break;

            case Param_Value1:
                args.name = "Value1";
                args.listValues = &listValues;
                args.defaultVariantValue = 0;
                break;

            case Param_Value2:
                args.name = "Value2";
                args.listValues = &listValues;
                args.defaultVariantValue = 0;
                break;

             case Param_Channel:
                args.name = "Channel";
                args.listValues = &listChannels;
                args.defaultVariantValue = 1;
                break;

            default:
                return 0;
        }

        return PinFactory::MakePin(args);
    }

    return 0;
}

void MidiSender::Render()
{
    if(!msgChanged)
        return;

    msgChanged=false;

    unsigned char status=0;

    unsigned char msgType = static_cast<ParameterPinIn*>(listParameterPinIn->listPins.value(Param_MsgType))->GetStepIndex();
    unsigned char value1 = static_cast<ParameterPinIn*>(listParameterPinIn->listPins.value(Param_Value1))->GetVariantValue().toInt();
    unsigned char value2 = static_cast<ParameterPinIn*>(listParameterPinIn->listPins.value(Param_Value2))->GetVariantValue().toInt();
    unsigned char channel = static_cast<ParameterPinIn*>(listParameterPinIn->listPins.value(Param_Channel))->GetVariantValue().toInt();

    switch(msgType) {
        case 0:
            status = MidiConst::noteOff;
            break;
        case 1:
            status = MidiConst::noteOn;
            break;
        case 2:
            status = MidiConst::ctrl;
            break;
        case 3:
            status = MidiConst::prog;
            break;
    }

    status |= channel;

    midiMsg = MidiMsg(status, value1, value2);

    listMidiPinOut->GetPin(0)->SendMsg(PinMessage::MidiMsg, (void*)&midiMsg);
}

void MidiSender::OnParameterChanged(ConnectionInfo pinInfo, float value)
{
    Object::OnParameterChanged(pinInfo,value);

    msgChanged=true;
}
