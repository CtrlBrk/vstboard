#include "vstplugin32.h"
#include "../../loaddll32/ipc32.h"

using namespace Connectables;

HANDLE VstPlugin32::hMapFile = 0;
HANDLE VstPlugin32::ipcMutex = 0;
unsigned char* VstPlugin32::mapFileBuffer = 0;
// QByteArray VstPlugin32::ipcData;

VstPlugin32::VstPlugin32(MainHost *myHost,int index, const ObjectInfo & info) :
    VstPlugin(myHost,index,info)
{
    ipcMutexStartProcess = CreateMutex(NULL, FALSE, L"vstboardLockStart");
    if (!ipcMutexStartProcess) {
        return ;
    }
    WaitForSingleObject(ipcMutexStartProcess, 10000);
}

VstPlugin32::~VstPlugin32()
{
    EffDispatch(effClose);
    Close();
}

bool VstPlugin32::ProcessInit()
{
    if(!hMapFile) {

        TCHAR szName[] = TEXT("Local\\vstboard");
        hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(ipc32), szName);
        if (!hMapFile)
        {
            MessageBoxA(GetActiveWindow(), "CreateFileMapping failed", "VstBoard", MB_OK);
            return false;
        }
        mapFileBuffer = (unsigned char*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(ipc32));
        if (!mapFileBuffer)
        {
            MessageBoxA(GetActiveWindow(), "MapViewOfFile failed", "VstBoard", MB_OK);
            return false;
        }

        ipcMutex = CreateMutex(NULL, FALSE, L"vstboardLock");
        if (!ipcMutex) {
            return false;
        }


        if(WaitForSingleObject(ipcMutex, 10000) == WAIT_TIMEOUT) {
            return false;
        }
        ipc32* pf = (ipc32*)mapFileBuffer;
        pf->function=ipc32::Function::None;
        ReleaseMutex(ipcMutex);
    }

    return true;
}

bool VstPlugin32::Open()
{
    if(!ProcessInit()) {
        SetErrorMessage("32bit IPC error");
        return false;
    }

    return VstPlugin::Open();
}

bool VstPlugin32::Load(const std::wstring &name)
{
    if(WaitForSingleObject(ipcMutex, 10000) == WAIT_TIMEOUT) {
        return false;
    }
    ipc32* pf = (ipc32*)mapFileBuffer;

    pf->function=ipc32::Function::LoadDll;
    name.copy(pf->name,name.length());

    ReleaseMutex(ipcMutex);
    return true;
}


bool VstPlugin32::initPlugin()
{
    if(WaitForSingleObject(ipcMutex, 10000) == WAIT_TIMEOUT) {
        return false;
    }
    ipc32* pf = (ipc32*)mapFileBuffer;
    pf->function=ipc32::Function::GetAEffect;
    ReleaseMutex(ipcMutex);

    if(WaitForSingleObject(ipcMutex, 10000) == WAIT_TIMEOUT) {
        return false;
    }
    pEffect = new AEffect();
    pEffect->flags = pf->flags;
    pEffect->numInputs = pf->numInputs;
    pEffect->numOutputs = pf->numOutputs;
    pEffect->initialDelay = pf->initialDelay;
    pEffect->numParams = pf->numParams;
    ReleaseMutex(ipcMutex);
    return VstPlugin::initPlugin();
}

bool VstPlugin32::Unload()
{
    if(WaitForSingleObject(ipcMutex, 10000) == WAIT_TIMEOUT) {
        return false;
    }
    ipc32* pf = (ipc32*)mapFileBuffer;

    pf->function=ipc32::Function::UnloadDll;

    ReleaseMutex(ipcMutex);
    return true;
}

long VstPlugin32::EffEditOpen(void *ptr)
{
    if(WaitForSingleObject(ipcMutex, 10000) == WAIT_TIMEOUT) {
        return false;
    }
    ipc32* pf = (ipc32*)mapFileBuffer;
    pf->function=ipc32::Function::EditOpen;
    ReleaseMutex(ipcMutex);
    return true;
}

long VstPlugin32::EffEditGetRect(ERect **ptr)
{
    return true;
}
void VstPlugin32::CreateEditorWindow()
{

}

float VstPlugin32::EffGetParameter(long index)
{
    if(WaitForSingleObject(ipcMutex, 10000) == WAIT_TIMEOUT) {
        return 0;
    }
    ipc32* pf = (ipc32*)mapFileBuffer;
    pf->function=ipc32::Function::GetParam;
    pf->index=index;
    ReleaseMutex(ipcMutex);

    if(WaitForSingleObject(ipcMutex, 10000) == WAIT_TIMEOUT) {
        return 0;
    }
    float ret = pf->opt;
    ReleaseMutex(ipcMutex);
    return ret;
}

