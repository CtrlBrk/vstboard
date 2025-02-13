#include "vstplugin32.h"


using namespace Connectables;

HANDLE VstPlugin32::hMapFile = 0;
HANDLE VstPlugin32::ipcMutex = 0;
HANDLE VstPlugin32::ipcSemStart = 0;
HANDLE VstPlugin32::ipcSemEnd = 0;
ipc32* VstPlugin32::map = 0;
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

bool VstPlugin32::Close()
{
    EffEditClose();
    return true;
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
        map = (ipc32*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(ipc32));
        if (!map)
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

        map->function=ipc32::Function::None;

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

    map->function=ipc32::Function::LoadDll;
    name.copy(map->name,name.length());

    ProcessAndWaitResult();
    UnlockAfterResult();
    return true;
}


bool VstPlugin32::initPlugin()
{
    Lock();

    map->function=ipc32::Function::GetAEffect;

    ProcessAndWaitResult();

    pEffect = new AEffect();
    pEffect->flags = map->flags;
    pEffect->numInputs = map->numInputs;
    pEffect->numOutputs = map->numOutputs;
    pEffect->initialDelay = map->initialDelay;
    pEffect->numParams = map->numParams;

    //disable gui for now
    //pEffect->flags -= effFlagsHasEditor;

    UnlockAfterResult();

    return VstPlugin::initPlugin();
}

bool VstPlugin32::Unload()
{
    Lock();

    map->function=ipc32::Function::UnloadDll;

    Process();

    return true;
}
/*
long VstPlugin32::EffEditOpen(void *ptr)
{
    Lock();

    map->function=ipc32::Function::EditorOpen;

    Process();

    return true;
}
*/
long VstPlugin32::EffEditGetRect(ERect **ptr)
{
    return true;
}
void VstPlugin32::CreateEditorWindow()
{
    EffEditOpen(NULL);
}

void VstPlugin32::OnShowEditor()
{
    Lock();

    map->function=ipc32::Function::EditorShow;

    Process();

}

void VstPlugin32::OnHideEditor()
{
    Lock();

    map->function=ipc32::Function::EditorHide;

    Process();
}

float VstPlugin32::EffGetParameter(long index)
{
    Lock();

    map->function=ipc32::Function::GetParam;
    map->index=index;

    ProcessAndWaitResult();

    float ret = map->opt;

    UnlockAfterResult();

    return ret;
}

void VstPlugin32::EffSetParameter(long index, float parameter)
{
    Lock();

    map->function=ipc32::Function::SetParam;
    map->index=index;
    map->opt = parameter;

    Process();

}


void VstPlugin32::EffProcess(float **inputs, float **outputs, long sampleframes)
{
    if(sampleframes>IPC_BUFFER_SIZE) {
        LOG("buffer too big");
        return;
    }

    Lock();

    map->function=ipc32::Function::Process;
    map->dataSize = sampleframes;
    size_t s = sizeof(float)*sampleframes*pEffect->numInputs;
    memcpy(map->buffersIn,inputs,s);
    s = sizeof(float)*sampleframes*pEffect->numOutputs;
    memcpy(map->buffersOut,outputs,s);

    Process();
}

void VstPlugin32::EffProcessReplacing(float **inputs, float **outputs, long sampleframes)
{

    if(sampleframes>IPC_BUFFER_SIZE) {
        LOG("buffer too big");
        return;
    }

    Lock();

    map->function=ipc32::Function::ProcessReplace;
    map->dataSize = sampleframes;

    float* tmp = (float*)&map->buffersIn;
    for (int i = 0; i < pEffect->numInputs; i++) {
        memcpy(tmp,inputs[i], sizeof(float) * sampleframes );
        tmp += sizeof(float) * sampleframes;
    }

    //size_t s = sizeof(float)*sampleframes*pEffect->numInputs;
    //memcpy(map->buffersIn,*inputs,s);
    //s = sizeof(float)*sampleframes*pEffect->numOutputs;
    ProcessAndWaitResult();

    //memcpy(*outputs,map->buffersOut,s);

    tmp = (float*)&map->buffersOut;
    for (int i = 0; i < pEffect->numOutputs; i++) {
        memcpy(outputs[i],tmp, sizeof(float) * sampleframes );
        tmp += sizeof(float) * sampleframes;
    }




    map->callback = ipc32::Function::None;

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

    map->function=ipc32::Function::ProcessDouble;
    map->dataSize = sampleframes;
    size_t s = sizeof(double)*sampleframes*pEffect->numInputs;
    memcpy(map->buffersIn,inputs,s);
    s = sizeof(double)*sampleframes*pEffect->numOutputs;
    memcpy(map->buffersOut,outputs,s);

    Process();

#endif
}

