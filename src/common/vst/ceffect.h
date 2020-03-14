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
#pragma warning( disable : 4100 )
#endif

#include "../precomp.h"
#include "audioeffectx.h"

class MainHost;
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
        QString sName;

    #ifdef WIN32

    #elif MAC

    // yet to do
    // no idea how things look here...

    #endif

        virtual bool Load(const QString &name);
        virtual bool Unload();

        virtual bool LoadBank(std::string *name);
        virtual bool SaveBank(std::string *name);

        virtual bool LoadProgram(std::string *name);
        virtual bool SaveProgram(std::string *name);

        static VstInt32 PluginIdFromBankFile(std::string *name);

        virtual long EffDispatch(VstInt32 opCode, VstInt32 index=0, VstInt32 value=0, void *ptr=0, float opt=0.);
        virtual VstIntPtr OnMasterCallback(long opcode, long index, long value, void *ptr, float opt,long currentReturnCode);

        virtual void EffProcess(float **inputs, float **outputs, long sampleframes);
        virtual void EffProcessReplacing(float **inputs, float **outputs, long sampleframes);
        virtual void EffProcessDoubleReplacing(double** inputs, double** outputs, long sampleFrames);
        virtual void EffSetParameter(long index, float parameter);
        virtual float EffGetParameter(long index);

        virtual void EffOpen() { EffDispatch(effOpen); }
        virtual void EffClose() { EffDispatch(effClose); }
        virtual void EffSetProgram(long lValue) { EffBeginSetProgram(); EffDispatch(effSetProgram, 0, lValue); EffEndSetProgram(); }
        virtual long EffGetProgram() { return EffDispatch(effGetProgram); }
        virtual void EffSetProgramName(char *ptr) { EffDispatch(effSetProgramName, 0, 0, ptr); }
        virtual void EffGetProgramName(char *ptr) { EffDispatch(effGetProgramName, 0, 0, ptr); }
        virtual void EffGetParamLabel(long index, char *ptr) { EffDispatch(effGetParamLabel, index, 0, ptr); }
        virtual void EffGetParamDisplay(long index, char *ptr) { EffDispatch(effGetParamDisplay, index, 0, ptr); }
        QString EffGetParamName(long index);
        virtual void EffSetSampleRate(float fSampleRate) { EffDispatch(effSetSampleRate, 0, 0, 0, fSampleRate); }
        virtual void EffSetBlockSize(long value) { EffDispatch(effSetBlockSize, 0, value); }
        virtual void EffMainsChanged(bool bOn) { EffDispatch(effMainsChanged, 0, bOn); }
        virtual void EffSuspend() { EffDispatch(effMainsChanged, 0, false); }
        virtual void EffResume() { EffDispatch(effMainsChanged, 0, true); }
        virtual float EffGetVu() { return (float)EffDispatch(effGetVu) / (float)32767.; }
        virtual long EffEditGetRect(ERect **ptr) { return EffDispatch(effEditGetRect, 0, 0, ptr); }
        virtual long EffEditOpen(void *ptr) { long l = EffDispatch(effEditOpen, 0, 0, ptr); /* if (l > 0) */ bEditOpen = true; return l; }
        virtual void EffEditClose() { if(!bEditOpen) return; EffDispatch(effEditClose); bEditOpen = false; }
        virtual void EffEditIdle() { if (!bEditOpen || bInEditIdle) return; bInEditIdle = true; EffDispatch(effEditIdle); bInEditIdle = false; }
    #if MAC
        virtual void EffEditDraw(void *ptr) { EffDispatch(nEffect, effEditDraw, 0, 0, ptr); }
        virtual long EffEditMouse(long index, long value) { return EffDispatch(nEffect, effEditMouse, index, value); }
        virtual long EffEditKey(long value) { return EffDispatch(effEditKey, 0, value); }
        virtual void EffEditTop() { EffDispatch(effEditTop); }
        virtual void EffEditSleep() { EffDispatch(effEditSleep); }
    #endif
        virtual long EffIdentify() { return EffDispatch(effIdentify); }

        //as const
        virtual long EffGetChunk(void **ptr, bool isPreset = false) const
        { if (!pEffect)
            return 0;

            long disp=0L;
            disp = pEffect->dispatcher(pEffect, effGetChunk, isPreset, 0, ptr, .0);
            return disp;
          //  return EffDispatch(effGetChunk, isPreset, 0, ptr);
        }
        virtual long EffSetChunk(void *data, long byteSize, bool isPreset = false) { EffBeginSetProgram(); long lResult = EffDispatch(effSetChunk, isPreset, byteSize, data); EffEndSetProgram(); return lResult;}
                                            /* VST 2.0                           */
        virtual long EffProcessEvents(VstEvents* ptr) { return EffDispatch(effProcessEvents, 0, 0, ptr); }
        virtual long EffCanBeAutomated(long index) { return EffDispatch(effCanBeAutomated, index); }
        virtual long EffString2Parameter(long index, char *ptr) { return EffDispatch(effString2Parameter, index, 0, ptr); }
        virtual long EffGetNumProgramCategories() { return EffDispatch(effGetNumProgramCategories); }
        virtual long EffGetProgramNameIndexed(long category, long index, char* text) { return EffDispatch(effGetProgramNameIndexed, index, category, text); }
        virtual long EffCopyProgram(long index) { return EffDispatch(effCopyProgram, index); }
        virtual long EffConnectInput(long index, bool state) { return EffDispatch(effConnectInput, index, state); }
        virtual long EffConnectOutput(long index, bool state) { return EffDispatch(effConnectOutput, index, state); }
        virtual long EffGetInputProperties(long index, VstPinProperties *ptr) { return EffDispatch(effGetInputProperties, index, 0, ptr); }
        virtual long EffGetOutputProperties(long index, VstPinProperties *ptr) { return EffDispatch(effGetOutputProperties, index, 0, ptr); }
        virtual long EffGetPlugCategory() { return EffDispatch(effGetPlugCategory); }
        virtual long EffGetCurrentPosition() { return EffDispatch(effGetCurrentPosition); }
        virtual long EffGetDestinationBuffer() { return EffDispatch(effGetDestinationBuffer); }
        virtual long EffOfflineNotify(VstAudioFile* ptr, long numAudioFiles, bool start) { return EffDispatch(effOfflineNotify, start, numAudioFiles, ptr); }
        virtual long EffOfflinePrepare(VstOfflineTask *ptr, long count) { return EffDispatch(effOfflinePrepare, 0, count, ptr); }
        virtual long EffOfflineRun(VstOfflineTask *ptr, long count) { return EffDispatch(effOfflineRun, 0, count, ptr); }
        virtual long EffProcessVarIo(VstVariableIo* varIo) { return EffDispatch(effProcessVarIo, 0, 0, varIo); }
        virtual long EffSetSpeakerArrangement(VstSpeakerArrangement* pluginInput, VstSpeakerArrangement* pluginOutput) { return EffDispatch(effSetSpeakerArrangement, 0, (long)pluginInput, pluginOutput); }
        virtual long EffSetBlockSizeAndSampleRate(long blockSize, float sampleRate) { return EffDispatch(effSetBlockSizeAndSampleRate, 0, blockSize, 0, sampleRate); }
        virtual long EffSetBypass(bool onOff) { return EffDispatch(effSetBypass, 0, onOff); }
        virtual long EffGetEffectName(char *ptr) { return EffDispatch(effGetEffectName, 0, 0, ptr); }
        virtual long EffGetErrorText(char *ptr) { return EffDispatch(effGetErrorText, 0, 0, ptr); }
        virtual long EffGetVendorString(char *ptr) { return EffDispatch(effGetVendorString, 0, 0, ptr); }
        virtual long EffGetProductString(char *ptr) { return EffDispatch(effGetProductString, 0, 0, ptr); }
        virtual long EffGetVendorVersion() { return EffDispatch(effGetVendorVersion); }
        virtual long EffVendorSpecific(long index, long value, void *ptr, float opt) { return EffDispatch(effVendorSpecific, index, value, ptr, opt); }
        virtual long EffCanDo(const char *ptr) { return EffDispatch(effCanDo, 0, 0, (void *)ptr); }
        virtual long EffGetTailSize() { return EffDispatch(effGetTailSize); }
        virtual long EffIdle() { if (bNeedIdle) return EffDispatch(effIdle); else return 0; }
        virtual long EffGetIcon() { return EffDispatch(effGetIcon); }
        virtual long EffSetViewPosition(long x, long y) { return EffDispatch(effSetViewPosition, x, y); }
        virtual long EffGetParameterProperties(long index, VstParameterProperties* ptr) { return EffDispatch(effGetParameterProperties, index, 0, ptr); }
        virtual long EffKeysRequired() { return EffDispatch(effKeysRequired); }
        virtual long EffGetVstVersion() { return EffDispatch(effGetVstVersion); }
                                            /* VST 2.1 extensions                */
        virtual long EffKeyDown(VstKeyCode &keyCode) { return EffDispatch(effEditKeyDown, keyCode.character, keyCode.virt, 0, keyCode.modifier); }
        virtual long EffKeyUp(VstKeyCode &keyCode) { return EffDispatch(effEditKeyUp, keyCode.character, keyCode.virt, 0, keyCode.modifier); }
        virtual void EffSetKnobMode(long value) { EffDispatch(effSetEditKnobMode, 0, value); }
        virtual long EffGetMidiProgramName(long channel, MidiProgramName* midiProgramName) { return EffDispatch(effGetMidiProgramName, channel, 0, midiProgramName); }
        virtual long EffGetCurrentMidiProgram (long channel, MidiProgramName* currentProgram) { return EffDispatch(effGetCurrentMidiProgram, channel, 0, currentProgram); }
        virtual long EffGetMidiProgramCategory (long channel, MidiProgramCategory* category) { return EffDispatch(effGetMidiProgramCategory, channel, 0, category); }
        virtual long EffHasMidiProgramsChanged (long channel) { return EffDispatch(effHasMidiProgramsChanged, channel); }
        virtual long EffGetMidiKeyName(long channel, MidiKeyName* keyName) { return EffDispatch(effGetMidiKeyName, channel, 0, keyName); }
        virtual long EffBeginSetProgram() { bInSetProgram = !!EffDispatch(effBeginSetProgram); return bInSetProgram; }
        virtual long EffEndSetProgram() { bInSetProgram = false; return EffDispatch(effEndSetProgram); }
                                            /* VST 2.3 Extensions                */
        virtual long EffGetSpeakerArrangement(VstSpeakerArrangement** pluginInput, VstSpeakerArrangement** pluginOutput) { return EffDispatch(effGetSpeakerArrangement, 0, (long)pluginInput, pluginOutput); }
        virtual long EffSetTotalSampleToProcess (long value) { return EffDispatch(effSetTotalSampleToProcess, 0, value); }
        virtual long EffGetNextShellPlugin(char *name) { return EffDispatch(effShellGetNextPlugin, 0, 0, name); }
        virtual long EffStartProcess() { return EffDispatch(effStartProcess); }
        virtual long EffStopProcess() { return EffDispatch(effStopProcess); }
        virtual long EffSetPanLaw(long type, float val) { return EffDispatch(effSetPanLaw, 0, type, 0, val); }
        virtual long EffBeginLoadBank(VstPatchChunkInfo* ptr) { return EffDispatch(effBeginLoadBank, 0, 0, ptr); }
        virtual long EffBeginLoadProgram(VstPatchChunkInfo* ptr) { return EffDispatch(effBeginLoadProgram, 0, 0, ptr); }
                                            /* VST 2.4 extensions                */
        virtual long EffSetProcessPrecision(long precision) { return EffDispatch(effSetProcessPrecision, 0, precision, 0); }
        virtual long EffGetNumMidiInputChannels() { return EffDispatch(effGetNumMidiInputChannels, 0, 0, 0); }
        virtual long EffGetNumMidiOutputChannels() { return EffDispatch(effGetNumMidiOutputChannels, 0, 0, 0); }

    // overridables
    public:
        virtual void OnSizeEditorWindow(long width, long height) { }
        virtual bool OnUpdateDisplay() { return false; }
        virtual void * OnOpenWindow(VstWindow* window) { return 0; }
        virtual bool OnCloseWindow(VstWindow* window) { return false; }
        virtual bool OnIoChanged() { return false; }
        virtual long OnGetNumAutomatableParameters() { return (pEffect) ? pEffect->numParams : 0; }

    private:
        QLibrary *pluginLib;
    };
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif // CEFFECT_H
