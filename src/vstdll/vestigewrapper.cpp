//------------------------------------------------------------------------
// Flags       : clang-format auto
// Project     : VST SDK
//
// Category    : Helpers
// Filename    : public.sdk/source/vst/aaxwrapper/aaxwrapper.cpp
// Created by  : Steinberg, 08/2017
// Description : VST 3 -> AAX Wrapper
//
//-----------------------------------------------------------------------------
// LICENSE
// (c) 2024, Steinberg Media Technologies GmbH, All Rights Reserved
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
//#    Copyright 2010-2025 Raphaël François
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

#include "../common/globals.h"
#include "base/thread/include/fcondition.h"
#include "pluginterfaces/base/funknownimpl.h"
#include "VestigeWrapper.h"
#include "vstboardprocessor.h"
#include "pluginterfaces/vst/ivstmidicontrollers.h"

#include "gui.h"

#include "ids.h"

using namespace Steinberg;
using namespace Steinberg::Vst;
using namespace Steinberg::Base::Thread;

class VestigeEditorWrapper : public BaseEditorWrapper
{
public:
    VestigeEditorWrapper (VestigeWrapper* wrapper, IEditController* controller);
    bool getVstRect (VstRect** rect) ;
    void open(HWND w);
    VstRect mRect;
    VestigeWrapper* mWrapper;
};

VestigeEditorWrapper::VestigeEditorWrapper (VestigeWrapper* wrapper, IEditController* controller)
    : BaseEditorWrapper (controller), mWrapper(wrapper)
{
}

void VestigeEditorWrapper::open(HWND w)
{
    createView();
    _open(w);
}

bool VestigeEditorWrapper::getVstRect (VstRect** rect)
{
    createView();

    MainWindow* win = dynamic_cast<Gui*>(mView.get())->GetWindow();
    if(win) {

        mRect.left = (short)win->pos().x();
        mRect.top = (short)win->pos().y();
        mRect.right = (short) (win->pos().x() + win->size().width());
        mRect.bottom = (short) (win->pos().y() + win->size().height());

        *rect = &mRect;
        return true;
    }
    *rect = nullptr;
    return false;
}

intptr_t static_dispatcher(AEffect* pEffect, int opCode, int index, intptr_t value, void* ptr, float opt)
{
    VestigeWrapper* e=(VestigeWrapper*)pEffect->ptr3;
    return e->__dispatcher( opCode,  index,  value,  ptr,  opt);
}

void static_setParameter(AEffect* pEffect, int param, float value)
{
    VestigeWrapper* e=(VestigeWrapper*)pEffect->ptr3;
    e->__setParameter(param, value);
}

float static_getParameter(AEffect* pEffect, int param)
{
    VestigeWrapper* e=(VestigeWrapper*)pEffect->ptr3;
    return e->__getParameter(param);
}

void static_processReplacing (AEffect* pEffect, float** inputs, float** outputs, int sampleFrames)
{
    VestigeWrapper* e=(VestigeWrapper*)pEffect->ptr3;
    e->__processReplacing( inputs,  outputs,  sampleFrames);

}
void static_process(AEffect* pEffect, float** inputs, float** outputs, int sampleFrames)
{

}


tresult PLUGIN_API VestigeWrapper::getName (String128 name)
{
    String str ("VestigeWrapper");
    str.copyTo16 (name, 0, 127);
    return kResultTrue;
}

