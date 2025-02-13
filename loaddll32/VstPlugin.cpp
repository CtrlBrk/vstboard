#include "VstPlugin.h"
#include "VstHost.h"
#include "ipcvst.h"
#include <iostream>
VstPlugin::VstPlugin(IpcVst* i) :
    pEffect(0),
    bEditOpen(false),
    bNeedIdle(false),
    bInEditIdle(false),
    bWantMidi(false),
    bInSetProgram(false),
    pMasterEffect(0),
    pluginLib(0),
    ipc(i)
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

    return ipc->onCallback(opcode, index, value, ptr, opt, currentReturnCode);
}

long VstPlugin::EffGetParamName(long index, char* txt)
{
    return EffDispatch(effGetParamName, index, 0, txt);
}

