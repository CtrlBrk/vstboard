#include "vstplugin32.h"
#include "mainhost.h"
using namespace Connectables;

HANDLE VstPlugin32::hMapFile = 0;
HANDLE VstPlugin32::ipcMutex = 0;
HANDLE VstPlugin32::ipcSemStart = 0;
HANDLE VstPlugin32::ipcSemEnd = 0;
char* VstPlugin32::chunkData = 0;

QProcess *VstPlugin32::vst32Process = 0;

structPilot* VstPlugin32::st_dataTo32 = 0;
structFrom32* VstPlugin32::dataFrom32 = 0;
Ipc VstPlugin32::st_ipcTo32(L"to32",(void**)&st_dataTo32,sizeof(structPilot)) ;
Ipc VstPlugin32::ipcFrom32(L"from32",(void**)&dataFrom32,sizeof(structFrom32)) ;


VstPlugin32::VstPlugin32(MainHost *myHost,int index, const ObjectInfo & info) :
    VstPlugin(myHost,index,info),
    dataTo32(0),
    ipcTo32(L"to32" + std::to_wstring(GetIndex()), (void**)&dataTo32, sizeof(structTo32)),
    dataBuffers(0),
    ipcBuffers(L"buff" + std::to_wstring(GetIndex()), (void**)&dataBuffers, sizeof(structBuffers))
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


bool VstPlugin32::Load(const std::wstring &name)
{
    if(!vst32Process) {
        vst32Process = new QProcess(myHost);
        connect(vst32Process, SIGNAL(errorOccurred(QProcess::ProcessError)),
                this,SLOT(Vst32Error(QProcess::ProcessError)));

        connect(vst32Process, SIGNAL(finished(int,QProcess::ExitStatus)),
                this, SLOT(Vst32Finished(int,QProcess::ExitStatus)) );

        vst32Process->start("loaddll32.exe");
    }



    st_ipcTo32.LockData();
    st_dataTo32->pluginId = GetIndex();

    st_dataTo32->index = myHost->GetBufferSize();
    st_dataTo32->value = myHost->GetSampleRate();

    st_dataTo32->function=IpcFunction::LoadDll;
    wcscpy(st_dataTo32->name,name.c_str());
    st_ipcTo32.SignalStartAndRelease();
    return true;
}

void VstPlugin32::Vst32Error(QProcess::ProcessError error)
{
    SetErrorMessage( tr("Error while launching the 32bit host")  );
    vst32Process=0;
}

void VstPlugin32::Vst32Finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    SetErrorMessage( tr("32bit hsot process ended")  );
    vst32Process=0;
}

bool VstPlugin32::initPlugin()
{
    ipcTo32.LockData();
    dataTo32->pluginId = GetIndex();

    dataTo32->function=IpcFunction::GetAEffect;

    ipcTo32.SignalStart();

    pEffect = new AEffect();
    pEffect->flags = dataTo32->flags;
    pEffect->numInputs = dataTo32->numInputs;
    pEffect->numOutputs = dataTo32->numOutputs;
    pEffect->initialDelay = dataTo32->initialDelay;
    pEffect->numParams = dataTo32->numParams;

    //disable gui for now
    //pEffect->flags -= effFlagsHasEditor;

    ipcTo32.Unlockdata();

    return VstPlugin::initPlugin();
}


