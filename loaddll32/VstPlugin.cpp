#include "VstPlugin.h"
#include "VstHost.h"
#include "ipcvst.h"
#include <iostream>
#include "vstwindow.h"

VstPlugin::VstPlugin(IpcVst* i, int id) :
    pEffect(0),
    bEditOpen(false),
    bNeedIdle(false),
    bInEditIdle(false),
    bWantMidi(false),
    bInSetProgram(false),
    pMasterEffect(0),
    pluginLib(0),
    ipc(i),
    pluginId(id),
    chunk(0),
    chunkIn(0),
    win(0),
    listEvnts(0),
    chunkInSize(0),
    dataIn(0),
    ipcIn(L"to32" + to_wstring(id), (void**)&dataIn, sizeof(structTo32)),
    dataBuffers(0),
    ipcBuffers(L"buff" + to_wstring(id), (void**)&dataBuffers, sizeof(structBuffers)),
    parentWindow(0),
    closing(false)
{
   // cout << "vstplugin " << this << " ipc " << &ipc << endl;
    sName.clear();
}

/*****************************************************************************/
/* ~VstPlugin : destructor                                                     */
/*****************************************************************************/

VstPlugin::~VstPlugin()
{
    closing = true;

//    processThread->join();
 //   guiThread->join();

    if (listEvnts) {
        free(listEvnts);
        listEvnts = 0;
    }

    if (chunkIn) {
        delete chunkIn;
    }

    if (win) {
        delete win;
    }
    Unload();

    delete processThread;
   // delete guiThread;
}

/*****************************************************************************/
/* Load : loads the effect module                                            */
/*****************************************************************************/

bool VstPlugin::Load(const std::wstring& name,float sampleRate, int blocksize)
{
    const wchar_t* buf = name.c_str();

    pluginLib = LoadLibrary(name.c_str());
    if (!pluginLib) {
        return false;
    }

    vstPluginFuncPtr entryPoint = 0;
    try {
        entryPoint = (vstPluginFuncPtr)GetProcAddress(pluginLib, "VSTPluginMain");
        if (!entryPoint) {
            entryPoint = (vstPluginFuncPtr)GetProcAddress(pluginLib, "main");
        }
    }
    catch (...)
    {
        pEffect = NULL;
    }
    if (!entryPoint) {
        Unload();
        return false;
    }

    try
    {
        pEffect = entryPoint(&CVSTHost::AudioMasterCallback);
    }
    catch (...)
    {
        pEffect = NULL;
    }

    if (!pEffect) {
        Unload();
        return false;
    }

    if (pEffect->magic != kEffectMagic) {
        Unload();
        return false;
    }
    sName = name;
    pEffect->user = this;
  //  cout << "peffect " << &pEffect << endl;
   
    
    EffDispatch(effOpen);
    EffDispatch(effSetSampleRate, 0, 0, 0, sampleRate);
    EffDispatch(effSetBlockSize, 0, blocksize);
#ifdef VSTSDK
    EffDispatch(effSetProcessPrecision, 0, kVstProcessPrecision32);
#endif
    EffDispatch(effMainsChanged, 0, 1);
    EffDispatch(effStartProcess);
    /*
    if (getFlags(effFlagsHasEditor)) {
        EditOpen();

        EffDispatch(effEditOpen, 0, 0, win->hWin);
        RECT rc{};
        ERect* erc = nullptr;
        EffDispatch(effEditGetRect, 0, 0, &erc);
        rc.left = erc->left;
        rc.top = erc->top;
        rc.right = erc->right;
        rc.bottom = erc->bottom;
        win->resizeEditor(rc);
       // ShowWindow(win->hWin, SW_SHOW);
    }
    */
    //running in a separate thread locks some gui (guitar rig)
   // guiThread = new std::thread(RunPlugin, this);
   // guiThread->detach();

    processThread = new std::thread(RunPluginBuffer, this);
    processThread->detach();

    return true;
}


