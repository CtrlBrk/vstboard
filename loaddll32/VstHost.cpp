#include "VstHost.h"
#include "pluginterfaces/vst2.x/aeffectx.h"
#include <cmath>
#include "VstPlugin.h"

/*===========================================================================*/
/* CVSTHost class members                                                    */
/*===========================================================================*/

CVSTHost* CVSTHost::pHost = NULL;      /* pointer to the one and only host  */

/*****************************************************************************/
/* CVSTHost : constructor                                                    */
/*****************************************************************************/

CVSTHost::CVSTHost() :
    loopLenght(4),
    currentBar(0),
    barLengthq(0)
{
    if (pHost)                              /* disallow more than one host!      */
        return;//throw((int)1);
    vstTimeInfo.samplePos = 0.0;
    vstTimeInfo.sampleRate = 44100.0;
    vstTimeInfo.nanoSeconds = 0.0;
    vstTimeInfo.ppqPos = 0.0;
    vstTimeInfo.tempo = 120.0;
    vstTimeInfo.barStartPos = 0.0;
    vstTimeInfo.cycleStartPos = 0.0;
    vstTimeInfo.cycleEndPos = 0.0;
    vstTimeInfo.timeSigNumerator = 4;
    vstTimeInfo.timeSigDenominator = 4;
    vstTimeInfo.smpteOffset = 0;
    vstTimeInfo.smpteFrameRate = 1;
    vstTimeInfo.samplesToNextClock = 0;
    vstTimeInfo.flags = 0;

    vstTimeInfo.flags |= kVstTransportChanged;//     = 1,		///< indicates that play, cycle or record state has changed
    vstTimeInfo.flags |= kVstTransportPlaying;//     = 1 << 1,	///< set if Host sequencer is currently playing
    vstTimeInfo.flags |= kVstTransportCycleActive;// = 1 << 2,	///< set if Host sequencer is in cycle mode
    //vstTimeInfo.flags |= kVstTransportRecording;//   = 1 << 3,	///< set if Host sequencer is in record mode
    //vstTimeInfo.flags |= kVstAutomationWriting;//    = 1 << 6,	///< set if automation write mode active (record parameter changes)
    vstTimeInfo.flags |= kVstAutomationReading;//    = 1 << 7,	///< set if automation read mode active (play parameter changes)
    vstTimeInfo.flags |= kVstNanosValid;//           = 1 << 8,	///< VstTimeInfo::nanoSeconds valid
    vstTimeInfo.flags |= kVstPpqPosValid;//          = 1 << 9,	///< VstTimeInfo::ppqPos valid
    vstTimeInfo.flags |= kVstTempoValid;//           = 1 << 10,	///< VstTimeInfo::tempo valid
    vstTimeInfo.flags |= kVstBarsValid;//            = 1 << 11,	///< VstTimeInfo::barStartPos valid
    vstTimeInfo.flags |= kVstCyclePosValid;//        = 1 << 12,	///< VstTimeInfo::cycleStartPos and VstTimeInfo::cycleEndPos valid
    vstTimeInfo.flags |= kVstTimeSigValid;//         = 1 << 13,	///< VstTimeInfo::timeSigNumerator and VstTimeInfo::timeSigDenominator valid
    vstTimeInfo.flags |= kVstSmpteValid;//           = 1 << 14,	///< VstTimeInfo::smpteOffset and VstTimeInfo::smpteFrameRate valid
    vstTimeInfo.flags |= kVstClockValid;

    pHost = this;                           /* install this instance as the one  */
}

/*****************************************************************************/
/* ~CVSTHost : destructor                                                    */
/*****************************************************************************/

CVSTHost::~CVSTHost()
{
    if (pHost == this)                      /* if we're the chosen one           */
        pHost = NULL;                         /* remove ourselves from pointer     */
}

