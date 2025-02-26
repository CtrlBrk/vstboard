//------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Helpers
// Filename    : public.sdk/source/vst/vst2wrapper/vst2wrapper.cpp
// Created by  : Steinberg, 01/2009
// Description : VST 3 -> VST 2 Wrapper
//
//-----------------------------------------------------------------------------
// LICENSE
// (c) 2019, Steinberg Media Technologies GmbH, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this
//     software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

///**************************************************************************
//#    Copyright 2010-2020 Raphaël François
//#    Contact : ctrlbrk76@gmail.com
//#
//#    This file is part of VstBoard.
//#
//#    VstBoard is free software: you can redistribute it and/or modify
//#    it under the terms of the under the terms of the GNU Lesser General Public License as published by
//#    the Free Software Foundation, either version 3 of the License, or
//#    (at your option) any later version.
//#
//#    VstBoard is distributed in the hope that it will be useful,
//#    but WITHOUT ANY WARRANTY; without even the implied warranty of
//#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#    under the terms of the GNU Lesser General Public License for more details.
//#
//#    You should have received a copy of the under the terms of the GNU Lesser General Public License
//#    along with VstBoard.  If not, see <http://www.gnu.org/licenses/>.
//**************************************************************************/

#include "myvst2wrapper.h"
#include "vstboardprocessor.h"
#include "pluginterfaces/vst/ivstmidicontrollers.h"

#include "gui.h"

using namespace Steinberg;
using namespace Steinberg::Vst;

class MyVst2EditorWrapper : public BaseEditorWrapper
{
public:
    MyVst2EditorWrapper (MyVst2Wrapper* effect, IEditController* controller);
    bool getVstRect (VstRect** rect) ;
    void open(HWND w);
    VstRect mERect;
};

MyVst2EditorWrapper::MyVst2EditorWrapper (MyVst2Wrapper* effect, IEditController* controller)
    : BaseEditorWrapper (controller)
{
}

void MyVst2EditorWrapper::open(HWND w)
{
    createView();
    // MainWindow* win = dynamic_cast<Gui*>(mView.get())->GetWindow();
    // HWND w = (HWND)win->winId();
    _open(w);
}

bool MyVst2EditorWrapper::getVstRect (VstRect** rect)
{
    createView();

    IPlugView* i = mView.get();
    MainWindow* win = dynamic_cast<Gui*>(mView.get())->GetWindow();
    if(win) {

        mERect.left = (short)win->pos().x();
        mERect.top = (short)win->pos().y();
        mERect.right = (short) (win->pos().x() + win->size().width());
        mERect.bottom = (short) (win->pos().y() + win->size().height());

        *rect = &mERect;
        return true;
    }
    *rect = nullptr;
    return false;
}


//------------------------------------------------------------------------
// Vst2MidiEventQueue Declaration
//------------------------------------------------------------------------
class Vst2MidiEventQueue
{
public:
    //------------------------------------------------------------------------
    Vst2MidiEventQueue (int32 maxEventCount);
    ~Vst2MidiEventQueue ();

    bool isEmpty () const { return eventList->numEvents == 0; }
    bool add (const VstMidiEvent& e);
    // bool add (const VstMidiSysexEvent& e);
    void flush ();

    operator VstEvents* () { return eventList; }
    //------------------------------------------------------------------------
protected:
    VstEvents* eventList;
    int32 maxEventCount;
};

//------------------------------------------------------------------------
// Vst2MidiEventQueue Implementation
//------------------------------------------------------------------------
Vst2MidiEventQueue::Vst2MidiEventQueue (int32 _maxEventCount) : maxEventCount (_maxEventCount)
{
    eventList = (VstEvents*)new char[sizeof (VstEvents) + (maxEventCount - 2) * sizeof (VstEvent*)];
    eventList->numEvents = 0;
    eventList->reserved = 0;
   /*
    int32 eventSize = sizeof (VstMidiSysexEvent) > sizeof (VstMidiEvent) ?
                          sizeof (VstMidiSysexEvent) :
                          sizeof (VstMidiEvent);
    */

    int32 eventSize = sizeof (VstMidiEvent);

    for (int32 i = 0; i < maxEventCount; i++)
    {
        char* eventBuffer = new char[eventSize];
        memset (eventBuffer, 0, eventSize);
        eventList->events[i] = (VstEvent*)eventBuffer;
    }
}

//------------------------------------------------------------------------
Vst2MidiEventQueue::~Vst2MidiEventQueue ()
{
    for (int32 i = 0; i < maxEventCount; i++)
        delete[] (char*) eventList->events[i];

    delete[] (char*) eventList;
}