void VstPlugin::RunPluginBuffer(VstPlugin* p) {
    while (!p->closing) {
        p->BuffersLoop();
    }
}
/*
void VstPlugin::RunPlugin(VstPlugin* p) {
    
    MSG msg;

    while (!p->closing) {
        
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                break;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        p->MsgLoop();
        
    }
}
*/
/*****************************************************************************/
/* Unload : unloads effect module                                            */
/*****************************************************************************/

bool VstPlugin::Unload()
{
    if (pEffect)
        EffDispatch(effClose);

    pEffect = NULL;

    if (pluginLib) {
        // if(pluginLib->isLoaded())
            // if(!pluginLib->unload()) {
                // LOG("can't unload plugin"<< sName);
            // }
        // delete pluginLib;
        FreeLibrary(pluginLib);
        pluginLib = 0;
    }
    return true;
}

/*****************************************************************************/
/* LoadBank : loads a .fxb file ... IF it's for this effect                  */
/*****************************************************************************/

bool VstPlugin::LoadBank(std::string* name)
{
    /*
    try
    {
        CFxBank bank(name);

        if (!bank.IsLoaded())
            throw (int)1;

        if (pEffect->uniqueID != bank.GetFxID()) {
            LOG("LoadBank ID doesn't match");
            QMessageBox msgBox;
            msgBox.setText(QObject::tr("Wrong plugin ID."));
            msgBox.setInformativeText(QObject::tr("Bank file not designed for that plugin"));
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.exec();
            return false;
        }

        EffBeginLoadBank(bank.GetPatchChunkInfo());

        if (bank.IsChunk()) {
            if (!(pEffect->flags & effFlagsProgramChunks)) {
                LOG("LoadBank chunk not handled");
                throw (int)1;
            }
            EffSetChunk(bank.GetChunk(), bank.GetChunkSize(), 0);

        }
        else {

            //pEffect->numPrograms=bank.GetNumPrograms();

            for (int i = 0; i < bank.GetNumPrograms(); i++) {
                SFxProgram* prog = bank.GetProgram(i);

                VstPatchChunkInfo info;
                info = *bank.GetPatchChunkInfo();
                info.numElements = prog->numParams;
                EffBeginLoadProgram(&info);
                EffSetProgram(i);
                EffSetProgramName(prog->prgName);
                for (int j = 0; j < prog->numParams; j++) {
                    float val = bank.GetProgParm(i, j);
                    EffSetParameter(j, val);
                }
            }
            EffSetProgram(0);

        }
    }
    catch (...)
    {
        return false;
    }
    */
    return true;
}

/*****************************************************************************/
/* SaveBank : saves current sound bank to a .fxb file                        */
/*****************************************************************************/

bool VstPlugin::SaveBank(std::string* name)
{
    /*
    CFxBank bank;

    if (pEffect->flags & effFlagsProgramChunks) {
        void* ptr = 0;
        long size = EffGetChunk(&ptr, false);
        bank.SetSize(size);
        bank.SetChunk(ptr);
    }
    else {
        bank.SetSize(pEffect->numPrograms, pEffect->numParams);
        long oldPrg = EffGetProgram();
        for (int i = 0; i < pEffect->numPrograms; i++) {
            char prgName[24];
            EffSetProgram(i);
            EffGetProgramName(prgName);
            bank.SetProgramName(i, prgName);

            for (int j = 0; j < pEffect->numParams; j++) {
                bank.SetProgParm(i, j, EffGetParameter(j));
            }
        }
        EffSetProgram(oldPrg);
    }

    bank.SetFxID(pEffect->uniqueID);
    bank.SetFxVersion(pEffect->version);
    bank.SaveBank(name);
    */
    return false;
}

