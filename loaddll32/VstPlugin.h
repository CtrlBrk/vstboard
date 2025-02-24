#pragma once

#include <windows.h>
#include <string>
#include <thread>
#include <atomic>
#include "ipc32.h"
#include "ipc.h"

#define VST_FORCE_DEPRECATED 0
#include "vestige.h"

typedef AEffect* (*vstPluginFuncPtr)(audioMasterCallback host);
typedef int (*dispatcherFuncPtr)(AEffect* effect, int  opCode, int  index, int  value, void* ptr, float opt);

class VstWin;
class CVSTHost;
class IpcVst;
class VstPlugin
{
public:
    VstPlugin(IpcVst *ipc, int pluginId);
    ~VstPlugin();

public:
    VstWin* win;
    AEffect* pEffect;
    bool bEditOpen;
    bool bNeedIdle;
    bool bInEditIdle;
    bool bWantMidi;
    bool bInSetProgram;
    VstPlugin* pMasterEffect;             /* for Shell-type plugins            */
    std::wstring sName;

    bool Load(const std::wstring& name, float sampleRate, int  blocksize);
    bool Unload();

    bool LoadBank(std::string* name);
    bool SaveBank(std::string* name);

    bool LoadProgram(std::string* name);
    bool SaveProgram(std::string* name);

    static int  PluginIdFromBankFile(std::string* name);

    long EffDispatch(int  opCode, int  index = 0, int  value = 0, void* ptr = nullptr, float opt = .0f);
    int  OnMasterCallback(long opcode, long index, long value, void* ptr, float opt, long currentReturnCode);

    void EffProcess(float** inputs, float** outputs, long sampleframes);
    void EffProcessReplacing(float** inputs, float** outputs, long sampleframes);
    void EffProcessDoubleReplacing(double** inputs, double** outputs, long sampleFrames);
    long EffEditOpen(void* ptr) { long l = EffDispatch(effEditOpen, 0, 0, ptr); /* if (l > 0) */ bEditOpen = true; return l; }
    void EffEditClose() { if (!bEditOpen) return; EffDispatch(effEditClose); bEditOpen = false; }
    void EffEditIdle() { if (!bEditOpen || bInEditIdle) return; bInEditIdle = true; EffDispatch(effEditIdle); bInEditIdle = false; }

    long EffGetParamName(long index, char* txt);
    bool getFlags(int  m) const { if (!pEffect) return 0; return (pEffect->flags & m) == m; }
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

//    static void RunPlugin(VstPlugin* p);
    static void RunPluginBuffer(VstPlugin* p);

public:
    
    void OnSizeEditorWindow(long width, long height) { }
    bool OnUpdateDisplay() { return false; }
    bool OnIoChanged() { return false; }
    long OnGetNumAutomatableParameters() { return (pEffect) ? pEffect->numParams : 0; }

    void CrtVstWin();
    bool EditOpen();
    bool EditClose();
    void BuffersLoop();
    void MsgLoop();

    structTo32* dataIn;
    Ipc ipcIn;

    structBuffers* dataBuffers;
    Ipc ipcBuffers;

    std::atomic_bool closing;

 //   std::thread* guiThread;
    std::thread* processThread;

private:
    void TranslateMidiEvents(structTo32* map, void* data, int datasize);

    VstEvents* listEvnts;
    IpcVst* ipc;
    int pluginId;
    HMODULE pluginLib;
    void* chunk;
    void* chunkIn;
    int chunkInSize;
    HWND parentWindow;
};