void CVSTHost::SetTimeInfo(const VstTimeInfo* info) {
    if (!info)
        return;

    //todo : recalculate values depending on the given info

    if (info->flags & kVstNanosValid)
        vstTimeInfo.nanoSeconds = info->nanoSeconds;
    if (info->flags & kVstPpqPosValid)
        vstTimeInfo.ppqPos = info->ppqPos;
    if (info->flags & kVstTempoValid)
        vstTimeInfo.tempo = info->tempo;
    if (info->flags & kVstBarsValid)
        vstTimeInfo.barStartPos = info->barStartPos;
    if (info->flags & kVstCyclePosValid) {
        vstTimeInfo.cycleStartPos = info->cycleStartPos;
        vstTimeInfo.cycleEndPos = info->cycleEndPos;
    }
    if (info->flags & kVstTimeSigValid) {
        vstTimeInfo.timeSigNumerator = info->timeSigNumerator;
        vstTimeInfo.timeSigDenominator = info->timeSigDenominator;
    }
    if (info->flags & kVstSmpteValid) {
        vstTimeInfo.smpteOffset = info->smpteOffset;
        vstTimeInfo.smpteFrameRate = info->smpteFrameRate;
    }
    if (info->flags & kVstClockValid)
        vstTimeInfo.samplesToNextClock = info->samplesToNextClock;
}

void CVSTHost::SetTempo(int tempo, int sign1, int sign2)
{
    vstTimeInfo.tempo = tempo;
    if (sign1 != 0)
        vstTimeInfo.timeSigNumerator = sign1;
    if (sign2 != 0)
        vstTimeInfo.timeSigDenominator = sign2;

    vstTimeInfo.flags |= kVstTempoValid;
    vstTimeInfo.flags |= kVstTimeSigValid;
}

void CVSTHost::GetTempo(int& tempo, int& sign1, int& sign2)
{
    if (vstTimeInfo.flags & kVstTempoValid)
        tempo = vstTimeInfo.tempo;

    if (vstTimeInfo.flags & kVstTimeSigValid) {
        sign1 = vstTimeInfo.timeSigNumerator;
        sign2 = vstTimeInfo.timeSigDenominator;
    }
}

void CVSTHost::UpdateTimeInfo(double timer, int addSamples, double sampleRate)
{
    vstTimeInfo.sampleRate = sampleRate;
    vstTimeInfo.nanoSeconds = timer * 1000000.0L;
    vstTimeInfo.samplePos += addSamples;

    //bar length in quarter notes
    barLengthq = (double)(4 * vstTimeInfo.timeSigNumerator) / vstTimeInfo.timeSigDenominator;


    vstTimeInfo.cycleEndPos = barLengthq * (double)loopLenght;
    vstTimeInfo.cycleStartPos = 0;

    // we don't care for the mask in here
    static double fSmpteDiv[] =
    {
            24.f,
            25.f,
            24.f,
            30.f,
            29.97f,
            30.f
    };

    double dPos = vstTimeInfo.samplePos / vstTimeInfo.sampleRate;
    vstTimeInfo.ppqPos = dPos * vstTimeInfo.tempo / 60.L;

    if (vstTimeInfo.ppqPos > vstTimeInfo.cycleEndPos) {

        vstTimeInfo.ppqPos -= vstTimeInfo.cycleEndPos;
        dPos = vstTimeInfo.ppqPos / vstTimeInfo.tempo * 60.L;
        vstTimeInfo.samplePos = dPos * vstTimeInfo.sampleRate;
        double dOffsetInSecond = dPos - floor(dPos);
        vstTimeInfo.smpteOffset = (VstInt32)(dOffsetInSecond * fSmpteDiv[vstTimeInfo.smpteFrameRate] * 80.L);

    }

    /* offset in fractions of a second   */
    double dOffsetInSecond = dPos - floor(dPos);
    vstTimeInfo.smpteOffset = (VstInt32)(dOffsetInSecond * fSmpteDiv[vstTimeInfo.smpteFrameRate] * 80.L);

    //start of last bar
    currentBar = floor(vstTimeInfo.ppqPos / barLengthq);
    vstTimeInfo.barStartPos = barLengthq * (double)currentBar;
}