bool VstPlugin::LoadProgram(std::string* name)
{
    /*
    try
    {
        CFxProgram progFile(name);

        if (!progFile.IsLoaded())
            throw (int)1;

        if (pEffect->uniqueID != progFile.GetFxID()) {
            LOG("LoadPreset ID doesn't match");
            QMessageBox msgBox;
            msgBox.setText(QObject::tr("Wrong plugin ID."));
            msgBox.setInformativeText(QObject::tr("Program file not designed for that plugin"));
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.exec();
            return false;
        }

        if (progFile.IsChunk()) {
            if (!(pEffect->flags & effFlagsProgramChunks)) {
                LOG("LoadPreset chunk not handled");
                throw (int)1;
            }
            EffSetChunk(progFile.GetChunk(), progFile.GetChunkSize(), 0);

        }
        else {
            SFxProgram* prog = progFile.GetProgram();

            VstPatchChunkInfo info;
            info = *progFile.GetPatchChunkInfo();
            info.numElements = prog->numParams;
            EffBeginLoadProgram(&info);
            EffSetProgramName(prog->prgName);
            for (int i = 0; i < prog->numParams; i++) {
                float val = progFile.GetProgParm(i);
                EffSetParameter(i, val);
            }
        }

    }
    catch (...)
    {
        return false;
    }
    */
    return true;
}

bool VstPlugin::SaveProgram(std::string* name)
{
    /*
    CFxProgram progFile;

    if (pEffect->flags & effFlagsProgramChunks) {
        void* ptr = 0;
        long size = EffGetChunk(&ptr, false);
        progFile.SetChunkSize(size);
        progFile.SetChunk(ptr);
    }
    else {
        progFile.SetSize(pEffect->numParams);
        for (int i = 0; i < pEffect->numParams; i++) {
            progFile.SetProgParm(i, EffGetParameter(i));
        }
    }

    char prgName[24];
    EffGetProgramName(prgName);
    progFile.SetProgramName(prgName);

    progFile.SetFxID(pEffect->uniqueID);
    progFile.SetFxVersion(pEffect->version);
    progFile.SaveProgram(name);
    */
    return false;
}

int VstPlugin::PluginIdFromBankFile(std::string* name)
{
    /*
    return CFxBank::PluginIdFromBank(name);
    */
    return 0;
}

/*****************************************************************************/
/* EffDispatch : calls an effect's dispatcher                                */
/*****************************************************************************/

long VstPlugin::EffDispatch(int opCode, int index, int value, void* ptr, float opt)
{
    if (!pEffect)
        return 0;

    long disp = 0L;
    disp = pEffect->dispatcher(pEffect, opCode, index, value, ptr, opt);

    //that's odd
    if (opCode == effCanDo) {
        return bWantMidi;
    }

    return disp;
}

/*****************************************************************************/
/* EffProcess : calles an effect's process() function                        */
/*****************************************************************************/

void VstPlugin::EffProcess(float** inputs, float** outputs, long sampleframes)
{
    pEffect->process(pEffect, inputs, outputs, sampleframes);
}

/*****************************************************************************/
/* EffProcessReplacing : calls an effect's processReplacing() function       */
/*****************************************************************************/

void VstPlugin::EffProcessReplacing(float** inputs, float** outputs, long sampleframes)
{
    pEffect->processReplacing(pEffect, inputs, outputs, sampleframes);
}

/*****************************************************************************/
/* EffProcessDoubleReplacing : calls an effect's processDoubleReplacing() f. */
/*****************************************************************************/

void VstPlugin::EffProcessDoubleReplacing(double** inputs, double** outputs, long sampleFrames)
{
#if defined(VST_2_4_EXTENSIONS)
    pEffect->processDoubleReplacing(pEffect, inputs, outputs, sampleFrames);
#endif
}


int VstPlugin::OnMasterCallback(long opcode, long index, long value, void* ptr, float opt, long currentReturnCode)
{
   // cout << "callback " << this << " ipc " << &ipc << endl;
    switch (opcode) {
    case audioMasterWantMidi: //6
        bWantMidi = true;
        return true;
    }

    return ipc->onCallback(pluginId, opcode, index, value, ptr, opt, currentReturnCode);
}

long VstPlugin::EffGetParamName(long index, char* txt)
{
    return EffDispatch(effGetParamName, index, 0, txt);
}


