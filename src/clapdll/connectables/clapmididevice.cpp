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

#include "clapmididevice.h"
#include "globals.h"
#include "clapmainhost.h"

using namespace Connectables;
ClapMidiDevice::ClapMidiDevice(MainHost *myHost, int index, const ObjectInfo &info) :
    Object(myHost,index, info)
{

}


ClapMidiDevice::~ClapMidiDevice()
{
    Close();
}

void ClapMidiDevice::Render()
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

void ClapMidiDevice::MidiMsgFromInput(long msg) {
    if(objInfo.outputs) {
        QMutexLocker objlock(&objMutex);
        midiQueue << msg;
    }
}

void ClapMidiDevice::EventFromInput(const clap_input_events_t *events) {
    if (objInfo.outputs) {
        QMutexLocker objlock(&objMutex);

        auto size = events->size(events);
        const clap_event_header_t *nextEvent{nullptr};
        uint32_t evId{0};

        while(evId < size) {
            nextEvent = events->get(events,evId);
            HandleEvent( nextEvent );
            evId++;
        }
    }
}

void ClapMidiDevice::HandleEvent(const clap_event_header_t *evt)
{
    if (evt->space_id != CLAP_CORE_EVENT_SPACE_ID)
        return;

    // uint8_t status = 0;
    // uint8_t channel = 0;
    // uint8_t midiData0 = 0;
    // uint8_t midiData1 = 0;


    switch (evt->type)
    {
    case CLAP_EVENT_MIDI:
    {
        auto mevt = reinterpret_cast<const clap_event_midi *>(evt);
        // auto msg = mevt->data[0] & 0xF0;
        // auto chan = mevt->data[0] & 0x0F;

        midiQueue << MidiMsg(mevt->data[0], mevt->data[1], mevt->data[2]);
        /*
        switch (msg)
        {
        case 0x90:
        {
            //note on
            status = MidiConst::noteOn;
            channel = chan;
            midiData0 = mevt->data[1];
            midiData1 = mevt->data[2];
            midiQueue << MidiMsg(channel | status, midiData0, midiData1);
            break;
        }
        case 0x80:
        {
            //note off

            break;
        }
        case 0xE0:
        {
            // pitch bend
            auto bv = (mevt->data[1] + mevt->data[2] * 128 - 8192) / 8192.0;

            break;
        }
        }
*/
        break;
    }
    case CLAP_EVENT_NOTE_ON:
    {
        auto nevt = reinterpret_cast<const clap_event_note *>(evt);
        midiQueue << MidiMsg(nevt->channel | MidiConst::noteOn, nevt->key, (int)(127.f*nevt->velocity) );
    }
    break;
    case CLAP_EVENT_NOTE_OFF:
    {
        auto nevt = reinterpret_cast<const clap_event_note *>(evt);
        midiQueue << MidiMsg(nevt->channel | MidiConst::noteOff, nevt->key, (int)(127.f*nevt->velocity) );
    }
    break;
    case CLAP_EVENT_PARAM_VALUE:
    {
        auto v = reinterpret_cast<const clap_event_param_value *>(evt);

    }
    break;
    }
}

bool ClapMidiDevice::Close()
{
    if(objInfo.inputs>0)
        static_cast<ClapMainHost*>(myHost)->removeMidiIn(this);

    if(objInfo.outputs>0)
        static_cast<ClapMainHost*>(myHost)->removeMidiOut(this);

    return Object::Close();
}

bool ClapMidiDevice::Open()
{
    //has output pins, so that's a midi in
    if(objInfo.outputs>0)
        static_cast<ClapMainHost*>(myHost)->addMidiIn(this);

    //input pins of a midi out device
    if(objInfo.inputs>0)
        static_cast<ClapMainHost*>(myHost)->addMidiOut(this);

    listMidiPinOut->ChangeNumberOfPins(objInfo.outputs);
    listMidiPinIn->ChangeNumberOfPins(objInfo.inputs);

    Object::Open();
    return true;
}