void VestigeWrapper::setupProcessTimeInfo ()
{
    mProcessData.processContext = &mProcessContext;
/*
    if (AAX_ITransport* transport = mAAXParams->Transport ())
    {
        int64_t splPos, ppqPos, loopStart, loopEnd;
        bool playing, looping;
        if (transport->GetCurrentNativeSampleLocation (&splPos) == AAX_SUCCESS)
            mProcessContext.projectTimeSamples = (TSamples)splPos;

        if (transport->GetCurrentTickPosition (&ppqPos) == AAX_SUCCESS)
        {
            mProcessContext.projectTimeMusic = (double)ppqPos / 960000.0;
            mProcessContext.state |= ProcessContext::kProjectTimeMusicValid;
        }
        else
            mProcessContext.projectTimeMusic = 0;

        if (transport->GetCurrentTempo (&mProcessContext.tempo) == AAX_SUCCESS)
            mProcessContext.state |= ProcessContext::kTempoValid;

        if (transport->GetCurrentLoopPosition (&looping, &loopStart, &loopEnd) == AAX_SUCCESS)
        {
            mProcessContext.cycleStartMusic = (double)loopStart / 960000.0;
            mProcessContext.cycleEndMusic = (double)loopEnd / 960000.0;
            mProcessContext.state |= ProcessContext::kCycleValid;
            if (looping)
                mProcessContext.state |= ProcessContext::kCycleActive;
        }

        if (transport->IsTransportPlaying (&playing) == AAX_SUCCESS)
        {
            mProcessContext.state |= (playing ? ProcessContext::kPlaying : 0);
        }

        // workaround ppqPos not updating for every 2nd audio blocks @ 96 kHz (and more for higher
        // frequencies)
        //  and while the UI is frozen, e.g. during save
        static const int32 playflags = ProcessContext::kPlaying |
                                       ProcessContext::kProjectTimeMusicValid |
                                       ProcessContext::kTempoValid;

        if ((playflags & mProcessContext.state) == playflags && mSampleRate != 0)
        {
            TQuarterNotes ppq = mProcessContext.projectTimeMusic;
            if (ppq == mLastPpqPos && mLastPpqPos != 0 && mNextPpqPos != 0)
            {
                TQuarterNotes nextppq = mNextPpqPos;
                if (mProcessContext.state & ProcessContext::kCycleActive)
                    if (nextppq >= mProcessContext.cycleEndMusic)
                        nextppq += mProcessContext.cycleStartMusic - mProcessContext.cycleEndMusic;

                mProcessContext.projectTimeMusic = nextppq;
            }
            mLastPpqPos = ppq;
            mNextPpqPos = mProcessContext.projectTimeMusic +
                          mProcessContext.tempo / 60 * mProcessData.numSamples / mSampleRate;
        }
        else
        {
            mLastPpqPos = mNextPpqPos = 0;
        }

        int32_t num, den;
        if (transport->GetCurrentMeter (&num, &den) == AAX_SUCCESS)
        {
            mProcessContext.timeSigNumerator = num;
            mProcessContext.timeSigDenominator = den;
            mProcessContext.state |= ProcessContext::kTimeSigValid;
        }
        else
            mProcessContext.timeSigNumerator = mProcessContext.timeSigDenominator = 4;

        AAX_EFrameRate frameRate;
        int32_t offset;
        if (transport->GetTimeCodeInfo (&frameRate, &offset) == AAX_SUCCESS)
        {
            mProcessContext.state |= ProcessContext::kSmpteValid;
            mProcessContext.smpteOffsetSubframes = offset;
            switch (frameRate)
            {
            case AAX_eFrameRate_24Frame: mProcessContext.frameRate.framesPerSecond = 24; break;
            case AAX_eFrameRate_25Frame: mProcessContext.frameRate.framesPerSecond = 25; break;
            case AAX_eFrameRate_2997NonDrop:
                mProcessContext.frameRate.framesPerSecond = 30;
                mProcessContext.frameRate.flags = FrameRate::kPullDownRate;
                break;
            case AAX_eFrameRate_2997DropFrame:
                mProcessContext.frameRate.framesPerSecond = 30;
                mProcessContext.frameRate.flags =
                    FrameRate::kDropRate | FrameRate::kPullDownRate;
                break;
            case AAX_eFrameRate_30NonDrop:
                mProcessContext.frameRate.framesPerSecond = 30;
                break;
            case AAX_eFrameRate_30DropFrame:
                mProcessContext.frameRate.framesPerSecond = 30;
                mProcessContext.frameRate.flags = FrameRate::kDropRate;
                break;
            case AAX_eFrameRate_23976:
                mProcessContext.frameRate.framesPerSecond = 24;
                mProcessContext.frameRate.flags = FrameRate::kPullDownRate;
                break;
            case AAX_eFrameRate_Undeclared:
            default: mProcessContext.state &= ~ProcessContext::kSmpteValid;
            }
        }

        mProcessData.processContext = &mProcessContext;
    }
    else
        mProcessData.processContext = nullptr;
*/
}