bool VstPlugin32::Unload()
{
    st_ipcTo32.LockData();
    st_dataTo32->pluginId = GetIndex();

    st_dataTo32->function=IpcFunction::UnloadDll;

    st_ipcTo32.SignalStartAndRelease();

    return true;
}
/*
long VstPlugin32::EffEditOpen(void *ptr)
{
    ipcTo32.LockData();
    dataTo32->pluginId = GetIndex();

    dataTo32->function=IpcFunction::EditorOpen;

    ipcTo32.SignalStartAndRelease();

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
    ipcTo32.LockData();
    dataTo32->pluginId = GetIndex();

    dataTo32->function=IpcFunction::EditorShow;

    ipcTo32.SignalStartAndRelease();

}

void VstPlugin32::OnHideEditor()
{
    ipcTo32.LockData();
    dataTo32->pluginId = GetIndex();

    dataTo32->function=IpcFunction::EditorHide;

    ipcTo32.SignalStartAndRelease();
}

float VstPlugin32::EffGetParameter(long index)
{

    ipcTo32.LockData();
    dataTo32->pluginId = GetIndex();

    dataTo32->function=IpcFunction::GetParam;
    dataTo32->index=index;

    ipcTo32.SignalStart();

    float ret = dataTo32->opt;

    ipcTo32.Unlockdata();

    return ret;
}

void VstPlugin32::EffSetParameter(long index, float parameter)
{
    ipcTo32.LockData();
    dataTo32->pluginId = GetIndex();

    dataTo32->function=IpcFunction::SetParam;
    dataTo32->index=index;
    dataTo32->opt = parameter;

    ipcTo32.SignalStartAndRelease();

}

void VstPlugin32::ProcessMidi()
{
    QMutexLocker lock(&midiEventsMutex);


    if(!bWantMidi) return;
    if(listVstMidiEvents.size()==0) return;

    size_t size = listVstMidiEvents.size()*sizeof(VstMidiEvent);

    VstMidiEvent* evt=0;
    char* data = (char*)malloc( size );
    char* dataPtr = data;
    foreach(evt, listVstMidiEvents) {
        memcpy_s(dataPtr,sizeof(VstMidiEvent), evt,sizeof(VstMidiEvent));
        dataPtr+=sizeof(VstMidiEvent);
    }

    EffDispatch(effProcessEvents, listVstMidiEvents.size(), 0, data,0, (VstInt32)size);

    free(data);
    listVstMidiEvents.clear();
}

void VstPlugin32::EffProcess(float **inputs, float **outputs, long sampleframes)
{
    if(sampleframes>IPC_BUFFER_SIZE) {
        LOG("buffer too big");
        return;
    }

    ipcBuffers.LockData();
    dataBuffers->pluginId = GetIndex();

    dataBuffers->function=IpcFunction::Process;
    dataBuffers->dataSize = sampleframes;
    size_t s = sizeof(float)*sampleframes*pEffect->numInputs;
    memcpy(dataBuffers->buffersIn,inputs,s);
    s = sizeof(float)*sampleframes*pEffect->numOutputs;
    memcpy(dataBuffers->buffersOut,outputs,s);

    ipcBuffers.SignalStartAndRelease();
}

void VstPlugin32::EffProcessReplacing(float **inputs, float **outputs, long sampleframes)
{
    ipcFrom32.LockData();
    dataTo32->pluginId = GetIndex();

    if(dataFrom32->callback == IpcFunction::SetParam) {
        if(dataFrom32->cbPluginId == GetIndex()) {
            // LOG("callback " << dataFrom32->cbPluginId << " " << (int)dataFrom32->callback << " " << dataFrom32->cbOpcode << dataFrom32->cbIndex << dataFrom32->cbOpt )
            OnMasterCallback(dataFrom32->cbOpcode,dataFrom32->cbIndex,dataFrom32->cbValue,nullptr,dataFrom32->cbOpt,0);
            dataFrom32->callback = IpcFunction::None;
        }
    }
    ipcFrom32.Unlockdata();

    if(sampleframes>IPC_BUFFER_SIZE) {
        LOG("buffer too big");
        return;
    }

    ipcBuffers.LockData();
    dataBuffers->pluginId = GetIndex();

    dataBuffers->function=IpcFunction::ProcessReplace;
    dataBuffers->dataSize = sampleframes;

    float* tmp = (float*)&dataBuffers->buffersIn;
    for (int i = 0; i < pEffect->numInputs; i++) {
        memcpy(tmp,inputs[i], sizeof(float) * sampleframes );
        tmp += sizeof(float) * sampleframes;
    }

    //size_t s = sizeof(float)*sampleframes*pEffect->numInputs;
    //memcpy(dataBuffers->buffersIn,*inputs,s);
    //s = sizeof(float)*sampleframes*pEffect->numOutputs;
    ipcBuffers.SignalStart();

    //memcpy(*outputs,dataBuffers->buffersOut,s);

    tmp = (float*)&dataBuffers->buffersOut;
    for (int i = 0; i < pEffect->numOutputs; i++) {
        memcpy(outputs[i],tmp, sizeof(float) * sampleframes );
        tmp += sizeof(float) * sampleframes;
    }




   // dataBuffers->callback = IpcFunction::None;

/*
    QString l="";
    for(int a=0;a<sampleframes;a++) {
        l.append( QString::number( outputs[0][a] ) );
    }
    LOG("out " << l)
*/
    ipcBuffers.Unlockdata();
}

