#include "vstplugin32.h"
#include "../../loaddll32/ipc32.h"

using namespace Connectables;

HANDLE VstPlugin32::hMapFile = 0;
HANDLE VstPlugin32::ipcMutex = 0;
HANDLE VstPlugin32::ipcSemStart = 0;
HANDLE VstPlugin32::ipcSemEnd = 0;
unsigned char* VstPlugin32::mapFileBuffer = 0;
char* VstPlugin32::chunkData = 0;

VstPlugin32::VstPlugin32(MainHost *myHost,int index, const ObjectInfo & info) :
    VstPlugin(myHost,index,info)
{

}

VstPlugin32::~VstPlugin32()
{
    // VstPlugin32::EffDispatch(effClose);
    delete pEffect;
    pEffect=0;
    Close();
    Unload();
    if(chunkData) {
   //     delete chunkData;
        chunkData=0;
    }
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

        if (!ipcMutex) {
            ipcMutex = CreateMutex(NULL, FALSE, L"vstboardLock");
            if (!ipcMutex) {
                return false;
            }
        }

        if (!ipcSemStart) {
            ipcSemStart = CreateSemaphore(NULL, 0,1, L"vstboardSemStart");
            if (!ipcSemStart) {
                return false;
            }
        }

        if (!ipcSemEnd) {
            ipcSemEnd = CreateSemaphore(NULL, 0,1, L"vstboardSemEnd");
            if (!ipcSemEnd) {
                return false;
            }
        }


        Lock();

        ipc32* pf = (ipc32*)mapFileBuffer;
        pf->function=ipc32::Function::None;

        Process();

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
    Lock();

    ipc32* pf = (ipc32*)mapFileBuffer;

    pf->function=ipc32::Function::LoadDll;
    name.copy(pf->name,name.length());

    ProcessAndWaitResult();
    UnlockAfterResult();
    return true;
}


bool VstPlugin32::initPlugin()
{
    Lock();

    ipc32* pf = (ipc32*)mapFileBuffer;
    pf->function=ipc32::Function::GetAEffect;

    ProcessAndWaitResult();

    pEffect = new AEffect();
    pEffect->flags = pf->flags;
    pEffect->numInputs = pf->numInputs;
    pEffect->numOutputs = pf->numOutputs;
    pEffect->initialDelay = pf->initialDelay;
    pEffect->numParams = pf->numParams;

    //disable gui for now
    pEffect->flags -= effFlagsHasEditor;

    UnlockAfterResult();

    return VstPlugin::initPlugin();
}

bool VstPlugin32::Unload()
{
    Lock();

    ipc32* pf = (ipc32*)mapFileBuffer;

    pf->function=ipc32::Function::UnloadDll;

    Process();

    return true;
}

long VstPlugin32::EffEditOpen(void *ptr)
{
    Lock();

    ipc32* pf = (ipc32*)mapFileBuffer;
    pf->function=ipc32::Function::EditOpen;

    Process();

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
    Lock();

    ipc32* pf = (ipc32*)mapFileBuffer;
    pf->function=ipc32::Function::GetParam;
    pf->index=index;

    ProcessAndWaitResult();

    float ret = pf->opt;

    UnlockAfterResult();

    return ret;
}

void VstPlugin32::EffSetParameter(long index, float parameter)
{
    Lock();

    ipc32* pf = (ipc32*)mapFileBuffer;
    pf->function=ipc32::Function::SetParam;
    pf->index=index;
    pf->opt = parameter;

    Process();

}


void VstPlugin32::EffProcess(float **inputs, float **outputs, long sampleframes)
{
    if(sampleframes>IPC_BUFFER_SIZE) {
        LOG("buffer too big");
        return;
    }

    Lock();

    ipc32* pf = (ipc32*)mapFileBuffer;

    pf->function=ipc32::Function::Process;
    pf->dataSize = sampleframes;
    size_t s = sizeof(float)*sampleframes*pEffect->numInputs;
    memcpy(pf->buffersIn,inputs,s);
    s = sizeof(float)*sampleframes*pEffect->numOutputs;
    memcpy(pf->buffersOut,outputs,s);

    Process();
}