long VstPlugin32::EffGetChunk(void **ptr, bool isPreset)
{
    Lock();

    map->function=ipc32::Function::GetChunk;

    ProcessAndWaitResult();

    VstInt32 chunkSize = map->dataSize;
    LOG("get chunk size:" << map->dataSize);

    if(chunkData) {
        delete chunkData;
        chunkData=0;
    }
    chunkData = new char[chunkSize];
    *ptr=chunkData;

    UnlockAfterResult();

    GetChunkSegment(chunkData,chunkSize);

    // Lock();
    // map->function = ipc32::Function::DeleteChunk;
    // Process();

    return chunkSize;
}

bool VstPlugin32::GetChunkSegment(char *ptr, VstInt32 chunkSize)
{
    VstInt32 start=0;

    while(start<chunkSize) {

        Lock();

        map->function=ipc32::Function::GetChunkSegment;
        map->value=start;
        map->dataSize = std::min( chunkSize - start , IPC_CHUNK_SIZE);

        ProcessAndWaitResult();

        memcpy_s(ptr + map->value,IPC_CHUNK_SIZE,map->data, std::min( map->dataSize , IPC_CHUNK_SIZE) );

        UnlockAfterResult();

        start+=IPC_CHUNK_SIZE;
    }

    return true;
}

bool VstPlugin32::SendChunkSegment(char *ptr, VstInt32 chunkSize)
{
    Lock();

    map->function=ipc32::Function::SetChunk;
    map->dataSize = chunkSize;

    ProcessAndWaitResult();
    UnlockAfterResult();

    VstInt32 start=0;

    while(start<chunkSize) {

        Lock();

        map->function=ipc32::Function::SetChunkSegment;
        map->value = start;
        map->dataSize = std::min( chunkSize - start , IPC_CHUNK_SIZE);
        memcpy_s(map->data,IPC_CHUNK_SIZE,ptr+start,map->dataSize);

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

    map->function = ipc32::Function::Dispatch;
    map->opCode = opCode;
    map->index = index;
    map->value = (VstInt32)value;
    map->opt = opt;
    //if the data is not already sent segmented
    if(ptr && size<=IPC_CHUNK_SIZE) {
        memcpy_s(&map->data,IPC_CHUNK_SIZE,ptr,size);
    }
    map->dataSize=size;



    if(opCode==effSetSpeakerArrangement || opCode==effGetSpeakerArrangement) {
        //value is also a pointer
    }

    ProcessAndWaitResult();

    long disp = map->dispatchReturn;
    size = map->dataSize;
    if(ptr && size<=IPC_CHUNK_SIZE) {
        memcpy(ptr,&map->data,size);
    }

    UnlockAfterResult();


    if(size>IPC_CHUNK_SIZE) {
        LOG("get segmented");
        GetChunkSegment((char*)ptr,size);
    }

    // Lock();
    // map->function = ipc32::Function::DeleteChunk;
    // Process();

    return disp;
}

void VstPlugin32::Lock()
{
    if(WaitForSingleObject(ipcMutex, IPC_TIMEOUT) == WAIT_TIMEOUT) {
        LOG("ipcMutex timeout");
        return;
    }
    map->pluginId = GetIndex();
}

void VstPlugin32::Process()
{
    ReleaseSemaphore(ipcSemStart, 1, NULL);
    ReleaseMutex(ipcMutex);
}

void VstPlugin32::ProcessAndWaitResult()
{
    //maybe try to flush ?
    //FlushViewOfFile(map,0);

    ReleaseSemaphore(ipcSemStart, 1, NULL);
    ReleaseMutex(ipcMutex);

    LONG val=0;
    ReleaseSemaphore(ipcSemEnd, 0, &val);
    if(val>0) {
        LOG("semaphore not locked?");
    }

    if(WaitForSingleObject(ipcSemEnd, IPC_TIMEOUT) == WAIT_TIMEOUT) {
        LOG("ipcSemEnd timeout");
        return;
    }

    //ipcSemEnd won't behave, really check if the process is finished :
    //lock, read, unlock.. until the server reset the function to none
    ipc32::Function currentF=ipc32::Function::Wait;
    do {
        if(WaitForSingleObject(ipcMutex, IPC_TIMEOUT) == WAIT_TIMEOUT) {
            LOG("ipcMutex timeout");
            return;
        }

        currentF = map->function;
        ReleaseMutex(ipcMutex);
    } while(currentF != ipc32::Function::None);

    if(WaitForSingleObject(ipcMutex, IPC_TIMEOUT) == WAIT_TIMEOUT) {
        LOG("ipcMutex timeout");
        return;
    }

    if(map->callback == ipc32::Function::SetParam) {
        if(map->cbPluginId == GetIndex()) {
           // LOG("callback " << map->cbPluginId << " " << (int)map->callback << " " << map->cbOpcode << map->cbIndex << map->cbOpt )
            OnMasterCallback(map->cbOpcode,map->cbIndex,map->cbValue,nullptr,map->cbOpt,0);
            map->callback = ipc32::Function::None;
        }
    }
}

void VstPlugin32::UnlockAfterResult()
{
    ReleaseMutex(ipcMutex);
}