VestigeWrapper::VestigeWrapper(Steinberg::Vst::BaseWrapper::SVST3Config config, audioMasterCallback audioMaster)
: BaseWrapper (config)
{


    Effect.magic=kEffectMagic;
    Effect.numPrograms=0;
    Effect.numParams=0;
    Effect.numInputs=2;
    Effect.numOutputs=2;
    Effect.flags= effFlagsHasEditor | effFlagsCanReplacing | effFlagsProgramChunks;
    if(mVst3EffectClassID == VstBoardInstProcessorUID) {
        Effect.flags |= effFlagsIsSynth;
    }
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

    masterCallback = audioMaster;
}

//from aaxwrapper
VestigeWrapper* VestigeWrapper::create (IPluginFactory* factory, const TUID vst3ComponentID, audioMasterCallback audioMaster)
{
    if (!factory)
        return nullptr;

    BaseWrapper::SVST3Config config;
    config.processor = nullptr;
    config.factory = factory;

    FReleaser factoryReleaser (factory);

    factory->createInstance (vst3ComponentID, IAudioProcessor::iid, (void**)&config.processor);
    if (!config.processor)
        return nullptr;

    config.controller = nullptr;
    if (config.processor->queryInterface (IEditController::iid, (void**)&config.controller) != kResultTrue)
    {
        if (auto component = U::cast<IComponent> (config.processor))
        {
            TUID editorCID;
            if (component->getControllerClassId (editorCID) == kResultTrue)
            {
                factory->createInstance (editorCID, IEditController::iid,
                                        (void**)&config.controller);
            }
        }
    }

    config.vst3ComponentID = FUID::fromTUID (vst3ComponentID);

    auto* wrapper = new VestigeWrapper (config, audioMaster);
    if (wrapper->init () == false )
    {
        wrapper->release ();
        return nullptr;
    }
    wrapper->setupBuses (); // again to adjust to changes done by setupBusArrangements

    if (auto factory2 = U::cast<IPluginFactory2> (factory))
    {
        PFactoryInfo factoryInfo;
        if (factory2->getFactoryInfo (&factoryInfo) == kResultTrue) {
            wrapper->setVendorName (factoryInfo.vendor);
        }

        for (int32 i = 0; i < factory2->countClasses (); i++)
        {
            Steinberg::PClassInfo2 classInfo2;
            if (factory2->getClassInfo2 (i, &classInfo2) == Steinberg::kResultTrue)
            {
                if (memcmp (classInfo2.cid, vst3ComponentID, sizeof (TUID)) == 0)
                {
                    wrapper->setSubCategories (classInfo2.subCategories);
                    wrapper->setEffectName (classInfo2.name);

                    if (classInfo2.vendor[0] != 0) {
                        wrapper->setVendorName (classInfo2.vendor);
                    }
                    break;
                }
            }
        }
    }

    return wrapper;
}



bool VestigeWrapper::_sizeWindow (int32 width, int32 height)
{
    /*
    HLOG (HAPI, "%s(width=%x, height=%x)", __FUNCTION__, width, height);

    AAX_ASSERT (mainThread == getCurrentThread ());

    if (!mAAXGUI)
        return false;

    AAX_Point size (height, width);
    return mAAXGUI->setWindowSize (size);
    */
    return true;
}


//------------------------------------------------------------------------
struct VestigeWrapper::GetChunkMessage : public FCondition
{
    void* mData = nullptr;
    int32 mDataSize = 0;
    int32 mResult = 0;
};