//------------------------------------------------------------------------
bool Vst2MidiEventQueue::add (const VstMidiEvent& e)
{
    if (eventList->numEvents >= maxEventCount)
        return false;

    auto* dst = (VstMidiEvent*)eventList->events[eventList->numEvents++];
    memcpy (dst, &e, sizeof (VstMidiEvent));
    dst->type = kVstMidiType;
    dst->byteSize = sizeof (VstMidiEvent);
    return true;
}

//------------------------------------------------------------------------
void Vst2MidiEventQueue::flush ()
{
    eventList->numEvents = 0;
}


intptr_t static_dispatcher(AEffect* pEffect, int opCode, int index, intptr_t value, void* ptr, float opt)
{
    MyVst2Wrapper* e=(MyVst2Wrapper*)pEffect->ptr3;
    return e->__dispatcher( opCode,  index,  value,  ptr,  opt);
}

void static_setParameter(AEffect* pEffect, int param, float value)
{
    MyVst2Wrapper* e=(MyVst2Wrapper*)pEffect->ptr3;
    e->__setParameter(param, value);
}

float static_getParameter(AEffect* pEffect, int param)
{
    MyVst2Wrapper* e=(MyVst2Wrapper*)pEffect->ptr3;
    return e->__getParameter(param);
}

void static_processReplacing (AEffect* pEffect, float** inputs, float** outputs, int sampleFrames)
{
    MyVst2Wrapper* e=(MyVst2Wrapper*)pEffect->ptr3;
    e->__processReplacing( inputs,  outputs,  sampleFrames);

}
void static_process(AEffect* pEffect, float** inputs, float** outputs, int sampleFrames)
{
    MyVst2Wrapper* e=(MyVst2Wrapper*)pEffect->ptr3;
    e->__process( inputs,  outputs,  sampleFrames);
}


MyVst2Wrapper::MyVst2Wrapper(Steinberg::Vst::BaseWrapper::SVST3Config config, audioMasterCallback audioMaster, int vst2ID)
: BaseWrapper (config)
{
    Effect.magic=kEffectMagic;
    Effect.numPrograms=0;
    Effect.numParams=0;
    Effect.numInputs=2;
    Effect.numOutputs=2;
    Effect.flags= effFlagsHasEditor | effFlagsCanReplacing | effFlagsProgramChunks ; //effFlagsIsSynth
    Effect.ptr1=0;
    Effect.ptr2=0;
    Effect.initialDelay=0;
    Effect.empty3a=0;
    Effect.empty3b=0;
    Effect.unkown_float=.0f;
    Effect.ptr3=this;
    Effect.user=0;
    Effect.uniqueID=0;
    Effect.version=0;

    Effect.dispatcher = static_dispatcher;
    Effect.setParameter = static_setParameter;
    Effect.getParameter = static_getParameter;
    Effect.process = static_process;
    Effect.processReplacing = static_processReplacing;
}

MyVst2Wrapper* MyVst2Wrapper::create (IPluginFactory* factory, const TUID vst3ComponentID, int vst2ID, audioMasterCallback audioMaster)
{
    if (!factory)
        return nullptr;

    BaseWrapper::SVST3Config config;
    config.factory = factory;
    config.processor = nullptr;

    FReleaser factoryReleaser (factory);

    factory->createInstance (vst3ComponentID, IAudioProcessor::iid, (void**)&config.processor);
    if (config.processor)
    {
        config.controller = nullptr;
        if (config.processor->queryInterface (IEditController::iid, (void**)&config.controller) != kResultTrue)
        {
            FUnknownPtr<IComponent> component (config.processor);
            if (component)
            {
                TUID editorCID;
                if (component->getControllerClassId (editorCID) == kResultTrue)
                {
                    factory->createInstance (editorCID, IEditController::iid, (void**)&config.controller);
                }
            }
        }

        config.vst3ComponentID = FUID::fromTUID (vst3ComponentID);

        auto* wrapper = new MyVst2Wrapper (config, audioMaster, vst2ID);
        FUnknownPtr<IPluginFactory2> factory2 (factory);
        if (factory2)
        {
            PFactoryInfo factoryInfo;
            if (factory2->getFactoryInfo (&factoryInfo) == kResultTrue) {
                memcpy (wrapper->mVendor, factoryInfo.vendor, sizeof (wrapper->mVendor));
            }

            for (int32 i = 0; i < factory2->countClasses (); i++)
            {
                Steinberg::PClassInfo2 classInfo2;
                if (factory2->getClassInfo2 (i, &classInfo2) == Steinberg::kResultTrue)
                {
                    if (memcmp (classInfo2.cid, vst3ComponentID, sizeof (TUID)) == 0)
                    {
                        memcpy (wrapper->mSubCategories, classInfo2.subCategories, sizeof (mSubCategories));
                        memcpy (wrapper->mName, classInfo2.name, sizeof (wrapper->mName));
                        //wrapper->mVersion = classInfo2.version;
                        wrapper->mVersion = 0;

                        if (classInfo2.vendor[0] != 0) {
                            memcpy (wrapper->mVendor, classInfo2.vendor, sizeof (wrapper->mVendor));
                        }
                        break;
                    }
                }
            }
        }

        if (wrapper->init () == false)
        {
            // wrapper->release ();
            return nullptr;
        }

        return wrapper;
    }

    return nullptr;
}

