#ifndef VST3HOST_H
#define VST3HOST_H

//#include "../precomp.h"
//#include "const.h"


#ifdef _MSC_VER
#pragma warning( push, 1 )
#endif
#include "public.sdk/source/vst/hosting/hostclasses.h"
#include "pluginterfaces/vst/ivstprocesscontext.h"
#include "pluginterfaces/vst/vsttypes.h"
#ifndef __linux__
    #include "pluginterfaces/vst2.x/aeffectx.h"
#endif
#ifdef _MSC_VER
#pragma warning( pop )
#endif




using namespace Steinberg;

class Vst3Host : public QObject, public Vst::HostApplication
{
    Q_OBJECT

public:
    Vst3Host(QObject *parent);
    tresult PLUGIN_API getName (Vst::String128 name) override;
    void UpdateTime(long buffSize, float sampleRate);
    float GetCurrentBarTic();
    void SetTempo(int tempo, int sign1, int sign2);
    void GetTempo(int &tempo, int &sign1, int &sign2);
    void SetTimeInfo(const Vst::ProcessContext *info);
//    void GetTimeInfo(VstTimeInfo *info);

    Vst::ProcessContext processContext;
  //  VstTimeInfo vst2TimeInfo;
    int currentBar;
    int loopLenght;

public slots:
    void SetSampleRate(float rate=44100.0);
};

#endif // VST3HOST_H
