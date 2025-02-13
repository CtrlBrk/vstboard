#include "VstPlugin.h"
#include "VstHost.h"
#include "ipcvst.h"
#include <iostream>
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
    hWin(nullptr),
    pluginId(id)
{
   // cout << "vstplugin " << this << " ipc " << &ipc << endl;
    sName.clear();
}

/*****************************************************************************/
/* ~VstPlugin : destructor                                                     */
/*****************************************************************************/

VstPlugin::~VstPlugin()
{
    Unload();
}

/*****************************************************************************/
/* Load : loads the effect module                                            */
/*****************************************************************************/

bool VstPlugin::Load(const std::wstring& name)
{
    pluginLib = LoadLibrary(name.c_str());
    if (!pluginLib) {
        return false;
    }

    vstPluginFuncPtr entryPoint = 0;
    try {
        entryPoint = (vstPluginFuncPtr)GetProcAddress(pluginLib, "VSTPluginMain");
        // entryPoint = (vstPluginFuncPtr)pluginLib->resolve("VSTPluginMain");
        if (!entryPoint) {
            entryPoint = (vstPluginFuncPtr)GetProcAddress(pluginLib, "main");
            // entryPoint = (vstPluginFuncPtr)pluginLib->resolve("main");
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
    return true;
}

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

VstInt32 VstPlugin::PluginIdFromBankFile(std::string* name)
{
    /*
    return CFxBank::PluginIdFromBank(name);
    */
    return 0;
}

/*****************************************************************************/
/* EffDispatch : calls an effect's dispatcher                                */
/*****************************************************************************/

long VstPlugin::EffDispatch(VstInt32 opCode, VstInt32 index, VstIntPtr value, void* ptr, float opt)
{
    if (!pEffect)
        return 0;

    long disp = 0L;
    disp = pEffect->dispatcher(pEffect, opCode, index, value, ptr, opt);

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

/*****************************************************************************/
/* EffSetParameter : calls an effect's setParameter() function               */
/*****************************************************************************/

void VstPlugin::EffSetParameter(long index, float parameter)
{
    pEffect->setParameter(pEffect, index, parameter);
}

/*****************************************************************************/
/* EffGetParameter : calls an effect's getParameter() function               */
/*****************************************************************************/

float VstPlugin::EffGetParameter(long index)
{
    return pEffect->getParameter(pEffect, index);
}

VstIntPtr VstPlugin::OnMasterCallback(long opcode, long index, long value, void* ptr, float opt, long currentReturnCode)
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


void CrtVstWin(VstPlugin* vst) {
    /*
    win = new VstWin();
    vst->hWin = win->CrtWindow();



    //while (GetMessage(&msg, vst->hWin, 0, 0) != 0)
    //{
        //TranslateMessage(&msg);
        //DispatchMessage(&msg);
    //}
    */
    WNDCLASSEX wcex{ sizeof(wcex) };
    wcex.lpfnWndProc = DefWindowProc;
    wcex.hInstance = GetModuleHandle(0);
    wcex.lpszClassName = L"from github.com/t-mat/";
    RegisterClassEx(&wcex);
    const auto style = WS_CAPTION | WS_THICKFRAME | WS_OVERLAPPEDWINDOW;
    vst->hWin = CreateWindow(
        wcex.lpszClassName, L"Vst32", style
        , 0, 0, 0, 0, NULL, 0, 0, 0
    );
    /*
    MSG msg;
    while (BOOL b = GetMessage(&msg, 0, 0, 0)) {
        if (b == -1) {
            //run = false;
            break;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    */
}

void VstPlugin::resizeEditor(const RECT& clientRc) const {
    if (hWin) {
        auto rc = clientRc;
        const auto style = GetWindowLongPtr(hWin, GWL_STYLE);
        const auto exStyle = GetWindowLongPtr(hWin, GWL_EXSTYLE);
        const BOOL fMenu = GetMenu(hWin) != nullptr;
        AdjustWindowRectEx(&rc, style, fMenu, exStyle);
        MoveWindow(hWin, 0, 0, rc.right - rc.left, rc.bottom - rc.top, TRUE);
    }
}

bool VstPlugin::EditOpen() {
    //std::thread vstwin(CrtVstWin,this);
    //vstwin.detach();
    //while (hWin == 0) {};
    CrtVstWin(this);

    //return true;
    //VstWin* win = new VstWin();
    //HWND hWin = win->CrtWindow();

    //bool windowOk = false;
    //ERect* rect = 0;

    //if (hWin) {
    //	ShowWindow(hWin, SW_SHOW);
    //}
    if (!hWin) return false;

    ERect* erc = nullptr;
    EffDispatch(effEditGetRect, 0, 0, &erc);
    if (EffDispatch(effEditOpen, 0, 0, hWin) == 1L) {
        EffDispatch(effEditGetRect, 0, 0, &erc);
        RECT rc{};
        if (erc) {
            rc.left = erc->left;
            rc.top = erc->top;
            rc.right = erc->right;
            rc.bottom = erc->bottom;
        }
        resizeEditor(rc);

    }
    /*
    if (plugin->EffEditGetRect(&rect) == 1L) windowOk = true;
    if (plugin->EffEditOpen(hWin) == 1L) windowOk = true;
    if (plugin->EffEditGetRect(&rect) == 1L) windowOk = true;	//try to get rect again
    if (!windowOk) return false;
    */
    //SetWindowSize(rect->right, rect->bottom);
    return true;
}

bool VstPlugin::EditClose() {
    if (!hWin) return false;
    DestroyWindow(hWin);
    hWin = 0;
    return true;
}

void VstPlugin::MsgLoop(ipc32* map)
{
    if (!map) return;

    float** ins = 0;
    float** outs = 0;
    float* tmp = 0;

    void* chunk = 0;

    switch (map->function) {
        case ipc32::Function::UnloadDll:
            Unload();
            break;
        case ipc32::Function::GetAEffect:
            if (pEffect) {
                map->flags = pEffect->flags;
                map->numInputs = pEffect->numInputs;
                map->numOutputs = pEffect->numOutputs;
                map->initialDelay = pEffect->initialDelay;
                map->numParams = pEffect->numParams;
            }
            break;
        //case ipc32::Function::EditorOpen:
        //    EditOpen();
        //    break;
        case ipc32::Function::EditorShow:
            if (hWin) {
                ShowWindow(hWin, SW_SHOW);
            }
            break;
        case ipc32::Function::EditorHide:
            if (hWin) {
                ShowWindow(hWin, SW_HIDE);
            }
            break;
        case ipc32::Function::GetParam:
            if (pEffect) {
                map->opt = pEffect->getParameter(pEffect, map->index);
            }
            break;
        case ipc32::Function::SetParam:
            if (pEffect) {
                pEffect->setParameter(pEffect, map->index, map->opt);
            }
            break;
        case ipc32::Function::Process:
        case ipc32::Function::ProcessReplace:
            if (pEffect) {
                ins = new float* [pEffect->numInputs];
                outs = new float* [pEffect->numOutputs];

                tmp = (float*)&map->buffersIn;
                for (int i = 0; i < pEffect->numInputs; i++) {
                    ins[i] = tmp;
                    tmp += sizeof(float) * map->dataSize;
                }
                tmp = (float*)&map->buffersOut;
                for (int i = 0; i < pEffect->numOutputs; i++) {
                    outs[i] = tmp;
                    tmp += sizeof(float) * map->dataSize;
                }

                if (map->function == ipc32::Function::Process) {
                    pEffect->process(pEffect, ins, outs, map->dataSize);
                }
                if (map->function == ipc32::Function::ProcessReplace) {
                    pEffect->processReplacing(pEffect, ins, outs, map->dataSize);
                }

                delete[] ins;
                delete[] outs;
            }
            break;
        case ipc32::Function::ProcessDouble:

            break;

        case ipc32::Function::GetChunk:
            if (pEffect) {
                map->dataSize = EffGetChunk(&chunk, false);
                cout << "get chunk size:" << map->dataSize << endl;
            }
            break;
        case ipc32::Function::GetChunkSegment:
            cout << "get segment start:" << map->value << " size:" << map->dataSize << endl;
            memcpy_s(map->data, IPC_CHUNK_SIZE, (char*)chunk + map->value, map->dataSize);

            break;
        case ipc32::Function::SetChunk:
            cout << "set chunk size:" << map->dataSize << endl;
            chunk = new char[map->dataSize];
            break;
        case ipc32::Function::SetChunkSegment:
            cout << "set segment start:" << map->value << " size:" << map->dataSize << endl;
            memcpy_s((char*)chunk + map->value, IPC_CHUNK_SIZE, map->data, min(map->dataSize, IPC_CHUNK_SIZE));
            break;
        case ipc32::Function::DeleteChunk:
            if (chunk) {
                delete chunk;
                chunk = 0;
            }
            break;
        case ipc32::Function::Dispatch:
            
            if (map->opCode == effEditOpen) {
                EditOpen();
                break;
            }
            if (map->opCode == effEditClose) {
                EditClose();
                break;
            }
            //	cout << "optcode " << map->opCode << endl;
                //we have segemented data, use it
            if (chunk) {
                cout << "use chunk" << endl;
                map->dispatchReturn = EffDispatch(map->opCode, map->index, map->value, chunk, map->opt);
                //delete chunk;
                //chunk = 0;
            }
            else {
                map->dispatchReturn = EffDispatch(map->opCode, map->index, map->value, map->data, map->opt);
            }
          
        }
    
}