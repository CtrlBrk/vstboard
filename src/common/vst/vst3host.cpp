#include "vst3host.h"

using namespace Steinberg;

Vst3Host::Vst3Host() : QObject(), Vst::HostApplication()
{
    currentBar=0;
    loopLenght=4;

    processContext.state =
			Vst::ProcessContext::kPlaying |
            Vst::ProcessContext::kCycleActive |
            Vst::ProcessContext::kSystemTimeValid |
            Vst::ProcessContext::kContTimeValid |
            Vst::ProcessContext::kProjectTimeMusicValid |
            Vst::ProcessContext::kBarPositionValid |
            Vst::ProcessContext::kCycleValid |
            Vst::ProcessContext::kTempoValid |
            Vst::ProcessContext::kTimeSigValid;
    processContext.sampleRate = 44.1;
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
}

tresult PLUGIN_API Vst3Host::getName (Vst::String128 name)
{
    String str ("VstBoard");
    str.copyTo (name, 0, 127);
    return kResultTrue;
}

void Vst3Host::UpdateTime(long buffSize, float sampleRate)
{
    processContext.sampleRate = sampleRate;
    processContext.projectTimeSamples += buffSize;
    processContext.continousTimeSamples += buffSize;

	Vst::TQuarterNotes barLengthq = (Vst::TQuarterNotes)(processContext.timeSigNumerator*4) /processContext.timeSigDenominator;
    processContext.cycleStartMusic = 0;
    processContext.cycleEndMusic = barLengthq*(Vst::TQuarterNotes)loopLenght;

	Vst::TQuarterNotes dPos = processContext.continousTimeSamples / processContext.sampleRate;
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
    processContext.barPositionMusic = barLengthq*(Vst::TQuarterNotes)currentBar;
}

float Vst3Host::GetCurrentBarTic()
{
    float barLengthq = (float)(4*processContext.timeSigNumerator)/processContext.timeSigDenominator;
    float step = floor((processContext.projectTimeMusic-processContext.barPositionMusic)*processContext.timeSigDenominator/4);
    float total = ((barLengthq*processContext.timeSigDenominator/4)-1);
    return step / total;

//    if(processContext.state & Vst::ProcessContext::kTimeSigValid) {

//        if(processContext.state & Vst::ProcessContext::kProjectTimeMusicValid) {
//            int s = processContext.projectTimeMusic*processContext.timeSigDenominator/4;
//            float r = s % processContext.timeSigNumerator;
//            return r/(processContext.timeSigNumerator-1);
//        }
//    }
//    return .0f;
}

void Vst3Host::SetTimeInfo(const Vst::ProcessContext *info) {
    Vst::ProcessContext newCxt = *info;

    //keep tempo if not provided
    if( (newCxt.state & Vst::ProcessContext::kTempoValid)==0 ) {
        newCxt.tempo = processContext.tempo;
        newCxt.state |= Vst::ProcessContext::kTempoValid;
    }

    //keep sign if not provided
    if( (newCxt.state & Vst::ProcessContext::kTimeSigValid)==0 ) {
        newCxt.timeSigNumerator = processContext.timeSigNumerator;
        newCxt.timeSigDenominator = processContext.timeSigDenominator;
        newCxt.state |= Vst::ProcessContext::kTimeSigValid;
    }

    //get projectTimeMusic if not provided
    if( (newCxt.state & Vst::ProcessContext::kProjectTimeMusicValid)==0 ) {
        double nbSec = newCxt.projectTimeSamples / newCxt.sampleRate;
        newCxt.projectTimeMusic = nbSec / 60 * newCxt.tempo;
        newCxt.state |= Vst::ProcessContext::kProjectTimeMusicValid;
    }

    if( (newCxt.state & Vst::ProcessContext::kBarPositionValid)==0 ) {
        float barLengthq = (float)(4*newCxt.timeSigNumerator)/newCxt.timeSigDenominator;
        int32 nbBars = newCxt.projectTimeMusic/barLengthq;
        newCxt.barPositionMusic = (Vst::TQuarterNotes)barLengthq*(Vst::TQuarterNotes)nbBars;
        newCxt.state |= Vst::ProcessContext::kBarPositionValid;
    }

    processContext = newCxt;
}

void Vst3Host::GetTimeInfo(VstTimeInfo *vst2timeInfo) {
    const uint32 portableFlags =
        Vst::ProcessContext::kPlaying |
        Vst::ProcessContext::kCycleActive |
        Vst::ProcessContext::kRecording |
        Vst::ProcessContext::kSystemTimeValid |
        Vst::ProcessContext::kProjectTimeMusicValid |
        Vst::ProcessContext::kBarPositionValid |
        Vst::ProcessContext::kCycleValid |
        Vst::ProcessContext::kTempoValid |
        Vst::ProcessContext::kTimeSigValid |
        Vst::ProcessContext::kSmpteValid |
        Vst::ProcessContext::kClockValid;

    vst2timeInfo->flags = processContext.state & portableFlags;
    vst2timeInfo->sampleRate = processContext.sampleRate;
    vst2timeInfo->samplePos = processContext.projectTimeSamples;

    if (processContext.state & Vst::ProcessContext::kSystemTimeValid)
        vst2timeInfo->nanoSeconds = processContext.systemTime;

    if (processContext.state & Vst::ProcessContext::kProjectTimeMusicValid)
        vst2timeInfo->ppqPos = processContext.projectTimeMusic;

    if (processContext.state & Vst::ProcessContext::kBarPositionValid)
        vst2timeInfo->barStartPos = processContext.barPositionMusic;

    if (processContext.state & Vst::ProcessContext::kCycleValid) {
        vst2timeInfo->cycleStartPos = processContext.cycleStartMusic;
        vst2timeInfo->cycleEndPos = processContext.cycleEndMusic;
    }

    if (processContext.state & Vst::ProcessContext::kTempoValid)
        vst2timeInfo->tempo = processContext.tempo;

    if (processContext.state & Vst::ProcessContext::kTimeSigValid)
    {
        vst2timeInfo->timeSigNumerator = processContext.timeSigNumerator;
        vst2timeInfo->timeSigDenominator = processContext.timeSigDenominator;
    }
}

void Vst3Host::SetTempo(int tempo, int sign1, int sign2)
{
    processContext.tempo = tempo;
    if(sign1!=0)
        processContext.timeSigNumerator = sign1;
    if(sign2!=0)
        processContext.timeSigDenominator = sign2;

    processContext.state |= Vst::ProcessContext::kTempoValid;
    processContext.state |= Vst::ProcessContext::kTimeSigValid;
}

void Vst3Host::GetTempo(int &tempo, int &sign1, int &sign2)
{
    if(processContext.state &  Vst::ProcessContext::kTempoValid)
        tempo=processContext.tempo;

    if(processContext.state & Vst::ProcessContext::kTimeSigValid) {
        sign1=processContext.timeSigNumerator;
        sign2=processContext.timeSigDenominator;
    }
}

void Vst3Host::SetSampleRate(float rate)
{
    processContext.sampleRate = rate;
}