void VstPlugin32::EffProcessReplacing(float **inputs, float **outputs, long sampleframes)
{

    if(sampleframes>IPC_BUFFER_SIZE) {
        LOG("buffer too big");
        return;
    }

    Lock();

    ipc32* pf = (ipc32*)mapFileBuffer;

    pf->function=ipc32::Function::ProcessReplace;
    pf->dataSize = sampleframes;

    float* tmp = (float*)&pf->buffersIn;
    for (int i = 0; i < pEffect->numInputs; i++) {
        memcpy(tmp,inputs[i], sizeof(float) * sampleframes );
        tmp += sizeof(float) * sampleframes;
    }

    //size_t s = sizeof(float)*sampleframes*pEffect->numInputs;
    //memcpy(pf->buffersIn,*inputs,s);
    //s = sizeof(float)*sampleframes*pEffect->numOutputs;
    ProcessAndWaitResult();

    //memcpy(*outputs,pf->buffersOut,s);

    tmp = (float*)&pf->buffersOut;
    for (int i = 0; i < pEffect->numOutputs; i++) {
        memcpy(outputs[i],tmp, sizeof(float) * sampleframes );
        tmp += sizeof(float) * sampleframes;
    }
/*
    QString l="";
    for(int a=0;a<sampleframes;a++) {
        l.append( QString::number( outputs[0][a] ) );
    }
    LOG("out " << l)
*/
    UnlockAfterResult();
}

void VstPlugin32::EffProcessDoubleReplacing(double **inputs, double **outputs, long sampleframes)
{
#if defined(VST_2_4_EXTENSIONS)

    if(sampleframes>IPC_BUFFER_SIZE) {
        LOG("buffer too big");
        return;
    }

    Lock();

    ipc32* pf = (ipc32*)mapFileBuffer;

    pf->function=ipc32::Function::ProcessDouble;
    pf->dataSize = sampleframes;
    size_t s = sizeof(double)*sampleframes*pEffect->numInputs;
    memcpy(pf->buffersIn,inputs,s);
    s = sizeof(double)*sampleframes*pEffect->numOutputs;
    memcpy(pf->buffersOut,outputs,s);

    Process();

#endif
}

long VstPlugin32::EffGetChunk(void **ptr, bool isPreset) const
{
    Lock();

    ipc32* pf = (ipc32*)mapFileBuffer;
    pf->function=ipc32::Function::GetChunk;

    ProcessAndWaitResult();

    VstInt32 chunkSize = pf->dataSize;
    LOG("get chunk size:" << pf->dataSize);

    if(chunkData) {
        delete chunkData;
        chunkData=0;
    }
    chunkData = new char[chunkSize];
    *ptr=chunkData;

    UnlockAfterResult();

    GetChunkSegment(chunkData,chunkSize);

    // Lock();
    // pf->function = ipc32::Function::DeleteChunk;
    // Process();

    return chunkSize;
}

bool VstPlugin32::GetChunkSegment(char *ptr, VstInt32 chunkSize)
{
    VstInt32 start=0;

    while(start<chunkSize) {

        Lock();

        ipc32* pf = (ipc32*)mapFileBuffer;
        pf->function=ipc32::Function::GetChunkSegment;
        pf->value=start;
        pf->dataSize = std::min( chunkSize - start , IPC_CHUNK_SIZE);

        ProcessAndWaitResult();

        memcpy_s(ptr + pf->value,IPC_CHUNK_SIZE,pf->data, std::min( pf->dataSize , IPC_CHUNK_SIZE) );

        UnlockAfterResult();

        start+=IPC_CHUNK_SIZE;
    }

    return true;
}

