#pragma once
#define VST_FORCE_DEPRECATED 0

#include "public.sdk/source/vst2.x/audioeffectx.h"

/*****************************************************************************/
/* CVSTHost class declaration                                                */
/*****************************************************************************/

class CVSTHost
{
    friend class VstPlugin;
public:
    static CVSTHost* Get() { return pHost; }
    CVSTHost();
    virtual ~CVSTHost();

    void SetTimeInfo(const VstTimeInfo* info);
    void SetTempo(int tempo, int sign1 = 0, int sign2 = 0);
    void GetTempo(int& tempo, int& sign1, int& sign2);
    void UpdateTimeInfo(double timer, int addSamples, double sampleRate);

    int loopLenght;
    VstTimeInfo vstTimeInfo;
    int currentBar;
    double barLengthq;

protected:
    static CVSTHost* pHost;
    static VstIntPtr VSTCALLBACK AudioMasterCallback(AEffect* effect, VstInt32 opcode, VstInt32  index, VstIntPtr  value, void* ptr, float opt);
};
