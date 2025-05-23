/******************************************************************************
#   Copyright 2006 Hermann Seib
#   Copyright 2010 Raphaël François
#
#    This file is part of VstBoard.
#
#    VstBoard is free software: you can redistribute it and/or modify
#    it under the terms of the under the terms of the GNU Lesser General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    VstBoard is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    under the terms of the GNU Lesser General Public License for more details.
#
#    You should have received a copy of the under the terms of the GNU Lesser General Public License
#    along with VstBoard.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#ifndef CEFFECT_H
#define CEFFECT_H

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning (disable: 4100)
#endif

//#include "../precomp.h"
#include "const.h"
#include <string>

//#ifdef _MSC_VER
//#pragma warning( push, 1 )
//#endif
//#include "public.sdk/source/vst2.x/audioeffectx.h"
//#ifdef _MSC_VER
//#pragma warning( pop )
//#endif


// class MainHost;
namespace vst
{
    class CVSTHost;
    class CEffect
    {
    public:
        CEffect();
        virtual ~CEffect();

    public:
        AEffect *pEffect;
        bool bEditOpen;
        bool bNeedIdle;
        bool bInEditIdle;
        bool bWantMidi;
        bool bInSetProgram;
        CEffect *pMasterEffect;             /* for Shell-type plugins            */
        std::wstring sName;

    #ifdef WIN32

    #elif MAC

    // yet to do
    // no idea how things look here...

    #endif

        virtual bool Load(const std::wstring &name);
        bool Unload();
#ifdef VST24SDK
        virtual bool LoadBank(std::string *name);
        virtual bool SaveBank(std::string *name);

        virtual bool LoadVstProgram(std::string *name);
        virtual bool SaveVstProgram(std::string *name);

        static int PluginIdFromBankFile(std::string *name);
