#ifndef IPC32_H
#define IPC32_H

#define VST_FORCE_DEPRECATED 0
#define IPC_CHUNK_SIZE 2048
#define IPC_BUFFER_SIZE 4096

#define IPC_TIMEOUT INFINITE
//#define IPC_TIMEOUT 10000

#include "pluginterfaces/vst2.x/aeffect.h"
enum class IpcFunction
{
    None,
    Wait,
    LoadDll,
    UnloadDll,
    GetAEffect,
    Dispatch,
    EditorOpen,
    GetRect,
    EditorShow,
    EditorHide,
    GetParam,
    SetParam,
    Process,
    ProcessReplace,
    ProcessDouble,
    ProcessMidi,
    GetChunk,
    GetChunkSegment,
    SetChunk,
    SetChunkSegment,
    DeleteChunk

};


struct structPilot {
    VstInt32 pluginId = 0;
    IpcFunction function;
    wchar_t name[256];
    VstInt32 value = 0;
    VstInt32 index = 0;
};

struct structFrom32 {
    VstInt32 cbPluginId = 0;
    IpcFunction callback;
    VstInt32 cbOpcode = 0;
    VstInt32 cbIndex = 0;
    VstInt32 cbValue = 0;
    float cbOpt = 0.;
};

struct structBuffers {
    VstInt32 pluginId = 0;
    IpcFunction function;
    VstInt32 dataSize = 0;
    float buffersIn[IPC_BUFFER_SIZE];
    float buffersOut[IPC_BUFFER_SIZE];
};

struct structTo32 {
    

    VstInt32 pluginId=0;
    IpcFunction function;
    VstInt32 opCode=0;
    VstInt32 index=0;
    VstInt32 value=0;
    float opt=0.;
    long dispatchReturn=0L;
        
    wchar_t name[256];
    
    VstInt32 flags=0;
    VstInt32 numInputs=0;
    VstInt32 numOutputs=0;
    VstInt32 initialDelay=0;
    VstInt32 numParams=0;
    char data[IPC_CHUNK_SIZE];
    VstInt32 dataSize=0;
};

#endif // IPC32_H