//------------------------------------------------------------------------
int32 VestigeWrapper::_getChunk (void** data, bool isPreset)
{
    /*
    if (mWantsSetChunk)
    {
        // isPreset is always false for AAX, so we can ignore it
        *data = mChunk.getData ();
        return static_cast<int32> (mChunk.getSize ());
    }
    if (mainThread == getCurrentThread ())
        return BaseWrapper::_getChunk (data, isPreset);

    auto* msg = NEW GetChunkMessage;
    {
        FGuard guard (msgQueueLock);
        msgQueue.push_back (msg);
    }
    msg->wait ();

    *data = msg->mData;
    return msg->mResult;
*/
    return 0;
}

//------------------------------------------------------------------------
int32 VestigeWrapper::_setChunk (void* data, int32 byteSize, bool isPreset)
{
    /*
    if (mainThread == getCurrentThread ())
        return BaseWrapper::_setChunk (data, byteSize, isPreset);

    FGuard guard (msgQueueLock);
    mChunk.setSize (byteSize);
    memcpy (mChunk.getData (), data, static_cast<size_t> (byteSize));
    mWantsSetChunk = true;
    mWantsSetChunkIsPreset = isPreset;
*/
    return 0;
}


tresult PLUGIN_API VestigeWrapper::beginEdit (ParamID tag)
{
    return kResultTrue;
}