void VstPlugin32::EffSetParameter(long index, float parameter)
{
    if(!WaitForSingleObject(ipcMutex, 10000)) {
        return;
    }
    ipc32* pf = (ipc32*)mapFileBuffer;
    pf->function=ipc32::Function::SetParam;
    pf->index=index;
    pf->opt = parameter;
    ReleaseMutex(ipcMutex);
}


void VstPlugin32::EffProcess(float **inputs, float **outputs, long sampleframes)
{
    // pEffect->process(pEffect, inputs, outputs, sampleframes);
    if(WaitForSingleObject(ipcMutex, 10000) == WAIT_TIMEOUT) {
        return;
    }
    ipc32* pf = (ipc32*)mapFileBuffer;

    pf->function=ipc32::Function::Process;
    pf->dataSize = sampleframes;
    size_t s = sizeof(float)*sampleframes*pEffect->numInputs;
    memcpy(pf->buffersIn,inputs,s);
    s = sizeof(float)*sampleframes*pEffect->numOutputs;
    memcpy(pf->buffersOut,outputs,s);

    ReleaseMutex(ipcMutex);
}

void VstPlugin32::EffProcessReplacing(float **inputs, float **outputs, long sampleframes)
{

    // pEffect->processReplacing(pEffect, inputs, outputs, sampleframes);
    if(WaitForSingleObject(ipcMutex, 10000) == WAIT_TIMEOUT) {
        return;
    }
    ipc32* pf = (ipc32*)mapFileBuffer;

    pf->function=ipc32::Function::ProcessReplace;
    pf->dataSize = sampleframes;
    size_t s = sizeof(float)*sampleframes*pEffect->numInputs;
    memcpy(pf->buffersIn,*inputs,s);
    s = sizeof(float)*sampleframes*pEffect->numOutputs;
    memcpy(pf->buffersOut,*outputs,s);

    ReleaseMutex(ipcMutex);
}

void VstPlugin32::EffProcessDoubleReplacing(double **inputs, double **outputs, long sampleframes)
{
#if defined(VST_2_4_EXTENSIONS)
    // pEffect->processDoubleReplacing(pEffect, inputs, outputs, sampleFrames);
    if(WaitForSingleObject(ipcMutex, 10000) == WAIT_TIMEOUT) {
        return;
    }
    ipc32* pf = (ipc32*)mapFileBuffer;

    pf->function=ipc32::Function::ProcessDouble;
    pf->dataSize = sampleframes;
    size_t s = sizeof(double)*sampleframes*pEffect->numInputs;
    memcpy(pf->buffersIn,inputs,s);
    s = sizeof(double)*sampleframes*pEffect->numOutputs;
    memcpy(pf->buffersOut,outputs,s);

    ReleaseMutex(ipcMutex);
#endif
}

long VstPlugin32::EffDispatch(VstInt32 opCode, VstInt32 index, VstIntPtr value, void *ptr, float opt, VstInt32 size)
{
    if(WaitForSingleObject(ipcMutex, 10000) == WAIT_TIMEOUT) {
        return 0;
    }

    ipc32* pf = (ipc32*)mapFileBuffer;
    pf->function = ipc32::Function::Dispatch;
    pf->opCode = opCode;
    pf->index = index;
    pf->value = (VstInt32)value;
    pf->opt = opt;
    if(ptr) {
        memcpy(&pf->data,ptr,size);
    }
    pf->dataSize=size;

    if(opCode==effSetSpeakerArrangement || opCode==effGetSpeakerArrangement) {
        //value is also a pointer
    }

    ReleaseMutex(ipcMutex);
    // ReleaseMutex(ipcMutexStartProcess);

    if(WaitForSingleObject(ipcMutex, 10000) == WAIT_TIMEOUT) {
        return 0;
    }
    // if(WaitForSingleObject(ipcMutexStartProcess, 10000) == WAIT_TIMEOUT) {
        // return 0;
    // }

    if(pf->function == ipc32::Function::Dispatch) {
        ReleaseMutex(ipcMutex);
        LOG("locked too soon?")
        return -1;
    }
    long disp = pf->dispatchReturn;
    size = pf->dataSize;
    if(ptr) {
        memcpy(ptr,&pf->data,size);
    }
    ReleaseMutex(ipcMutex);

    return disp;
}