void VstPlugin32::EffProcessDoubleReplacing(double **inputs, double **outputs, long sampleframes)
{
#if defined(VST_2_4_EXTENSIONS)

    if(sampleframes>IPC_BUFFER_SIZE) {
        LOG("buffer too big");
        return;
    }

    ipcBuffers.LockData();
    dataBuffers->pluginId = GetIndex();

    dataBuffers->function=IpcFunction::ProcessDouble;
    dataBuffers->dataSize = sampleframes;
    size_t s = sizeof(double)*sampleframes*pEffect->numInputs;
    memcpy(dataBuffers->buffersIn,inputs,s);
    s = sizeof(double)*sampleframes*pEffect->numOutputs;
    memcpy(dataBuffers->buffersOut,outputs,s);

    ipcBuffers.SignalStartAndRelease();

#endif
}

long VstPlugin32::EffGetChunk(void **ptr, bool isPreset)
{
    ipcTo32.LockData();
    dataTo32->pluginId = GetIndex();

    dataTo32->function=IpcFunction::GetChunk;

    ipcTo32.SignalStart();

    VstInt32 chunkSize = dataTo32->dataSize;
    LOG("get chunk size:" << dataTo32->dataSize);

    if(chunkData) {
        delete chunkData;
        chunkData=0;
    }
    chunkData = new char[chunkSize];
    *ptr=chunkData;

    ipcTo32.Unlockdata();

    GetChunkSegment(chunkData,chunkSize);

    // Lock();
    // dataTo32->function = IpcFunction::DeleteChunk;
    // Process();

    return chunkSize;
}

bool VstPlugin32::GetChunkSegment(char *ptr, VstInt32 chunkSize)
{
    VstInt32 start=0;

    while(start<chunkSize) {

        ipcTo32.LockData();
        dataTo32->pluginId = GetIndex();

        dataTo32->function=IpcFunction::GetChunkSegment;
        dataTo32->value=start;
        dataTo32->dataSize = std::min( chunkSize - start , IPC_CHUNK_SIZE);

        ipcTo32.SignalStart();

        memcpy_s(ptr + dataTo32->value,IPC_CHUNK_SIZE,dataTo32->data, std::min( dataTo32->dataSize , IPC_CHUNK_SIZE) );

        ipcTo32.Unlockdata();

        start+=IPC_CHUNK_SIZE;
    }

    return true;
}

bool VstPlugin32::SendChunkSegment(char *ptr, VstInt32 chunkSize)
{
    ipcTo32.LockData();
    dataTo32->pluginId = GetIndex();

    dataTo32->function=IpcFunction::SetChunk;
    dataTo32->dataSize = chunkSize;

    ipcTo32.SignalStartAndRelease();

    VstInt32 start=0;

    while(start<chunkSize) {

        ipcTo32.LockData();

        dataTo32->function=IpcFunction::SetChunkSegment;
        dataTo32->value = start;
        dataTo32->dataSize = std::min( chunkSize - start , IPC_CHUNK_SIZE);
        memcpy_s(dataTo32->data,IPC_CHUNK_SIZE,ptr+start,dataTo32->dataSize);

        ipcTo32.SignalStartAndRelease();

        start+=IPC_CHUNK_SIZE;
    }
    return true;
}