void VstPlugin::CrtVstWin() {
    if (win) {
        return;
    }
    cout << "crt window " << pluginId << endl;
    win = new VstWin(this);

    win->hWin = win->CrtWindow();
    //SetParent(win->hWin, parentWindow);
    
    
    //ShowWindow(win->hWin, SW_SHOW);
    VstRect* vstRect = nullptr;
    EffDispatch(effEditGetRect, 0, 0, &vstRect);
    if (EffDispatch(effEditOpen, 0, 0, win->hWin) == 1L) {
        EffDispatch(effEditGetRect, 0, 0, &vstRect);
        RECT rc{};
        if (vstRect) {
            rc.left = vstRect->left;
            rc.top = vstRect->top;
            rc.right = vstRect->right;
            rc.bottom = vstRect->bottom;
        }
        win->resizeEditor(rc);
    }
}

//void CrtVstWin(VstPlugin* vst) {
//    vst->CrtVstWin();
//}

bool VstPlugin::EditOpen() {
    //std::thread vstwin(CrtVstWin,this);
    //vstwin.detach();
        
    CrtVstWin();
    return true;
}

bool VstPlugin::EditClose() {
    if (!win) return false;
    cout << "del window " << pluginId << endl;
    delete(win);
    win = 0;
    return true;
}

void VstPlugin::BuffersLoop() {
    
    if (!ipcBuffers.IfWaitingToStart()) {
        return;
    }


    float** ins = 0;
    float** outs = 0;
    float* tmp = 0;

    switch (dataBuffers->function) {
    case IpcFunction::Process:
    case IpcFunction::ProcessReplace:
        if (pEffect) {
            ins = new float* [pEffect->numInputs];
            outs = new float* [pEffect->numOutputs];

            tmp = (float*)&dataBuffers->buffersIn;
            for (int i = 0; i < pEffect->numInputs; i++) {
                ins[i] = tmp;
                tmp += sizeof(float) * dataBuffers->dataSize;
            }
            tmp = (float*)&dataBuffers->buffersOut;
            for (int i = 0; i < pEffect->numOutputs; i++) {
                outs[i] = tmp;
                tmp += sizeof(float) * dataBuffers->dataSize;
            }

            if (dataBuffers->function == IpcFunction::Process) {
                pEffect->process(pEffect, ins, outs, dataBuffers->dataSize);
            }
            if (dataBuffers->function == IpcFunction::ProcessReplace) {
                pEffect->processReplacing(pEffect, ins, outs, dataBuffers->dataSize);
            }

            delete[] ins;
            delete[] outs;
        }
        break;
    case IpcFunction::ProcessDouble:

        break;
    }

    ipcBuffers.SignalResultReady();

}

