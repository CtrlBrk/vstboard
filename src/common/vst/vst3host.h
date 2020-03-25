#ifndef VST3HOST_H
#define VST3HOST_H

#include "public.sdk/source/vst/hosting/hostclasses.h"
#include "pluginterfaces/vst/ivstprocesscontext.h"
#include "pluginterfaces/vst2.x/aeffectx.h"
#include "pluginterfaces/vst/vsttypes.h"

using namespace Steinberg;

class Vst3Host : public Vst::HostApplication
{
public:
    Vst3Host();
    tresult PLUGIN_API getName (Vst::String128 name);
    void UpdateTime(long buffSize, float sampleRate);
    float GetCurrentBarTic();
    void SetTempo(int tempo, int sign1, int sign2);
    void GetTempo(int &tempo, int &sign1, int &sign2);
    void SetTimeInfo(const Vst::ProcessContext *info);
    void GetTimeInfo(VstTimeInfo *info);

    Vst::ProcessContext processContext;
  //  VstTimeInfo vst2TimeInfo;
    int currentBar;
    int loopLenght;
};

#endif // VST3HOST_H