tresult PLUGIN_API MyVst2Wrapper::getName (String128 name)
{
    char8 productString[128];
  /*  if (getHostProductString (productString))
    {
        String str (productString);
        str.copyTo16 (name, 0, 127);

        return kResultTrue;
    }
*/
    return kResultFalse;
}

bool MyVst2Wrapper::_sizeWindow (int32 width, int32 height)
{
    // return sizeWindow (width, height);
    return true;
}

void MyVst2Wrapper::setupProcessTimeInfo ()
{
    /*
    // VstTimeInfo* vst2timeInfo = AudioEffectX::getTimeInfo (0xffffffff);
    if (vst2timeInfo)
    {
        const uint32 portableFlags =
            ProcessContext::kPlaying | ProcessContext::kCycleActive | ProcessContext::kRecording |
            ProcessContext::kSystemTimeValid | ProcessContext::kProjectTimeMusicValid |
            ProcessContext::kBarPositionValid | ProcessContext::kCycleValid |
            ProcessContext::kTempoValid | ProcessContext::kTimeSigValid |
            ProcessContext::kSmpteValid | ProcessContext::kClockValid;

        mProcessContext.state = ((uint32)vst2timeInfo->flags) & portableFlags;
        mProcessContext.sampleRate = vst2timeInfo->sampleRate;
        mProcessContext.projectTimeSamples = (TSamples)vst2timeInfo->samplePos;

        if (mProcessContext.state & ProcessContext::kSystemTimeValid)
            mProcessContext.systemTime = (TSamples)vst2timeInfo->nanoSeconds;
        else
            mProcessContext.systemTime = 0;

        if (mProcessContext.state & ProcessContext::kProjectTimeMusicValid)
            mProcessContext.projectTimeMusic = vst2timeInfo->ppqPos;
        else
            mProcessContext.projectTimeMusic = 0;

        if (mProcessContext.state & ProcessContext::kBarPositionValid)
            mProcessContext.barPositionMusic = vst2timeInfo->barStartPos;
        else
            mProcessContext.barPositionMusic = 0;

        if (mProcessContext.state & ProcessContext::kCycleValid)
        {
            mProcessContext.cycleStartMusic = vst2timeInfo->cycleStartPos;
            mProcessContext.cycleEndMusic = vst2timeInfo->cycleEndPos;
        }
        else
            mProcessContext.cycleStartMusic = mProcessContext.cycleEndMusic = 0.0;

        if (mProcessContext.state & ProcessContext::kTempoValid)
            mProcessContext.tempo = vst2timeInfo->tempo;
        else
            mProcessContext.tempo = 120.0;

        if (mProcessContext.state & ProcessContext::kTimeSigValid)
        {
            mProcessContext.timeSigNumerator = vst2timeInfo->timeSigNumerator;
            mProcessContext.timeSigDenominator = vst2timeInfo->timeSigDenominator;
        }
        else
            mProcessContext.timeSigNumerator = mProcessContext.timeSigDenominator = 4;

        mProcessContext.frameRate.flags = 0;
        if (mProcessContext.state & ProcessContext::kSmpteValid)
        {
            mProcessContext.smpteOffsetSubframes = vst2timeInfo->smpteOffset;
            switch (vst2timeInfo->smpteFrameRate)
            {
            case kVstSmpte24fps: ///< 24 fps
                mProcessContext.frameRate.framesPerSecond = 24;
                break;
            case kVstSmpte25fps: ///< 25 fps
                mProcessContext.frameRate.framesPerSecond = 25;
                break;
            case kVstSmpte2997fps: ///< 29.97 fps
                mProcessContext.frameRate.framesPerSecond = 30;
                mProcessContext.frameRate.flags = FrameRate::kPullDownRate;
                break;
            case kVstSmpte30fps: ///< 30 fps
                mProcessContext.frameRate.framesPerSecond = 30;
                break;
            case kVstSmpte2997dfps: ///< 29.97 drop
                mProcessContext.frameRate.framesPerSecond = 30;
                mProcessContext.frameRate.flags =
                    FrameRate::kPullDownRate | FrameRate::kDropRate;
                break;
            case kVstSmpte30dfps: ///< 30 drop
                mProcessContext.frameRate.framesPerSecond = 30;
                mProcessContext.frameRate.flags = FrameRate::kDropRate;
                break;
            case kVstSmpteFilm16mm: // not a smpte rate
            case kVstSmpteFilm35mm:
                mProcessContext.state &= ~ProcessContext::kSmpteValid;
                break;
            case kVstSmpte239fps: ///< 23.9 fps
                mProcessContext.frameRate.framesPerSecond = 24;
                mProcessContext.frameRate.flags = FrameRate::kPullDownRate;
                break;
            case kVstSmpte249fps: ///< 24.9 fps
                mProcessContext.frameRate.framesPerSecond = 25;
                mProcessContext.frameRate.flags = FrameRate::kPullDownRate;
                break;
            case kVstSmpte599fps: ///< 59.9 fps
                mProcessContext.frameRate.framesPerSecond = 60;
                mProcessContext.frameRate.flags = FrameRate::kPullDownRate;
                break;
            case kVstSmpte60fps: ///< 60 fps
                mProcessContext.frameRate.framesPerSecond = 60;
                break;
            default: mProcessContext.state &= ~ProcessContext::kSmpteValid; break;
            }
        }
        else
        {
            mProcessContext.smpteOffsetSubframes = 0;
            mProcessContext.frameRate.framesPerSecond = 0;
        }

        ///< MIDI Clock Resolution (24 Per Quarter Note), can be negative (nearest)
        // if (mProcessContext.state & ProcessContext::kClockValid)
            // mProcessContext.samplesToNextClock = vst2timeInfo->samplesToNextClock;
        // else
            mProcessContext.samplesToNextClock = 0;

        mProcessData.processContext = &mProcessContext;
    }
    else
*/
        mProcessData.processContext = nullptr;
}

