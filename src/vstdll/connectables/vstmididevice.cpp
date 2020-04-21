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

#include "vstmididevice.h"
#include "globals.h"
#include "vstboardprocessor.h"

using namespace Connectables;

VstMidiDevice::VstMidiDevice(MainHost *myHost, int index, const ObjectInfo &info) :
        Object(myHost,index, info)
{
}

VstMidiDevice::~VstMidiDevice()
{
    Close();
}

void VstMidiDevice::Render()
{
	//send msg from output pins
    if(objInfo.outputs) {
		QMutexLocker objlock(&objMutex);

        foreach(long msg, midiQueue) {
            foreach(Pin *pin,listMidiPinOut->listPins) {
                //static_cast<MidiPinOut*>(pin)->SendMsg(PinMessage::MidiMsg,(void*)&buffer.message);
                static_cast<MidiPinOut*>(pin)->SendMsg(PinMessage::MidiMsg,(void*)&msg);
            }
        }
		midiQueue.clear();

		/*
		foreach(const Vst::Event &msg, eventQueue) {
			foreach(Pin *pin, listMidiPinOut->listPins) {
				static_cast<MidiPinOut*>(pin)->SendMsg(PinMessage::VstEvent, (void*)&msg);
			}
		}
		eventQueue.clear();
*/
    }
}

void VstMidiDevice::MidiMsgFromInput(long msg) {
    if(objInfo.outputs) {
		QMutexLocker objlock(&objMutex);
        midiQueue << msg;
    }
}

void VstMidiDevice::EventFromInput(const Vst::Event &event) {
	if (objInfo.outputs) {
		QMutexLocker objlock(&objMutex);
		
		//eventQueue << event;

		uint8_t status = 0;
		uint8_t channel = 0;
		uint8_t midiData0 = 0;
		uint8_t midiData1 = 0;

		switch(event.type) {
		case Vst::Event::kNoteOnEvent:
			status = MidiConst::noteOn;
			channel = event.noteOn.channel;
			midiData0 = event.noteOn.pitch;
			midiData1 = event.noteOn.velocity * 127.f;
			midiQueue << MidiMessage(channel | status, midiData0, midiData1);
			break;
		case Vst::Event::kNoteOffEvent:
			status = MidiConst::noteOff;
			channel = event.noteOff.channel;
			midiData0 = event.noteOff.pitch;
			midiData1 = event.noteOff.velocity * 127.f;
			midiQueue << MidiMessage(channel | status, midiData0, midiData1);
			break;
		case Vst::Event::kPolyPressureEvent:
			status = MidiConst::aftertouch;
			channel = event.polyPressure.channel;
			midiData0 = event.polyPressure.pitch;
			midiData1 = event.polyPressure.pressure * 127.f;
			midiQueue << MidiMessage(channel | status, midiData0, midiData1);
			break;
		case Vst::Event::kLegacyMIDICCOutEvent:
			status = MidiConst::ctrl;
			channel = event.midiCCOut.channel;
			midiData0 = event.midiCCOut.value;
			midiData1 = event.midiCCOut.value2;
			midiQueue << MidiMessage(channel | status, midiData0, midiData1);
			break;
		case Vst::Event::kDataEvent:
			//a normal midi msg maybe ?
			if (event.data.size == 3) {
				long msg = *(long*)event.data.bytes;
				midiQueue << msg;
			}
		}
	}
}

bool VstMidiDevice::Close()
{
    if(objInfo.inputs>0)
        static_cast<VstBoardProcessor*>(myHost)->removeMidiIn(this);

    if(objInfo.outputs>0)
        static_cast<VstBoardProcessor*>(myHost)->removeMidiOut(this);

    return Object::Close();
}

bool VstMidiDevice::Open()
{
	//has output pins, so that's a midi in
    if(objInfo.outputs>0)
        static_cast<VstBoardProcessor*>(myHost)->addMidiIn(this);

	//input pins of a midi out device
    if(objInfo.inputs>0)
        static_cast<VstBoardProcessor*>(myHost)->addMidiOut(this);

    listMidiPinOut->ChangeNumberOfPins(objInfo.outputs);
    listMidiPinIn->ChangeNumberOfPins(objInfo.inputs);

    Object::Open();
    return true;
}