tresult PLUGIN_API VestigeWrapper::performEdit (ParamID tag, ParamValue valueNormalized)
{
    mInputTransfer.addChange (tag, valueNormalized, 0);
    return kResultTrue;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API VestigeWrapper::endEdit (ParamID tag)
{
    return kResultTrue;
}


bool VestigeWrapper::init ()
{
    bool res = BaseWrapper::init ();

    if (mController && BaseEditorWrapper::hasEditor (mController))
        _setEditor (NEW VestigeEditorWrapper (this, mController));

    return res;

}



// String strProdName(APP_NAME);
// String strProdVendor(APP_VENDOR);


intptr_t VestigeWrapper::__dispatcher( int opCode, int index, intptr_t value, void* ptr, float opt)
{
    // LOG("opcode:" << opCode)

    switch(opCode) {
    case effGetPlugCategory: //35
        if( memcmp(mSubCategories,"Instrument",10) == 0 ) {
            return kPlugCategSynth;
        }
        return kPlugCategEffect;

    case effGetVstVersion: //58
        return 1;
    case effSetSampleRate: //10
        _setSampleRate(value);
        return 1;
    case effSetBlockSize: //11
        _setBlockSize(value);
        return 1;
    case effCanDo: //51
        return 0;
    case effMainsChanged: //12
        if(value) {
            _resume();
        } else {
            _suspend();
        }
        return 0;
    case effBeginSetProgram: //67
        return 0;
    case effSetProgram: //2
        return 0;
    case effEndSetProgram: //68
        return 0;
    case effGetVendorString:
        strcpy_s((char*)ptr,127,mVendor);
        // strProdVendor.copyTo8((char8*)ptr, 0, 127);
        return 1;
    case effGetProductString: //48
        strcpy_s((char*)ptr,127,mName);
        // strProdName.copyTo8((char8*)ptr, 0, 127);
        return 1;
    case effStartProcess: //71
        BaseWrapper::_startProcess ();
        return 1;
    case effStopProcess: //72
        BaseWrapper::_stopProcess ();
        return 1;
    case effEditGetRect: //13
        if(!mEditor) return 0;
        static_cast<VestigeEditorWrapper*>(mEditor.get())->getVstRect( (VstRect**)ptr );
        return 1;
    case effEditOpen: //14
        if(!mEditor) return 0;
        static_cast<VestigeEditorWrapper*>(mEditor.get())->open( (HWND)ptr );
        return 1;
    case effEditClose: //15
        if(!mEditor) return 0;
        static_cast<VestigeEditorWrapper*>(mEditor.get())->_close();
        return 1;
    case effEditIdle : //19
        // LOG("opcode:" << opCode)
        return 1;
    case effEditTop: //20
        return 0;
    case effProcessEvents: //25
        UpdateTime();
        return processEvents( (VstEvents*)ptr );
    case effIdle: //53
        return 0;
    }

    LOG("opcode:" << opCode)

    return 0;
}

void VestigeWrapper::UpdateTime()
{
    intptr_t ret = masterCallback(&Effect, audioMasterGetTime, 0, (kVstTransportPlaying | kVstTempoValid | kVstPpqPosValid) ,NULL, 0.0f);
    VstTimeInfo* time = (VstTimeInfo*)ret;

    mProcessContext.sampleRate = time->sampleRate;
    mProcessContext.projectTimeSamples = time->samplePos;

    mProcessContext.tempo = time->tempo;
    mProcessContext.state = ProcessContext::kTempoValid;

    if(time->timeSigNumerator==0 || time->timeSigDenominator==0) {
        mProcessContext.timeSigNumerator = 4;
        mProcessContext.timeSigDenominator = 4;
    } else {
        mProcessContext.timeSigNumerator = time->timeSigNumerator;
        mProcessContext.timeSigDenominator = time->timeSigDenominator;
    }
    mProcessContext.state |= ProcessContext::kTimeSigValid;

    // double nbSec = mProcessContext.projectTimeSamples / mProcessContext.sampleRate;
    // mProcessContext.projectTimeMusic = nbSec / 60 * mProcessContext.tempo;
    // mProcessContext.state |= ProcessContext::kProjectTimeMusicValid;
    mProcessContext.projectTimeMusic = time->ppqPos;

    float barLengthq = (float)(4*mProcessContext.timeSigNumerator)/mProcessContext.timeSigDenominator;
    int32 nbBars = mProcessContext.projectTimeMusic/barLengthq;
    mProcessContext.barPositionMusic = (TQuarterNotes)barLengthq*(TQuarterNotes)nbBars;
    mProcessContext.state |= ProcessContext::kBarPositionValid;

    // VstBoardProcessor * proc = static_cast<VstBoardProcessor*>(mProcessor.get());
    // proc->SetTempo( time->tempo, time->timeSigNumerator, time->timeSigDenominator );
    // proc->SetSampleRate( time->sampleRate );
}

float VestigeWrapper::__getParameter(int index)
{
    return _getParameter(index);
}

void VestigeWrapper::__setParameter(int index, float value)
{
    if (!mController)
        return;// 0.f;

    if (index < static_cast<int32> (mParameterMap.size ()))
    {
        ParamID id = mParameterMap.at (index).vst3ID;
        //return (float)mController->getParamNormalized (id);
        addParameterChange (id, (ParamValue)value, 0);
    }

}

int VestigeWrapper::startProcess ()
{
    BaseWrapper::_startProcess ();
    return 0;
}

int VestigeWrapper::stopProcess ()
{
    BaseWrapper::_stopProcess ();
    return 0;
}

void VestigeWrapper::__processReplacing (float** inputs, float** outputs, int sampleFrames)
{
    _processReplacing (inputs, outputs, sampleFrames);
}

//from basewrapper
void VestigeWrapper::processMidiEvent (Event& toAdd, char* midiData, bool isLive, int32 noteLength, float noteOffVelocity, float detune, int size)
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

//TODO
int VestigeWrapper::processEvents (VstEvents* events)
{

    for (int32 i = 0; i < events->numEvents; i++)
    {
        VstMidiEvent* ev = (VstMidiEvent*)events->events[i];

        // skip note-on events if bypassed to reduce processor load for instruments,
        // but let everything else through to avoid hanging notes
        // if (mSimulateBypass && mBypass)
            // if ((buf.mData[0] & Vst::kStatusMask) == Vst::kNoteOn && buf.mData[2] != 0)
                // continue;

        Event toAdd = {0, ev->deltaFrames, 0};
        // bool isLive = buf.mIsImmediate || buf.mTimestamp == 0;
        // processMidiEvent (toAdd, (char*)&buf.mData[0], isLive);
        processMidiEvent (toAdd, &ev->midiData[0] , true, ev->noteLength, ev->noteOffVelocity, ev->detune, ev->byteSize);

    }

    return 0;
}