void VstPlugin::MsgLoop()
{
    if (!ipcIn.IfWaitingToStart()) {
        return;
    }

    switch (dataIn->function) {
        case IpcFunction::UnloadDll:
            Unload();
            break;
        case IpcFunction::GetAEffect:
            parentWindow = (HWND)dataIn->mainWin;
            if (pEffect) {
                dataIn->flags = pEffect->flags;
                dataIn->numInputs = pEffect->numInputs;
                dataIn->numOutputs = pEffect->numOutputs;
                dataIn->initialDelay = pEffect->initialDelay;
                dataIn->numParams = pEffect->numParams;
            }
            break;
        case IpcFunction::EditorOpen:
            EditOpen();
            break;
        case IpcFunction::EditorShow:
            if (win) {
                //if (dataIn->mainWin) {
                //    parentWindow = (HWND)dataIn->mainWin;
                //    SetParent(win->hWin, parentWindow);
                //}
                ShowWindow(win->hWin, SW_SHOW);
                //BringWindowToTop(win->hWin);
                //SetForegroundWindow(win->hWin);
            }
            break;
        case IpcFunction::EditorHide:
            if (win) {
                //if (dataIn->mainWin) {
                //    parentWindow = (HWND)dataIn->mainWin;
                //    SetParent(win->hWin, parentWindow);
                //}
                ShowWindow(win->hWin, SW_HIDE);
            }
            break;
        case IpcFunction::GetParam:
            if (pEffect) {
                dataIn->opt = pEffect->getParameter(pEffect, dataIn->index);
            }
            break;
        case IpcFunction::SetParam:
            if (pEffect) {
                pEffect->setParameter(pEffect, dataIn->index, dataIn->opt);
            }
            break;
        

        case IpcFunction::GetChunk:
            if (pEffect) {
                dataIn->dataSize = EffGetChunk(&chunk, false);
               // cout << "get chunk size:" << dataIn->dataSize << endl;
            }
            break;
        case IpcFunction::GetChunkSegment:
            cout << "get segment start:" << dataIn->value << " size:" << dataIn->dataSize << endl;
            memcpy_s(dataIn->data, IPC_CHUNK_SIZE, (char*)chunk + dataIn->value, dataIn->dataSize);

            break;
        case IpcFunction::SetChunk:
            cout << "set chunk size:" << dataIn->dataSize << endl;
            if (chunkIn) {
                delete chunkIn;
                chunkIn = 0;
            }
            chunkIn = new char[dataIn->dataSize];
            chunkInSize = dataIn->dataSize;
            break;
        case IpcFunction::SetChunkSegment:
            cout << "set segment start:" << dataIn->value << " size:" << dataIn->dataSize << endl;
            memcpy_s((char*)chunkIn + dataIn->value, IPC_CHUNK_SIZE, dataIn->data, min(dataIn->dataSize, IPC_CHUNK_SIZE));
            break;
        case IpcFunction::DeleteChunk:
            if (chunk) {
                delete[] chunk;
                chunk = 0;
            }
            break;
        case IpcFunction::Dispatch:
            
            if (dataIn->opCode == effEditOpen) {
                EditOpen();
                break;
            }
            if (dataIn->opCode == effEditClose) {
                EditClose();
                break;
            }
                           
            if (dataIn->opCode == effProcessEvents) {
                if (chunkIn) {
                    TranslateMidiEvents(dataIn, chunkIn, chunkInSize);
                }
                else {
                    TranslateMidiEvents(dataIn, dataIn->data, dataIn->dataSize);
                }
            }

            //	cout << "optcode " << dataIn->opCode << endl;
            if (chunkIn) {
                
                //cout << "use chunk" << endl;
                dataIn->dispatchReturn = EffDispatch(dataIn->opCode, dataIn->index, dataIn->value, chunkIn, dataIn->opt);
                delete[] chunkIn;
                chunkIn = 0;
            }
            else {

                
                dataIn->dispatchReturn = EffDispatch(dataIn->opCode, dataIn->index, dataIn->value, dataIn->data, dataIn->opt);
            }

            //set size to 0 when the returned data is not used
#ifdef VSTSDK
            switch (dataIn->opCode) {
            case effSetChunk:
                dataIn->dataSize = 0;
            }
#endif
            
    }
    dataIn->function = IpcFunction::None;

    ipcIn.SignalResultReady();
}

void VstPlugin::TranslateMidiEvents(structTo32* map, void* data, int datasize)
{
    /*
       //probably memory leaks, each event should be deleted ?

        */
    size_t size = sizeof(VstEvents) + datasize;
    listEvnts = (VstEvents*)malloc(size);
    listEvnts->numEvents = map->index;
    listEvnts->reserved = 0;

    int cpt = 0;
    int ptr = 0;
    while (ptr < map->dataSize) {
        VstEvent* e = new VstEvent();
        memcpy_s(e, sizeof(VstEvent), (char*)data + ptr, sizeof(VstEvent));
        listEvnts->events[cpt] = e;

        cpt++;
        ptr += sizeof(VstMidiEvent);
    }

    delete chunkIn;
    chunkIn = listEvnts;
    listEvnts = 0;
    
}