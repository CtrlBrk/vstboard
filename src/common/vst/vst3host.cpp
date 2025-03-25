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
#include "vst3host.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstunits.h"
#include "pluginterfaces/vst/ivstmessage.h"
#include "public.sdk/source/vst/utility/stringconvert.h"

using namespace Steinberg;
using namespace Vst;

Vst3Host::Vst3Host(QObject *parent) :
    QObject(parent)
    ,IHostApplication()
    ,IPlugInterfaceSupport()
{
    FUNKNOWN_CTOR

    addPlugInterfaceSupported (IComponent::iid);
    addPlugInterfaceSupported (IAudioProcessor::iid);
    addPlugInterfaceSupported (IEditController::iid);

    currentBar=0;
    loopLenght=4;

    processContext.state =
            ProcessContext::kPlaying |
            ProcessContext::kCycleActive |
            ProcessContext::kSystemTimeValid |
            ProcessContext::kContTimeValid |
            ProcessContext::kProjectTimeMusicValid |
            ProcessContext::kBarPositionValid |
            ProcessContext::kCycleValid |
            ProcessContext::kTempoValid |
            ProcessContext::kTimeSigValid;
    processContext.sampleRate = 44100;
    processContext.projectTimeSamples = 0;

    processContext.systemTime = 0;
    processContext.continousTimeSamples = 0;
    processContext.projectTimeMusic = 0;
    processContext.barPositionMusic = 0;
    processContext.cycleStartMusic = 0;
    processContext.cycleEndMusic = 16.L;
    processContext.tempo = 120;
    processContext.timeSigNumerator = 4;
    processContext.timeSigDenominator = 4;
//    processContext.chord
//    processContext.smpteOffsetSubframes
//    processContext.frameRate
//    processContext.samplesToNextClock


    connect(parent,SIGNAL(SampleRateChanged(float)),
            this,SLOT(SetSampleRate(float)));
}

tresult PLUGIN_API Vst3Host::getName (String128 name)
{
    StringConvert::convert ("VstBoard", name, 127);
    return kResultTrue;
}

tresult PLUGIN_API Vst3Host::createInstance (TUID cid, TUID _iid, void** obj)
{
    if (FUnknownPrivate::iidEqual (cid, IMessage::iid) &&
        FUnknownPrivate::iidEqual (_iid, IMessage::iid))
    {
        *obj = new HostMessage;
        return kResultTrue;
    }
    if (FUnknownPrivate::iidEqual (cid, IAttributeList::iid) &&
        FUnknownPrivate::iidEqual (_iid, IAttributeList::iid))
    {
        if (auto al = HostAttributeList::make ())
        {
            *obj = al.take ();
            return kResultTrue;
        }
        return kOutOfMemory;
    }
    *obj = nullptr;
    return kResultFalse;
}

tresult PLUGIN_API Vst3Host::isPlugInterfaceSupported (const TUID _iid)
{
    auto uid = FUID::fromTUID (_iid);
    if (std::find (mFUIDArray.begin (), mFUIDArray.end (), uid) != mFUIDArray.end ())
        return kResultTrue;
    return kResultFalse;
}

void Vst3Host::addPlugInterfaceSupported (const TUID _iid)
{
    mFUIDArray.push_back (FUID::fromTUID (_iid));
}

bool Vst3Host::removePlugInterfaceSupported (const TUID _iid)
{
    auto uid = FUID::fromTUID (_iid);
    auto it = std::find (mFUIDArray.begin (), mFUIDArray.end (), uid);
    if (it  == mFUIDArray.end ())
        return false;
    mFUIDArray.erase (it);
    return true;
}

//IMPLEMENT_FUNKNOWN_METHODS (Vst3Host, IHostApplication, IHostApplication::iid)


PlugInterfaceSupport* Vst3Host::getPlugInterfaceSupport () const {
    return mPlugInterfaceSupport;
}


tresult PLUGIN_API Vst3Host::queryInterface (const char* _iid, void** obj)
{
    QUERY_INTERFACE (_iid, obj, FUnknown::iid, IHostApplication)
    QUERY_INTERFACE (_iid, obj, IHostApplication::iid, IHostApplication)
    //QUERY_INTERFACE (_iid, obj, IComponentHandler::iid, IComponentHandler)
    QUERY_INTERFACE (_iid, obj, IPlugInterfaceSupport::iid, IPlugInterfaceSupport)

    if (mPlugInterfaceSupport && mPlugInterfaceSupport->queryInterface (_iid, obj) == kResultTrue)
        return kResultOk;

    *obj = nullptr;
    return kResultFalse;
}