long VstPlugin32::EffDispatch(VstInt32 opCode, VstInt32 index, VstIntPtr value, void *ptr, float opt, VstInt32 size)
{

    if(size>IPC_CHUNK_SIZE) {
        LOG("send segmented " << size << " for opcode " << opCode);
        SendChunkSegment((char*)ptr,size);
    }

    ipcTo32.LockData();
    dataTo32->pluginId = GetIndex();

    dataTo32->function = IpcFunction::Dispatch;
    dataTo32->opCode = opCode;
    dataTo32->index = index;
    dataTo32->value = (VstInt32)value;
    dataTo32->opt = opt;
    //if the data is not already sent segmented
    if(ptr && size<=IPC_CHUNK_SIZE) {
        memcpy_s(&dataTo32->data,IPC_CHUNK_SIZE,ptr,size);
    }
    dataTo32->dataSize=size;



    if(opCode==effSetSpeakerArrangement || opCode==effGetSpeakerArrangement) {
        //value is also a pointer
    }

    ipcTo32.SignalStart();

    long disp = dataTo32->dispatchReturn;
    size = dataTo32->dataSize;
    if(ptr && size<=IPC_CHUNK_SIZE) {
        memcpy(ptr,&dataTo32->data,size);
    }

    ipcTo32.Unlockdata();


    if(size>IPC_CHUNK_SIZE) {
        LOG("get segmented");
        GetChunkSegment((char*)ptr,size);
    }

    // Lock();
    // dataTo32->function = IpcFunction::DeleteChunk;
    // Process();

    return disp;
}

/*
int ltest=0;
void VstPlugin32::Lock()
{
   // LOG(QThread::currentThread())


    if(WaitForSingleObject(ipcMutex, IPC_TIMEOUT) == WAIT_TIMEOUT) {
        LOG("ipcMutex timeout");
        return;
    }

    if(ltest!=0) {
        LOG("lock")
    }
    ltest++;

    dataTo32->pluginId = GetIndex();
}

void VstPlugin32::Process()
{
    if(ltest!=1) {
        LOG("lockProc")
    }
    ltest=0;

    ReleaseSemaphore(ipcSemStart, 1, NULL);
    ReleaseMutex(ipcMutex);
}

void VstPlugin32::ProcessAndWaitResult()
{
    if(ltest!=1) {
        LOG("lockProcWait")
    }
    ltest++;

    //maybe try to flush ?
    //FlushViewOfFile(map,0);

    ReleaseSemaphore(ipcSemStart, 1, NULL);
    ReleaseMutex(ipcMutex);

    if(WaitForSingleObject(ipcSemEnd, IPC_TIMEOUT) == WAIT_TIMEOUT) {
        LOG("ipcSemEnd timeout");
        return;
    }

    //ipcSemEnd won't behave, really check if the process is finished :
    //lock, read, unlock.. until the server reset the function to none
    IpcFunction currentF=IpcFunction::Wait;
    do {
        if(WaitForSingleObject(ipcMutex, IPC_TIMEOUT) == WAIT_TIMEOUT) {
            LOG("ipcMutex timeout");
            return;
        }

        currentF = dataTo32->function;
        ReleaseMutex(ipcMutex);
    } while(currentF != IpcFunction::None);

    if(WaitForSingleObject(ipcMutex, IPC_TIMEOUT) == WAIT_TIMEOUT) {
        LOG("ipcMutex timeout");
        return;
    }

    if(dataTo32->callback == IpcFunction::SetParam) {
        if(dataTo32->cbPluginId == GetIndex()) {
           // LOG("callback " << dataTo32->cbPluginId << " " << (int)dataTo32->callback << " " << dataTo32->cbOpcode << dataTo32->cbIndex << dataTo32->cbOpt )
            OnMasterCallback(dataTo32->cbOpcode,dataTo32->cbIndex,dataTo32->cbValue,nullptr,dataTo32->cbOpt,0);
            dataTo32->callback = IpcFunction::None;
        }
    }
}

void VstPlugin32::UnlockAfterResult()
{
    if(ltest!=2) {
        LOG("unlock")
    }
    ltest=0;

    ReleaseMutex(ipcMutex);
}
*/
