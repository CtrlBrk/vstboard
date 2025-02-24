#ifndef IPC32_H
#define IPC32_H

#define VST_FORCE_DEPRECATED 0
#define IPC_CHUNK_SIZE 2048
#define IPC_BUFFER_SIZE 4096

//#define IPC_TIMEOUT INFINITE
#define IPC_TIMEOUT 5000


#include "vestige.h"

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
    int pluginId = 0;
    IpcFunction function;
    wchar_t name[256];
    int value = 0;
    int index = 0;
};

struct structFrom32 {
    int cbPluginId = 0;
    IpcFunction callback;
    int cbOpcode = 0;
    int cbIndex = 0;
    int cbValue = 0;
    float cbOpt = 0.;
};

struct structBuffers {
    int pluginId = 0;
    IpcFunction function;
    int dataSize = 0;
    float buffersIn[IPC_BUFFER_SIZE];
    float buffersOut[IPC_BUFFER_SIZE];
};

struct structTo32 {
    int mainWin=0;

    int pluginId=0;
    IpcFunction function;
    int opCode=0;
    int index=0;
    int value=0;
    float opt=0.;
    long dispatchReturn=0L;
        
    wchar_t name[256];
    
    int flags=0;
    int numInputs=0;
    int numOutputs=0;
    int initialDelay=0;
    int numParams=0;
    char data[IPC_CHUNK_SIZE];
    int dataSize=0;
};

#endif // IPC32_H