//-----------------------------------------------------------------------------
uint32 PLUGIN_API Vst3Host::addRef ()
{
    return 1;
}

//-----------------------------------------------------------------------------
uint32 PLUGIN_API Vst3Host::release ()
{
    return 1;
}

void Vst3Host::UpdateTime(long buffSize, float sampleRate)
{
    processContext.sampleRate = sampleRate;
    processContext.projectTimeSamples += buffSize;
    processContext.continousTimeSamples += buffSize;

    TQuarterNotes barLengthq = (TQuarterNotes)(processContext.timeSigNumerator*4) /processContext.timeSigDenominator;
    processContext.cycleStartMusic = 0;
    processContext.cycleEndMusic = barLengthq*(TQuarterNotes)loopLenght;

    TQuarterNotes dPos = processContext.continousTimeSamples / processContext.sampleRate;
    processContext.projectTimeMusic = dPos * processContext.tempo / 60.L;

    if(processContext.projectTimeMusic > processContext.cycleEndMusic) {

        processContext.projectTimeMusic -= processContext.cycleEndMusic;
        dPos = processContext.projectTimeMusic / processContext.tempo * 60.L;
        processContext.projectTimeSamples = dPos * processContext.sampleRate;
//        double dOffsetInSecond = dPos - floor(dPos);
//        vstTimeInfo.smpteOffset = (long)(dOffsetInSecond * fSmpteDiv[vstTimeInfo.smpteFrameRate] * 80.L);
    }

    //start of last bar
    currentBar = floor(processContext.projectTimeMusic/barLengthq);
    processContext.barPositionMusic = barLengthq*(TQuarterNotes)currentBar;

    std::chrono::time_point<std::chrono::system_clock> timestamp = std::chrono::system_clock::now();
    processContext.systemTime = std::chrono::duration_cast<std::chrono::nanoseconds>(timestamp.time_since_epoch()).count();
}

float Vst3Host::GetCurrentBarTic()
{
    float barLengthq = (float)(4*processContext.timeSigNumerator)/processContext.timeSigDenominator;
    float step = floor((processContext.projectTimeMusic-processContext.barPositionMusic)*processContext.timeSigDenominator/4);
    float total = ((barLengthq*processContext.timeSigDenominator/4)-1);

    barTic.store(step / total);
    return step / total;

}

void Vst3Host::SetTimeInfo(const ProcessContext *info) {
    ProcessContext newCxt = *info;

    //keep tempo if not provided
    if( (newCxt.state & ProcessContext::kTempoValid)==0 ) {
        newCxt.tempo = processContext.tempo;
        newCxt.state |= ProcessContext::kTempoValid;
    }

    //keep sign if not provided
    if( (newCxt.state & ProcessContext::kTimeSigValid)==0 ) {
        newCxt.timeSigNumerator = processContext.timeSigNumerator;
        newCxt.timeSigDenominator = processContext.timeSigDenominator;
        newCxt.state |= ProcessContext::kTimeSigValid;
    }

    //get projectTimeMusic if not provided
    if( (newCxt.state & ProcessContext::kProjectTimeMusicValid)==0 ) {
        double nbSec = newCxt.projectTimeSamples / newCxt.sampleRate;
        newCxt.projectTimeMusic = nbSec / 60 * newCxt.tempo;
        newCxt.state |= ProcessContext::kProjectTimeMusicValid;
    }

    if( (newCxt.state & ProcessContext::kBarPositionValid)==0 ) {
        float barLengthq = (float)(4*newCxt.timeSigNumerator)/newCxt.timeSigDenominator;
        int32 nbBars = newCxt.projectTimeMusic/barLengthq;
        newCxt.barPositionMusic = (TQuarterNotes)barLengthq*(TQuarterNotes)nbBars;
        newCxt.state |= ProcessContext::kBarPositionValid;
    }

    processContext = newCxt;

    GetCurrentBarTic();
}

