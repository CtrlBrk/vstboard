/**************************************************************************
#    Copyright 2010-2020 Raphaël François
#    Contact : ctrlbrk76@gmail.com
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
**************************************************************************/
/*
#ifndef VST_H
#define VST_H

#define uniqueIDEffect CCONST('V','b','P','l')
#define uniqueIDInstrument CCONST('V','b','I','s')
#define VST_EVENT_BUFFER_SIZE 1000
#define DEFAULT_INPUTS 1
#define DEFAULT_OUTPUTS 1

#include <QObject>
#include <QtGui/QApplication>
#include <QTranslator>
#include "mainwindowvst.h"
#include "audioeffectx.h"
#include "gui.h"
#include "connectables/vstaudiodevicein.h"
#include "connectables/vstaudiodeviceout.h"
#include "connectables/vstmididevice.h"
#include "connectables/vstautomation.h"

AudioEffect* createEffectInstance (audioMasterCallback audioMaster, bool asInstrument);

//-------------------------------------------------------------------------------------------------------
class Vst : public QObject, public AudioEffectX
{
    Q_OBJECT
public:

        Vst (audioMasterCallback audioMaster, bool asInstrument=false);
        ~Vst ();

        VstInt32 canDo(char* text);
        VstInt32 processEvents(VstEvents* events);

        void processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames);
        void processDoubleReplacing (double** inputs, double** outputs, VstInt32 sampleFrames);

        void setProgramName (char* name);
        void getProgramName (char* name);
        void setProgram(VstInt32 program);
        VstInt32 getProgram();

        void setParameter (VstInt32 index, float value);
        float getParameter (VstInt32 index);
        void getParameterLabel (VstInt32 index, char* label);
        void getParameterDisplay (VstInt32 index, char* text);
        void getParameterName (VstInt32 index, char* text);

        VstInt32 setChunk (void* data, VstInt32 byteSize, bool isPreset = false);
        VstInt32 getChunk (void** data, bool isPreset);

        bool getEffectName (char* name);
        bool getVendorString (char* text);
        bool getProductString (char* text);
        VstInt32 getVendorVersion ();


        bool addAudioIn(Connectables::VstAudioDeviceIn *dev);
        bool addAudioOut(Connectables::VstAudioDeviceOut *dev);
        bool removeAudioIn(Connectables::VstAudioDeviceIn *dev);
        bool removeAudioOut(Connectables::VstAudioDeviceOut *dev);

        void addMidiIn(Connectables::VstMidiDevice *dev);
        void addMidiOut(Connectables::VstMidiDevice *dev);
        void removeMidiIn(Connectables::VstMidiDevice *dev);
        void removeMidiOut(Connectables::VstMidiDevice *dev);

        void addVstAutomation(Connectables::VstAutomation *dev);
        void removeVstAutomation(Connectables::VstAutomation *dev);

        VstInt32 getNumMidiInputChannels();
        VstInt32 getNumMidiOutputChannels();

        void open();
        void close();
        void setSampleRate(float sampleRate);
        void setBlockSize (VstInt32 blockSize);
        void suspend();
        void resume();
        VstPlugCategory getPlugCategory() {return kPlugCategEffect;}

        void updateParameter(int index, float value);

protected:
        QApplication *myApp;
        MainHostVst *myHost;
        MainWindowVst *myWindow;

        char programName[kVstMaxProgNameLen + 1];

        QList<Connectables::VstAudioDeviceIn*>lstAudioIn;
        QList<Connectables::VstAudioDeviceOut*>lstAudioOut;
        QList<Connectables::VstMidiDevice*>lstMidiIn;
        QList<Connectables::VstMidiDevice*>lstMidiOut;
        QList<Connectables::VstAutomation*>lstVstAutomation;
        QMap<int,float>listParameters;

        VstInt32 bufferSize;

        QMutex mutexDevices;
        QMutex mutexParam;

        Gui *qEditor;

        VstEvents *listEvnts;

        bool hostSendVstEvents;
        bool hostSendVstMidiEvent;
        bool hostReportConnectionChanges;
        bool hostAcceptIOChanges;
        bool hostSendVstTimeInfo;
        bool hostReceiveVstEvents;
        bool hostReceiveVstMidiEvents;
        bool hostReceiveVstTimeInfo;
        bool opened;
        VstInt32 currentHostProg;

        char *chunkData;
        void deleteChunkData();

        QTranslator qtTranslator;
        QTranslator commonTranslator;
        QTranslator myappTranslator;
    signals:
        void HostChangedProg(int prog);
    public slots:
        void OnMainHostTempoChange();
        void DelayChanged(long samples);
};


#endif // VST_H
*/