/*****************************************************************************/
/* AudioMasterCallback : callback to be called by plugins                    */
/*****************************************************************************/
VstIntPtr VSTCALLBACK CVSTHost::AudioMasterCallback(AEffect* effect, VstInt32 opcode, VstInt32  index, VstIntPtr  value, void* ptr, float opt)
{
    const char vendor[] = "CtrlBrk";
    const char product[] = "VstBoard";
    long retValue = 0L;
    const char* str;

    switch (opcode) {
    case audioMasterVersion: //1
        return 2400L;

    case audioMasterGetTime: //7
        return (VstIntPtr)&pHost->vstTimeInfo;

    case audioMasterSetTime: //9
        pHost->SetTimeInfo((VstTimeInfo*)ptr);
        return 1L;

    case audioMasterTempoAt: //10
        return 1000L * pHost->vstTimeInfo.tempo;

    case audioMasterGetCurrentProcessLevel: //23
        return 2L;

    case audioMasterGetVendorString: //32
        strcpy_s((char*)ptr, sizeof(vendor), vendor);
        return 1L;

    case audioMasterGetProductString: //33
        strcpy_s((char*)ptr, sizeof(product), product);
        return 1L;

    case audioMasterGetVendorVersion: //34
        return 0001L;

    case audioMasterCanDo: //37
        str = (const char*)ptr;

        if ((!strcmp(str, "sendVstEvents")) ||
            (!strcmp(str, "sendVstMidiEvent")) ||
            (!strcmp(str, "receiveVstEvents")) ||
            (!strcmp(str, "receiveVstMidiEvent")) ||
            (!strcmp(str, "sizeWindow")) ||
            (!strcmp(str, "sendVstMidiEventFlagIsRealtime")) ||
            (!strcmp(str, "reportConnectionChanges")) ||
            (!strcmp(str, "acceptIOChanges")) ||
            (!strcmp(str, "sendVstTimeInfo")) ||
            (!strcmp(str, "openFileSelector")) ||
            (!strcmp(str, "closeFileSelector")) ||
            (!strcmp(str, "supplyIdle")) ||
            (!strcmp(str, "receiveVstTimeInfo")) ||
            (!strcmp(str, "shellCategory")) ||
            0)
            return 1L;

        //ignore the rest
        return 0L;

        //handled by the object
    case audioMasterAutomate: //0
    case audioMasterCurrentId: //2
    case audioMasterIdle: //3
    case audioMasterPinConnected: //4
    case audioMasterWantMidi: //6
    case audioMasterProcessEvents: //8
    case audioMasterGetNumAutomatableParameters: //11
    case audioMasterIOChanged: //13
    case audioMasterNeedIdle: //14
    case audioMasterSizeWindow: //15
    case audioMasterGetSampleRate: //16
    case audioMasterGetInputLatency: //18
    case audioMasterGetOutputLatency: //19
    case audioMasterUpdateDisplay: //42
    case audioMasterBeginEdit: //43
    case audioMasterEndEdit: //44
        break;

    default:
        return 0L;
    }

    if (effect) {
        //send it to the connectable object
        //Connectables::VstPlugin* obj = Connectables::VstPlugin::mapPlugins.value(effect, 0);
        //if (obj)
        //    return obj->OnMasterCallback(opcode, index, value, ptr, opt, retValue);
        VstPlugin* p = (VstPlugin*)effect->user;
        if (p) {
            return p->OnMasterCallback(opcode, index, value, ptr, opt, retValue);
        }
    }

    //unknown pointer, it must be the plugin currently loading
    //if (Connectables::VstPlugin::pluginLoading) {
    //    return Connectables::VstPlugin::pluginLoading->OnMasterCallback(opcode, index, value, ptr, opt, retValue);
    //}

    return 0L;

}