//void Vst3Host::GetTimeInfo(VstTimeInfo *vst2timeInfo) {
//    const uint32 portableFlags =
//        ProcessContext::kPlaying |
//        ProcessContext::kCycleActive |
//        ProcessContext::kRecording |
//        ProcessContext::kSystemTimeValid |
//        ProcessContext::kProjectTimeMusicValid |
//        ProcessContext::kBarPositionValid |
//        ProcessContext::kCycleValid |
//        ProcessContext::kTempoValid |
//        ProcessContext::kTimeSigValid |
//        ProcessContext::kSmpteValid |
//        ProcessContext::kClockValid;

//    vst2timeInfo->flags = processContext.state & portableFlags;
//    vst2timeInfo->sampleRate = processContext.sampleRate;
//    vst2timeInfo->samplePos = processContext.projectTimeSamples;

//    if (processContext.state & ProcessContext::kSystemTimeValid)
//        vst2timeInfo->nanoSeconds = processContext.systemTime;

//    if (processContext.state & ProcessContext::kProjectTimeMusicValid)
//        vst2timeInfo->ppqPos = processContext.projectTimeMusic;

//    if (processContext.state & ProcessContext::kBarPositionValid)
//        vst2timeInfo->barStartPos = processContext.barPositionMusic;

//    if (processContext.state & ProcessContext::kCycleValid) {
//        vst2timeInfo->cycleStartPos = processContext.cycleStartMusic;
//        vst2timeInfo->cycleEndPos = processContext.cycleEndMusic;
//    }

//    if (processContext.state & ProcessContext::kTempoValid)
//        vst2timeInfo->tempo = processContext.tempo;

//    if (processContext.state & ProcessContext::kTimeSigValid)
//    {
//        vst2timeInfo->timeSigNumerator = processContext.timeSigNumerator;
//        vst2timeInfo->timeSigDenominator = processContext.timeSigDenominator;
//    }
//}

void Vst3Host::SetTempo(int tempo, int sign1, int sign2)
{
    processContext.tempo = tempo;
    if(sign1!=0)
        processContext.timeSigNumerator = sign1;
    if(sign2!=0)
        processContext.timeSigDenominator = sign2;

    processContext.state |= ProcessContext::kTempoValid;
    processContext.state |= ProcessContext::kTimeSigValid;
}

void Vst3Host::GetTempo(int &tempo, int &sign1, int &sign2)
{
    if(processContext.state &  ProcessContext::kTempoValid)
        tempo=processContext.tempo;

    if(processContext.state & ProcessContext::kTimeSigValid) {
        sign1=processContext.timeSigNumerator;
        sign2=processContext.timeSigDenominator;
    }
}

void Vst3Host::SetSampleRate(float rate)
{
    processContext.sampleRate = rate;
}




IMPLEMENT_FUNKNOWN_METHODS (HostMessage, IMessage, IMessage::iid)
//-----------------------------------------------------------------------------
HostMessage::HostMessage () {FUNKNOWN_CTOR}

//-----------------------------------------------------------------------------
HostMessage::~HostMessage () noexcept
{
    setMessageID (nullptr);
    FUNKNOWN_DTOR
}

//-----------------------------------------------------------------------------
const char* PLUGIN_API HostMessage::getMessageID ()
{
    return messageId;
}

//-----------------------------------------------------------------------------
void PLUGIN_API HostMessage::setMessageID (const char* mid)
{
    if (messageId)
        delete[] messageId;
    messageId = nullptr;
    if (mid)
    {
        size_t len = strlen (mid) + 1;
        messageId = new char[len];
        strcpy (messageId, mid);
    }
}

//-----------------------------------------------------------------------------
IAttributeList* PLUGIN_API HostMessage::getAttributes ()
{
    if (!attributeList)
        attributeList = HostAttributeList::make ();
    return attributeList;
}

struct HostAttributeList::Attribute
{
    enum class Type
    {
        kUninitialized,
        kInteger,
        kFloat,
        kString,
        kBinary
    };
    Attribute () = default;

    Attribute (int64 value) : type (Type::kInteger) { v.intValue = value; }
    Attribute (double value) : type (Type::kFloat) { v.floatValue = value; }
    /* size is in code unit (count of TChar) */
    Attribute (const TChar* value, uint32 sizeInCodeUnit)
        : size (sizeInCodeUnit), type (Type::kString)
    {
        v.stringValue = new TChar[sizeInCodeUnit];
        memcpy (v.stringValue, value, sizeInCodeUnit * sizeof (TChar));
    }
    Attribute (const void* value, uint32 sizeInBytes) : size (sizeInBytes), type (Type::kBinary)
    {
        v.binaryValue = new char[sizeInBytes];
        memcpy (v.binaryValue, value, sizeInBytes);
    }
    Attribute (Attribute&& o) { *this = std::move (o); }
    Attribute& operator= (Attribute&& o)
    {
        v = o.v;
        size = o.size;
        type = o.type;
        o.size = 0;
        o.type = Type::kUninitialized;
        o.v = {};
        return *this;
    }
    ~Attribute () noexcept
    {
        if (size)
            delete[] v.binaryValue;
    }

