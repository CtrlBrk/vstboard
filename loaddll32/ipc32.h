#ifndef IPC32_H
#define IPC32_H

#define VST_FORCE_DEPRECATED 0
#include "pluginterfaces/vst2.x/aeffect.h"

struct ipc32 {
    enum class Function
    {
        None,
        Wait,
        LoadDll,
        UnloadDll,
        GetAEffect,
        Dispatch,
        EditOpen,
        GetRect,
        GetParam,
        SetParam,
        Process,
        ProcessReplace,
        ProcessDouble
    };

    VstInt32 opCode;
    VstInt32 index=0;
    VstInt32 value=0;
    //void *ptr=0;
    float opt=0.;
    long dispatchReturn=0L;

    Function function;
    wchar_t name[256];
   // int buffSize;
    float buffersIn[10000];
    float buffersOut[10000];
    // AEffect pEffect;

    VstInt32 flags=0;
    VstInt32 numInputs=0;
    VstInt32 numOutputs=0;
    VstInt32 initialDelay=0;
    VstInt32 numParams=0;
    char data[1000];
    VstInt32 dataSize=0;
};

#endif // IPC32_H
