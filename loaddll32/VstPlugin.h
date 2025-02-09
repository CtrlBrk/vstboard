#pragma once

#include <windows.h>
#include <string>
#define VST_FORCE_DEPRECATED 0
#include "public.sdk/source/vst2.x/audioeffectx.h"
//#include "pluginterfaces/vst2.x/aeffectx.h"

typedef AEffect* (*vstPluginFuncPtr)(audioMasterCallback host);
typedef VstIntPtr(*dispatcherFuncPtr)(AEffect* effect, VstInt32 opCode, VstInt32 index, VstInt32 value, void* ptr, float opt);


class CVSTHost;
class VstPlugin //: public vst::CEffect
{
public:
    VstPlugin();
    ~VstPlugin();

public:
    AEffect* pEffect;
    bool bEditOpen;
    bool bNeedIdle;
    bool bInEditIdle;
    bool bWantMidi;
    bool bInSetProgram;
    VstPlugin* pMasterEffect;             /* for Shell-type plugins            */
    std::wstring sName;

    bool Load(const std::wstring& name);
    bool Unload();

    bool LoadBank(std::string* name);
    bool SaveBank(std::string* name);

    bool LoadProgram(std::string* name);
    bool SaveProgram(std::string* name);

    static VstInt32 PluginIdFromBankFile(std::string* name);

    long EffDispatch(VstInt32 opCode, VstInt32 index = 0, VstIntPtr value = 0, void* ptr = 0, float opt = 0.);
    VstIntPtr OnMasterCallback(long opcode, long index, long value, void* ptr, float opt, long currentReturnCode);

    void EffProcess(float** inputs, float** outputs, long sampleframes);
    void EffProcessReplacing(float** inputs, float** outputs, long sampleframes);
    void EffProcessDoubleReplacing(double** inputs, double** outputs, long sampleFrames);
    void EffSetParameter(long index, float parameter);
    float EffGetParameter(long index);


    long EffGetParamName(long index, char* txt);

    //as const
    long EffGetChunk(void** ptr, bool isPreset = false) const
    {
        if (!pEffect)
            return 0;

        long disp = 0L;
        disp = pEffect->dispatcher(pEffect, effGetChunk, isPreset, 0, ptr, .0);
        return disp;
        //  return EffDispatch(effGetChunk, isPreset, 0, ptr);
    }

    // overridables
public:
    void OnSizeEditorWindow(long width, long height) { }
    bool OnUpdateDisplay() { return false; }
    void* OnOpenWindow(VstWindow* window) { return 0; }
    bool OnCloseWindow(VstWindow* window) { return false; }
    bool OnIoChanged() { return false; }
    long OnGetNumAutomatableParameters() { return (pEffect) ? pEffect->numParams : 0; }

private:
    HMODULE pluginLib;
};