    int64 intValue () const { return v.intValue; }
    double floatValue () const { return v.floatValue; }
    /* sizeInCodeUnit is in code unit (count of TChar) */
    const TChar* stringValue (uint32& sizeInCodeUnit)
    {
        sizeInCodeUnit = size;
        return v.stringValue;
    }
    const void* binaryValue (uint32& sizeInBytes)
    {
        sizeInBytes = size;
        return v.binaryValue;
    }

    Type getType () const { return type; }

private:
    union v
    {
        int64 intValue;
        double floatValue;
        TChar* stringValue;
        char* binaryValue;
    } v;
    uint32 size {0};
    Type type {Type::kUninitialized};
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
IMPLEMENT_FUNKNOWN_METHODS (HostAttributeList, IAttributeList, IAttributeList::iid)

//-----------------------------------------------------------------------------
IPtr<IAttributeList> HostAttributeList::make ()
{
    return owned (new HostAttributeList);
}

//-----------------------------------------------------------------------------
HostAttributeList::HostAttributeList () {FUNKNOWN_CTOR}

//-----------------------------------------------------------------------------
HostAttributeList::~HostAttributeList () noexcept
    {
        FUNKNOWN_DTOR
    }

//-----------------------------------------------------------------------------
tresult PLUGIN_API HostAttributeList::setInt (AttrID aid, int64 value)
{
    if (!aid)
        return kInvalidArgument;
    list[aid] = Attribute (value);
    return kResultTrue;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API HostAttributeList::getInt (AttrID aid, int64& value)
{
    if (!aid)
        return kInvalidArgument;
    auto it = list.find (aid);
    if (it != list.end () && it->second.getType () == Attribute::Type::kInteger)
    {
        value = it->second.intValue ();
        return kResultTrue;
    }
    return kResultFalse;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API HostAttributeList::setFloat (AttrID aid, double value)
{
    if (!aid)
        return kInvalidArgument;
    list[aid] = Attribute (value);
    return kResultTrue;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API HostAttributeList::getFloat (AttrID aid, double& value)
{
    if (!aid)
        return kInvalidArgument;
    auto it = list.find (aid);
    if (it != list.end () && it->second.getType () == Attribute::Type::kFloat)
    {
        value = it->second.floatValue ();
        return kResultTrue;
    }
    return kResultFalse;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API HostAttributeList::setString (AttrID aid, const TChar* string)
{
    if (!aid)
        return kInvalidArgument;
    // + 1 for the null-terminate
    auto length = tstrlen (string) + 1;
    list[aid] = Attribute (string, length);
    return kResultTrue;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API HostAttributeList::getString (AttrID aid, TChar* string, uint32 sizeInBytes)
{
    if (!aid)
        return kInvalidArgument;
    auto it = list.find (aid);
    if (it != list.end () && it->second.getType () == Attribute::Type::kString)
    {
        uint32 sizeInCodeUnit = 0;
        const TChar* _string = it->second.stringValue (sizeInCodeUnit);
        memcpy (string, _string, std::min<uint32> (sizeInCodeUnit * sizeof (TChar), sizeInBytes));
        return kResultTrue;
    }
    return kResultFalse;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API HostAttributeList::setBinary (AttrID aid, const void* data, uint32 sizeInBytes)
{
    if (!aid)
        return kInvalidArgument;
    list[aid] = Attribute (data, sizeInBytes);
    return kResultTrue;
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API HostAttributeList::getBinary (AttrID aid, const void*& data, uint32& sizeInBytes)
{
    if (!aid)
        return kInvalidArgument;
    auto it = list.find (aid);
    if (it != list.end () && it->second.getType () == Attribute::Type::kBinary)
    {
        data = it->second.binaryValue (sizeInBytes);
        return kResultTrue;
    }
    sizeInBytes = 0;
    return kResultFalse;
}