#endif
        virtual long EffDispatch(int opCode, int index=0, intptr_t value=0, void *ptr=0, float opt=0., int size=0);
        virtual intptr_t OnMasterCallback(long opcode, long index, long value, void *ptr, float opt,long currentReturnCode);

        virtual void EffProcess(float **inputs, float **outputs, long sampleframes);
        virtual void EffProcessReplacing(float **inputs, float **outputs, long sampleframes);
        virtual void EffProcessDoubleReplacing(double** inputs, double** outputs, long sampleFrames);
        virtual void EffSetParameter(long index, float parameter);
        virtual float EffGetParameter(long index);

        virtual void EffOpen() { EffDispatch(effOpen); }
        void EffClose() { EffDispatch(effClose); }
        virtual void EffSetProgram(long lValue) { EffBeginSetProgram(); EffDispatch(effSetProgram, 0, lValue); EffEndSetProgram(); }
        virtual long EffGetProgram() { return EffDispatch(effGetProgram); }
        virtual void EffSetProgramName(char *ptr, int size) { EffDispatch(effSetProgramName, 0, 0, ptr,0,size); }
        virtual void EffGetProgramName(char *ptr, int size) { EffDispatch(effGetProgramName, 0, 0, ptr,0,size); }
        virtual void EffGetParamLabel(long index, char *ptr, int size) { EffDispatch(effGetParamLabel, index, 0, ptr,0,size); }
        virtual void EffGetParamDisplay(long index, char *ptr, int size) { EffDispatch(effGetParamDisplay, index, 0, ptr,0,size); }
        virtual long EffGetParamName(long index,char *txt, int size) { return EffDispatch(effGetParamName, index, 0, txt,0,size); }
        virtual void EffSetSampleRate(float fSampleRate) { EffDispatch(effSetSampleRate, 0, 0, 0, fSampleRate); }
        virtual void EffSetBlockSize(long value) { EffDispatch(effSetBlockSize, 0, value); }
        virtual void EffMainsChanged(bool bOn) { EffDispatch(effMainsChanged, 0, bOn); }
        virtual void EffSuspend() { EffDispatch(effMainsChanged, 0, false); }
        virtual void EffResume() { EffDispatch(effMainsChanged, 0, true); }
        // virtual float EffGetVu() { return (float)EffDispatch(effGetVu) / (float)32767.; }
        virtual long EffEditGetRect(VstRect **ptr) { return EffDispatch(effEditGetRect, 0, 0, ptr); }
        virtual long EffEditOpen(void *ptr) { long l = EffDispatch(effEditOpen, 0, 0, ptr); /* if (l > 0) */ bEditOpen = true; return l; }
        virtual void EffEditClose() { if(!bEditOpen) return; EffDispatch(effEditClose); bEditOpen = false; }
        virtual void EffEditIdle() { if (!bEditOpen || bInEditIdle) return; bInEditIdle = true; EffDispatch(effEditIdle); bInEditIdle = false; }
    #if MAC
        virtual void EffEditDraw(void *ptr) { EffDispatch( effEditDraw, 0, 0, ptr); }
        virtual long EffEditMouse(long index, long value) { return EffDispatch( effEditMouse, index, value); }
        virtual long EffEditKey(long value) { return EffDispatch(effEditKey, 0, value); }
        virtual void EffEditTop() { EffDispatch(effEditTop); }
        virtual void EffEditSleep() { EffDispatch(effEditSleep); }
    #endif
        virtual long EffIdentify() { return EffDispatch(effIdentify); }
        //as const
        virtual long EffGetChunk(void **ptr, bool isPreset = false)
        {   if (!pEffect)
                return 0;

            long disp=0L;
            disp = pEffect->dispatcher(pEffect, effGetChunk, isPreset, 0, ptr, .0);
            return disp;
          //  return EffDispatch(effGetChunk, isPreset, 0, ptr);
        }
        virtual long EffSetChunk(void *data, long byteSize, bool isPreset = false) { EffBeginSetProgram(); long lResult = EffDispatch(effSetChunk, isPreset, byteSize, data,0,byteSize); EffEndSetProgram(); return lResult;}
                                            /* VST 2.0                           */
        virtual long EffProcessEvents(VstEvents* ptr, int size) { return EffDispatch(effProcessEvents, 0, 0, ptr,0, size); }
        virtual long EffCanBeAutomated(long index) { return EffDispatch(effCanBeAutomated, index); }
        // virtual long EffString2Parameter(long index, char *ptr, int size) { return EffDispatch(effString2Parameter, index, 0, ptr,0, size); }
        // virtual long EffGetNumProgramCategories() { return EffDispatch(effGetNumProgramCategories); }
        virtual long EffGetProgramNameIndexed(long category, long index, char* text, int size) { return EffDispatch(effGetProgramNameIndexed, index, category, text,0, size); }
        // virtual long EffCopyProgram(long index) { return EffDispatch(effCopyProgram, index); }
        // virtual long EffConnectInput(long index, bool state) { return EffDispatch(effConnectInput, index, state); }
        // virtual long EffConnectOutput(long index, bool state) { return EffDispatch(effConnectOutput, index, state); }
        // virtual long EffGetInputProperties(long index, VstPinProperties *ptr, int size) { return EffDispatch(effGetInputProperties, index, 0, ptr,0, size); }
        // virtual long EffGetOutputProperties(long index, VstPinProperties *ptr, int size) { return EffDispatch(effGetOutputProperties, index, 0, ptr,0, size); }
        virtual long EffGetPlugCategory() { return EffDispatch(effGetPlugCategory); }
        // virtual long EffGetCurrentPosition() { return EffDispatch(effGetCurrentPosition); }
        // virtual long EffGetDestinationBuffer() { return EffDispatch(effGetDestinationBuffer); }
        // virtual long EffOfflineNotify(VstAudioFile* ptr, long numAudioFiles, bool start, int size) { return EffDispatch(effOfflineNotify, start, numAudioFiles, ptr,0, size); }
        // virtual long EffOfflinePrepare(VstOfflineTask *ptr, long count, int size) { return EffDispatch(effOfflinePrepare, 0, count, ptr,0, size); }
        // virtual long EffOfflineRun(VstOfflineTask *ptr, long count, int size) { return EffDispatch(effOfflineRun, 0, count, ptr,0, size); }
        // virtual long EffProcessVarIo(VstVariableIo* varIo, int size) { return EffDispatch(effProcessVarIo, 0, 0, varIo,0, size); }
        // virtual long EffSetSpeakerArrangement(VstSpeakerArrangement* pluginInput, VstSpeakerArrangement* pluginOutput, int size) { return EffDispatch(effSetSpeakerArrangement, 0, (VstIntPtr)pluginInput, pluginOutput,0, size); }
        // virtual long EffSetBlockSizeAndSampleRate(long blockSize, float sampleRate) { return EffDispatch(effSetBlockSizeAndSampleRate, 0, blockSize, 0, sampleRate); }
        // virtual long EffSetBypass(bool onOff) { return EffDispatch(effSetBypass, 0, onOff); }
        virtual long EffGetEffectName(char *ptr, int size) { return EffDispatch(effGetEffectName, 0, 0, ptr,0, size); }
        // virtual long EffGetErrorText(char *ptr, int size) { return EffDispatch(effGetErrorText, 0, 0, ptr,0,size); }
        virtual long EffGetVendorString(char *ptr, int size) { return EffDispatch(effGetVendorString, 0, 0, ptr,0,size); }
        virtual long EffGetProductString(char *ptr, int size) { return EffDispatch(effGetProductString, 0, 0, ptr,0,size); }
        virtual long EffGetVendorVersion() { return EffDispatch(effGetVendorVersion); }
        // virtual long EffVendorSpecific(long index, long value, void *ptr, float opt, int size) { return EffDispatch(effVendorSpecific, index, value, ptr, opt,size); }
        virtual long EffCanDo(const char *ptr, int size) { return EffDispatch(effCanDo, 0, 0, (void *)ptr,0,size); }
        // virtual long EffGetTailSize() { return EffDispatch(effGetTailSize); }
        virtual long EffIdle() { if (bNeedIdle) return EffDispatch(effIdle); else return 0; }
        // virtual long EffGetIcon() { return EffDispatch(effGetIcon); }
        // virtual long EffSetViewPosition(long x, long y) { return EffDispatch(effSetViewPosition, x, y); }
        virtual long EffGetParameterProperties(long index, VstParameterProperties* ptr, int size) { return EffDispatch(effGetParameterProperties, index, 0, ptr,0,size); }
        // virtual long EffKeysRequired() { return EffDispatch(effKeysRequired); }
        virtual long EffGetVstVersion() { return EffDispatch(effGetVstVersion); }
                                            /* VST 2.1 extensions                */
        // virtual long EffKeyDown(VstKeyCode &keyCode, int size) { return EffDispatch(effEditKeyDown, keyCode.character, keyCode.virt, 0, keyCode.modifier,size); }
        // virtual long EffKeyUp(VstKeyCode &keyCode, int size) { return EffDispatch(effEditKeyUp, keyCode.character, keyCode.virt, 0, keyCode.modifier,size); }
        // virtual void EffSetKnobMode(long value) { EffDispatch(effSetEditKnobMode, 0, value); }
        // virtual long EffGetMidiProgramName(long channel, MidiProgramName* midiProgramName, int size) { return EffDispatch(effGetMidiProgramName, channel, 0, midiProgramName,0,size); }
        // virtual long EffGetCurrentMidiProgram (long channel, MidiProgramName* currentProgram, int size) { return EffDispatch(effGetCurrentMidiProgram, channel, 0, currentProgram,0,size); }
        // virtual long EffGetMidiProgramCategory (long channel, MidiProgramCategory* category, int size) { return EffDispatch(effGetMidiProgramCategory, channel, 0, category,0,size); }
        // virtual long EffHasMidiProgramsChanged (long channel) { return EffDispatch(effHasMidiProgramsChanged, channel); }
        // virtual long EffGetMidiKeyName(long channel, MidiKeyName* keyName, int size) { return EffDispatch(effGetMidiKeyName, channel, 0, keyName,0,size); }
        virtual long EffBeginSetProgram() { bInSetProgram = !!EffDispatch(effBeginSetProgram); return bInSetProgram; }
        virtual long EffEndSetProgram() { bInSetProgram = false; return EffDispatch(effEndSetProgram); }
                                            /* VST 2.3 Extensions                */
        // virtual long EffGetSpeakerArrangement(VstSpeakerArrangement** pluginInput, VstSpeakerArrangement** pluginOutput, int size) { return EffDispatch(effGetSpeakerArrangement, 0, (VstIntPtr)pluginInput, pluginOutput,0,size); }
        // virtual long EffSetTotalSampleToProcess (long value) { return EffDispatch(effSetTotalSampleToProcess, 0, value); }
        virtual long EffGetNextShellPlugin(char *name, int size) { return EffDispatch(effShellGetNextPlugin, 0, 0, name,0,size); }
        virtual long EffStartProcess() { return EffDispatch(effStartProcess); }
        virtual long EffStopProcess() { return EffDispatch(effStopProcess); }
        // virtual long EffSetPanLaw(long type, float val) { return EffDispatch(effSetPanLaw, 0, type, 0, val); }
        virtual long EffBeginLoadBank(VstPatchChunkInfo* ptr, int size) { return EffDispatch(effBeginLoadBank, 0, 0, ptr,0,size); }
        virtual long EffBeginLoadProgram(VstPatchChunkInfo* ptr, int size) { return EffDispatch(effBeginLoadProgram, 0, 0, ptr,0,size); }
                                            /* VST 2.4 extensions                */
        // virtual long EffSetProcessPrecision(long precision) { return EffDispatch(effSetProcessPrecision, 0, precision, 0); }
        // virtual long EffGetNumMidiInputChannels() { return EffDispatch(effGetNumMidiInputChannels, 0, 0, 0); }
        // virtual long EffGetNumMidiOutputChannels() { return EffDispatch(effGetNumMidiOutputChannels, 0, 0, 0); }

    // overridables
    public:
        virtual void OnSizeEditorWindow(long width, long height) { }
        virtual bool OnUpdateDisplay() { return false; }
        // virtual void * OnOpenWindow(VstWindow* window) { return 0; }
        // virtual bool OnCloseWindow(VstWindow* window) { return false; }
        virtual bool OnIoChanged() { return false; }
        virtual long OnGetNumAutomatableParameters() { return (pEffect) ? pEffect->numParams : 0; }

    private:
        HMODULE pluginLib;
    };
}

// #ifdef _MSC_VER
// #pragma warning( pop )
// #endif

#endif // CEFFECT_H