bool VstPlugin32::SendChunkSegment(char *ptr, VstInt32 chunkSize)
{
    Lock();
    ipc32* pf = (ipc32*)mapFileBuffer;
    pf->function=ipc32::Function::SetChunk;
    pf->dataSize = chunkSize;

    ProcessAndWaitResult();
    UnlockAfterResult();

    VstInt32 start=0;

    while(start<chunkSize) {

        Lock();
        ipc32* pf = (ipc32*)mapFileBuffer;
        pf->function=ipc32::Function::SetChunkSegment;
        pf->value = start;
        pf->dataSize = std::min( chunkSize - start , IPC_CHUNK_SIZE);
        memcpy_s(pf->data,IPC_CHUNK_SIZE,ptr+start,pf->dataSize);

        ProcessAndWaitResult();
        UnlockAfterResult();

        start+=IPC_CHUNK_SIZE;
    }
    return true;
}

long VstPlugin32::EffDispatch(VstInt32 opCode, VstInt32 index, VstIntPtr value, void *ptr, float opt, VstInt32 size)
{

    if(size>IPC_CHUNK_SIZE) {
        LOG("send segmented");
        SendChunkSegment((char*)ptr,size);
    }

    Lock();

    ipc32* pf = (ipc32*)mapFileBuffer;
    pf->function = ipc32::Function::Dispatch;
    pf->opCode = opCode;
    pf->index = index;
    pf->value = (VstInt32)value;
    pf->opt = opt;
    //if the data is not already sent segmented
    if(ptr && size<=IPC_CHUNK_SIZE) {
        memcpy_s(&pf->data,IPC_CHUNK_SIZE,ptr,size);
    }
    pf->dataSize=size;



    if(opCode==effSetSpeakerArrangement || opCode==effGetSpeakerArrangement) {
        //value is also a pointer
    }

    ProcessAndWaitResult();

    long disp = pf->dispatchReturn;
    size = pf->dataSize;
    if(ptr && size<=IPC_CHUNK_SIZE) {
        memcpy(ptr,&pf->data,size);
    }

    UnlockAfterResult();


    if(size>IPC_CHUNK_SIZE) {
        LOG("get segmented");
        GetChunkSegment((char*)ptr,size);
    }

    // Lock();
    // pf->function = ipc32::Function::DeleteChunk;
    // Process();

    return disp;
}

void VstPlugin32::Lock()
{
    if(WaitForSingleObject(ipcMutex, 10000) == WAIT_TIMEOUT) {
        LOG("ipcMutex timeout");
        return;
    }
}

void VstPlugin32::Process()
{
    ReleaseSemaphore(ipcSemStart, 1, NULL);
    ReleaseMutex(ipcMutex);
}

void VstPlugin32::ProcessAndWaitResult()
{
    ReleaseSemaphore(ipcSemStart, 1, NULL);
    ReleaseMutex(ipcMutex);

    LONG val=0;
    ReleaseSemaphore(ipcSemEnd, 0, &val);
    if(val>0) {
        LOG("semaphore not locked?");
    }

    if(WaitForSingleObject(ipcSemEnd, 10000) == WAIT_TIMEOUT) {
        LOG("ipcSemEnd timeout");
        return;
    }

    //ipcSemEnd won't behave, really check if the process is finished :
    //lock, read, unlock.. until the server reset the function to none
    ipc32::Function currentF=ipc32::Function::Wait;
    do {
        if(WaitForSingleObject(ipcMutex, 10000) == WAIT_TIMEOUT) {
            LOG("ipcMutex timeout");
            return;
        }
        ipc32* pf = (ipc32*)mapFileBuffer;
        currentF = pf->function;
        ReleaseMutex(ipcMutex);
    } while(currentF != ipc32::Function::None);

    if(WaitForSingleObject(ipcMutex, 10000) == WAIT_TIMEOUT) {
        LOG("ipcMutex timeout");
        return;
    }
}

void VstPlugin32::UnlockAfterResult()
{
    ReleaseMutex(ipcMutex);
}