tresult PLUGIN_API MyVst2Wrapper::beginEdit (ParamID tag)
{
    std::map<ParamID, int32>::const_iterator iter = mParamIndexMap.find (tag);
    if (iter != mParamIndexMap.end ()) {
        // AudioEffectX::beginEdit ((*iter).second);
    }
    return kResultTrue;
}

tresult PLUGIN_API MyVst2Wrapper::performEdit (ParamID tag, ParamValue valueNormalized)
{
    std::map<ParamID, int32>::const_iterator iter = mParamIndexMap.find (tag);
    // if (iter != mParamIndexMap.end () && audioMaster) {
        // audioMaster (&cEffect, audioMasterAutomate, (*iter).second, 0, nullptr, (float)valueNormalized); // value is in opt
    // }
    mInputTransfer.addChange (tag, valueNormalized, 0);

    return kResultTrue;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API MyVst2Wrapper::endEdit (ParamID tag)
{
    std::map<ParamID, int32>::const_iterator iter = mParamIndexMap.find (tag);
    if (iter != mParamIndexMap.end ()) {
        // AudioEffectX::endEdit ((*iter).second);
    }
    return kResultTrue;
}


bool MyVst2Wrapper::init ()
{
    // if (strstr (mSubCategories, "Instrument"))
        // isSynth (true);

    bool res = BaseWrapper::init ();

    // numPrograms = cEffect.numPrograms = mNumPrograms;

    if (mController)
    {
        if (BaseEditorWrapper::hasEditor (mController))
        {
            _editor = new MyVst2EditorWrapper (this, mController);
            _setEditor (_editor);

            // setEditor (editor);
        }
    }
    return res;
}

VstPlugCategory MyVst2Wrapper::getPlugCategory ()
{
    if (mSubCategories[0])
    {
        if (strstr (mSubCategories, "Analyzer"))
            return kPlugCategAnalysis;

        else if (strstr (mSubCategories, "Delay") || strstr (mSubCategories, "Reverb"))
            return kPlugCategRoomFx;

        else if (strstr (mSubCategories, "Dynamics") || strstr (mSubCategories, "Mastering"))
            return kPlugCategMastering;

        else if (strstr (mSubCategories, "Restoration"))
            return kPlugCategRestoration;

        else if (strstr (mSubCategories, "Generator"))
            return kPlugCategGenerator;

        else if (strstr (mSubCategories, "Spatial"))
            return kPlugCategSpacializer;

        else if (strstr (mSubCategories, "Fx"))
            return kPlugCategEffect;

        else if (strstr (mSubCategories, "Instrument"))
            return kPlugCategSynth;
    }
    return kPlugCategUnknown;
}

int MyVst2Wrapper::canDo (char* text)
{
    if (stricmp (text, "sendVstEvents") == 0)
    {
        return -1;
    }
    else if (stricmp (text, "sendVstMidiEvent") == 0)
    {
        return mHasEventOutputBuses ? 1 : -1;
    }
    else if (stricmp (text, "receiveVstEvents") == 0)
    {
        return -1;
    }
    else if (stricmp (text, "receiveVstMidiEvent") == 0)
    {
        return mHasEventInputBuses ? 1 : -1;
    }
    else if (stricmp (text, "receiveVstTimeInfo") == 0)
    {
        return 1;
    }
    else if (stricmp (text, "offline") == 0)
    {
        if (mProcessing)
            return 0;
        if (mVst3processMode == kOffline)
            return 1;

        bool canOffline = setupProcessing (kOffline);
        setupProcessing ();
        return canOffline ? 1 : -1;
    }
    else if (stricmp (text, "midiProgramNames") == 0)
    {
        if (mUnitInfo)
        {
            UnitID unitId = -1;
            if (mUnitInfo->getUnitByBus (kEvent, kInput, 0, 0, unitId) == kResultTrue &&
                unitId >= 0)
                return 1;
        }
        return -1;
    }
    else if (stricmp (text, "bypass") == 0)
    {
        return mBypassParameterID != kNoParamId ? 1 : -1;
    }
    return 0; // do not know
}

String strProdName("VstBoard");


intptr_t MyVst2Wrapper::__dispatcher( int opCode, int index, intptr_t value, void* ptr, float opt)
{
    // LOG("opcode:" << opCode)

    switch(opCode) {
    case effGetPlugCategory: //35
        return getPlugCategory();
    case effGetVstVersion: //58
        return 1;
    case effSetSampleRate: //10
        _setSampleRate(value);
        return 1;
    case effSetBlockSize: //11
        _setBlockSize(value);
        return 1;
    case effCanDo: //51
        return canDo((char*)ptr);
    case effMainsChanged: //12
        if(value) {
            resume();
        } else {
            suspend();
        }
        return 0;
    case effBeginSetProgram: //67
        return 0;
    case effSetProgram: //2
        return 0;
    case effEndSetProgram: //68
        return 0;
    case effGetProductString: //48
        strProdName.copyTo16 ((char16*)ptr, 0, 127);
        return 1;
    case effStartProcess: //71
        BaseWrapper::_startProcess ();
        return 1;
    case effStopProcess: //72
        BaseWrapper::_stopProcess ();
        return 1;
    case effEditGetRect: //13
        _editor->getVstRect( (VstRect**)ptr );
        return 1;
    case effEditOpen: //14
        _editor->open( (HWND)ptr );
        return 1;
    case effEditClose: //15
        _editor->_close();
        return 1;
    case effEditIdle : //19
        // LOG("opcode:" << opCode)
        return 1;
    case effEditTop: //20
        return 0;
    case effProcessEvents: //25
        return processEvents( (VstEvents*)ptr );
    }

    LOG("opcode:" << opCode)

    return 0;
}

float MyVst2Wrapper::__getParameter(int index)
{
    return _getParameter(index);
}

void MyVst2Wrapper::__setParameter(int index, float value)
{
    if (!mController)
        return;

    if (index < (int32)mParameterMap.size ())
    {
        ParamID id = mParameterMap.at (index).vst3ID;
        addParameterChange (id, (ParamValue)value, 0);
    }
}

void MyVst2Wrapper::suspend ()
{
    BaseWrapper::_suspend ();
}

//------------------------------------------------------------------------
void MyVst2Wrapper::resume ()
{
    // AudioEffectX::resume ();
    BaseWrapper::_resume ();

    mChunk.setSize (0);
}

//------------------------------------------------------------------------
int MyVst2Wrapper::startProcess ()
{
    BaseWrapper::_startProcess ();
    return 0;
}

//------------------------------------------------------------------------
int MyVst2Wrapper::stopProcess ()
{
    BaseWrapper::_stopProcess ();
    return 0;
}

void MyVst2Wrapper::updateProcessLevel ()
{
    mVst3processMode = kRealtime;
    /*
    auto currentLevel = getCurrentProcessLevel ();
    if (mCurrentProcessLevel != currentLevel)
    {
        mCurrentProcessLevel = currentLevel;
        if (mCurrentProcessLevel == kVstProcessLevelOffline)
            mVst3processMode = kOffline;
        else
            mVst3processMode = kRealtime;

        bool callStartStop = mProcessing;

        if (callStartStop)
            stopProcess ();

        setupProcessing ();

        if (callStartStop)
            startProcess ();
    }
*/
}

void MyVst2Wrapper::__processReplacing (float** inputs, float** outputs, int sampleFrames)
{
    updateProcessLevel ();

    _processReplacing (inputs, outputs, sampleFrames);
}

void MyVst2Wrapper::__process (float** inputs, float** outputs, int sampleFrames)
{

}

void MyVst2Wrapper::processMidiEvent (Event& toAdd, char* midiData, bool isLive, int32 noteLength, float noteOffVelocity, float detune, int size)
{
    uint8 status = static_cast<uint8> (midiData[0] & kStatusMask);
    uint8 channel = static_cast<uint8> (midiData[0] & kChannelMask);

    // not allowed
    if (channel >= 16)
        return;

    if (isLive)
        toAdd.flags |= Event::kIsLive;

    //--- -----------------------------
    switch (status)
    {
    case kNoteOn:
    case kNoteOff:
    {
        if (status == kNoteOff || midiData[2] == 0) // note off
        {
            toAdd.type = Event::kNoteOffEvent;
            toAdd.noteOff.channel = channel;
            toAdd.noteOff.pitch = midiData[1];
            toAdd.noteOff.velocity = noteOffVelocity;
            toAdd.noteOff.noteId = -1; // TODO ?
        }
        else if (status == kNoteOn) // note on
        {
            toAdd.type = Event::kNoteOnEvent;
            toAdd.noteOn.channel = channel;
            toAdd.noteOn.pitch = midiData[1];
            toAdd.noteOn.tuning = detune;
            toAdd.noteOn.velocity = (float)midiData[2] * kMidiScaler;
            toAdd.noteOn.length = noteLength;
            toAdd.noteOn.noteId = -1; // TODO ?
        }
        mInputEvents->addEvent (toAdd);
    }
    break;
    //--- -----------------------------
    case kPolyPressure:
    {
        toAdd.type = Vst::Event::kPolyPressureEvent;
        toAdd.polyPressure.channel = channel;
        toAdd.polyPressure.pitch = static_cast<int16>(midiData[1] & kDataMask);
        toAdd.polyPressure.pressure = (float)(midiData[2] & kDataMask) * kMidiScaler;
        toAdd.polyPressure.noteId = -1; // TODO ?

        mInputEvents->addEvent (toAdd);
    }
    break;
    //--- -----------------------------
    case kController:
    {

        toAdd.type = Vst::Event::kDataEvent;
        toAdd.data.size = 3;
        toAdd.data.type = 0;
        toAdd.data.bytes = reinterpret_cast<uint8*>(midiData);
        mInputEvents->addEvent (toAdd);

/*
        if (toAdd.busIndex < kMaxMidiMappingBusses && mMidiCCMapping[toAdd.busIndex][channel])
        {
            ParamID paramID = midiData[1];
                // mMidiCCMapping[toAdd.busIndex][channel][static_cast<size_t> (midiData[1])];
            if (paramID != kNoParamId)
            {
                ParamValue value = (double)midiData[2] * kMidiScaler;

                int32 index = 0;
                if (IParamValueQueue* queue = mInputChanges.addParameterData (paramID, index))
                    queue->addPoint (toAdd.sampleOffset, value, index);
                // LOG(paramID << " " << value)
                mGuiTransfer.addChange (paramID, value, toAdd.sampleOffset);
            }
        }
*/
    }
    break;
    //--- -----------------------------
    case kPitchBendStatus:
    {
        toAdd.type = Vst::Event::kDataEvent;
        toAdd.data.size = 3;
        toAdd.data.type = 0;
        toAdd.data.bytes = reinterpret_cast<uint8*>(midiData);
        mInputEvents->addEvent (toAdd);
        /*
        if (toAdd.busIndex < kMaxMidiMappingBusses && mMidiCCMapping[toAdd.busIndex][channel])
        {
            ParamID paramID = mMidiCCMapping[toAdd.busIndex][channel][Vst::kPitchBend];
            if (paramID != kNoParamId)
            {
                const double kPitchWheelScaler = 1. / (double)0x3FFF;

                const int32 ctrl = static_cast<int32> ((midiData[1] & kDataMask) | ((midiData[2] & kDataMask) << 7));
                ParamValue value = kPitchWheelScaler * (double)ctrl;

                int32 index = 0;
                if (IParamValueQueue* queue = mInputChanges.addParameterData (paramID, index))
                    queue->addPoint (toAdd.sampleOffset, value, index);

                mGuiTransfer.addChange (paramID, value, toAdd.sampleOffset);
            }
        }
*/
    }
    break;
    //--- -----------------------------
    case kAfterTouchStatus:
    {
        toAdd.type = Vst::Event::kDataEvent;
        toAdd.data.size = 3;
        toAdd.data.type = 0;
        toAdd.data.bytes = reinterpret_cast<uint8*>(midiData);
        mInputEvents->addEvent (toAdd);
        /*
        if (toAdd.busIndex < kMaxMidiMappingBusses && mMidiCCMapping[toAdd.busIndex][channel])
        {
            ParamID paramID = mMidiCCMapping[toAdd.busIndex][channel][Vst::kAfterTouch];
            if (paramID != kNoParamId)
            {
                ParamValue value = (ParamValue) (midiData[1] & kDataMask) * kMidiScaler;

                int32 index = 0;
                if (IParamValueQueue* queue = mInputChanges.addParameterData (paramID, index))
                    queue->addPoint (toAdd.sampleOffset, value, index);

                mGuiTransfer.addChange (paramID, value, toAdd.sampleOffset);
            }
        }
*/
    }
    break;
    //--- -----------------------------
    case kProgramChangeStatus:
    {
        if (mProgramChangeParameterIDs[channel] != kNoParamId &&
            mProgramChangeParameterIdxs[channel] != -1)
        {
            ParameterInfo paramInfo = {};
            if (mController->getParameterInfo (mProgramChangeParameterIdxs[channel],
                                              paramInfo) == kResultTrue)
            {
                int32 program = midiData[1];
                if (paramInfo.stepCount > 0 && program <= paramInfo.stepCount)
                {
                    ParamValue normalized =
                        (ParamValue)program / (ParamValue)paramInfo.stepCount;
                    addParameterChange (mProgramChangeParameterIDs[channel], normalized,
                                       toAdd.sampleOffset);
                }
            }
        }
    }
    break;
    }
}

int MyVst2Wrapper::processEvents (VstEvents* events)
{

    if (mInputEvents == nullptr)
        return 0;
    mInputEvents->clear ();

    for (int32 i = 0; i < events->numEvents; i++)
    {
        VstMidiEvent* e = (VstMidiEvent*)events->events[i];
        if (e->type == kVstMidiType)
        {
            auto* midiEvent = (VstMidiEvent*)e;
            Event toAdd = {0, midiEvent->deltaFrames, 0};
            processMidiEvent (toAdd, &midiEvent->midiData[0],
                             midiEvent->flags , midiEvent->noteLength,
                             midiEvent->noteOffVelocity * kMidiScaler, midiEvent->detune, midiEvent->byteSize);
        }
        //--- -----------------------------
   /*     else
        {

            Event toAdd = {0, e->deltaFrames};
            VstMidiSysexEvent& src = *(VstMidiSysexEvent*)e;
            toAdd.type = Event::kDataEvent;
            toAdd.data.type = DataEvent::kMidiSysEx;
            toAdd.data.size = src.dumpBytes;
            toAdd.data.bytes = (uint8*)src.sysexDump;
            mInputEvents->addEvent (toAdd);
        }
*/
    }

    return 0;
}
/*
void MyVst2Wrapper::setupBuses ()
{
    BaseWrapper::setupBuses ();

    if (mHasEventOutputBuses)
    {
        if (mVst2OutputEvents == nullptr)
            mVst2OutputEvents = new Vst2MidiEventQueue (kMaxEvents);
    }
    else
    {
        if (mVst2OutputEvents)
        {
            delete mVst2OutputEvents;
            mVst2OutputEvents = nullptr;
        }
    }
}
*/

bool MyVst2Wrapper::pinIndexToBusChannel (BusDirection dir, int pinIndex, int32& busIndex, int32& busChannel)
{
    AudioBusBuffers* busBuffers = dir == kInput ? mProcessData.inputs : mProcessData.outputs;
    int32 busCount = dir == kInput ? mProcessData.numInputs : mProcessData.numOutputs;
    uint64 mainBusFlags = dir == kInput ? mMainAudioInputBuses : mMainAudioOutputBuses;

    int32 sourceIndex = 0;
    for (busIndex = 0; busIndex < busCount; busIndex++)
    {
        AudioBusBuffers& buffers = busBuffers[busIndex];
        if (mainBusFlags & (uint64 (1) << busIndex))
        {
            for (busChannel = 0; busChannel < buffers.numChannels; busChannel++)
            {
                if (pinIndex == sourceIndex)
                {
                    return true;
                }
                sourceIndex++;
            }
        }
    }
    return false;
}


/*
int MyVst2Wrapper::processEvents(VstEvents* events)
{
   VstBoardProcessor* proc = 0; //static_cast<VstBoardProcessor*>(mProcessor);
   if(!proc)
       return 0;

   return proc->processEvents(events);
}
*/
//AudioEffect* MyVst2Wrapper::crt (IPluginFactory* factory, const TUID vst3ComponentID, VstInt32 vst2ID, audioMasterCallback audioMaster)
//{
//    if (!factory)
//        return 0;

//    SVST3Config conf;
//    conf.factory = factory;
//    conf.processor = nullptr;
//    conf.controller = nullptr;
//    conf.vst3ComponentID = vst3ComponentID;

//    Vst::IAudioProcessor* processor = 0;
//    FReleaser factoryReleaser (factory);

//    factory->createInstance (vst3ComponentID, Vst::IAudioProcessor::iid, (void**)&processor);
//    if (processor)
//    {
//        Vst::IEditController* controller = 0;
//        if (processor->queryInterface (Vst::IEditController::iid, (void**)&controller) != kResultTrue)
//        {
//            FUnknownPtr<Vst::IComponent> component (processor);
//            if (component)
//            {
//                FUID editorCID;
//                if (component->getControllerClassId (editorCID) == kResultTrue)
//                {
//                    factory->createInstance (editorCID, Vst::IEditController::iid, (void**)&controller);
//                }
//            }
//        }

//        MyVst2Wrapper* wrapper = new MyVst2Wrapper (conf, audioMaster, vst2ID);
//        if (wrapper->init () == false)
//        {
//            delete wrapper;
//            return 0;
//        }

//        FUnknownPtr<IPluginFactory2> factory2 (factory);
//        if (factory2)
//        {
//            PFactoryInfo factoryInfo;
//            if (factory2->getFactoryInfo (&factoryInfo) == kResultTrue)
//                wrapper->setVendorName (factoryInfo.vendor);

//            for (int32 i = 0; i < factory2->countClasses (); i++)
//            {
//                Steinberg::PClassInfo2 classInfo2;
//                if (factory2->getClassInfo2 (i, &classInfo2) == Steinberg::kResultTrue)
//                {
//                    if (memcmp (classInfo2.cid, vst3ComponentID, sizeof (TUID)) == 0)
//                    {
//                        wrapper->s etSubCategories (classInfo2.subCategories);
//                        wrapper->setEffectName (classInfo2.name);

//                        if (classInfo2.vendor[0] != 0)
//                            wrapper->setVendorName (classInfo2.vendor);

//                        break;
//                    }
//                }
//            }
//        }

//        return wrapper;
//    }

//    return 0;
//}


/*

inline void MyVst2Wrapper::doProcess (int sampleFrames)
{
   if (mProcessor)
   {
       mProcessData.numSamples = sampleFrames;

//        if (processing == false)
//            startProcess ();

       mProcessData.inputEvents = mInputEvents;
       mProcessData.outputEvents = mOutputEvents;

       setupProcessTimeInfo ();
       setEventPPQPositions ();

       mInputTransfer.transferChangesTo (mInputChanges);

       mProcessData.inputParameterChanges = &mInputChanges;
       mProcessData.outputParameterChanges = &mOutputChanges;
       mOutputChanges.clearQueue ();

       // vst 3 process call
       mProcessor->process (mProcessData);

       mOutputTransfer.transferChangesFrom (mOutputChanges);
       processOutputEvents ();

       // clear input parameters and events
       mInputChanges.clearQueue ();
       if (mInputEvents)
           mInputEvents->clear ();
   }
}

void MyVst2Wrapper::processOutputEvents ()
{
   VstBoardProcessor* proc = 0;//static_cast<VstBoardProcessor*>(mProcessor);
   if(!proc)
       return;

   if(proc->processOutputEvents())
       sendVstEventsToHost(proc->getEvents());
}

void MyVst2Wrapper::onTimer (Timer* timer)
{
   if (mController)
   {
       Vst::ParamID id;
       Vst::ParamValue value;
       int32 sampleOffset;

       while (mOutputTransfer.getNextChange (id, value, sampleOffset))
       {
           setParameterAutomated(id,value);
           mController->setParamNormalized (id, value);
       }
       while (mGuiTransfer.getNextChange (id, value, sampleOffset))
       {
           mController->setParamNormalized (id, value);
       }
   }
}
*/
