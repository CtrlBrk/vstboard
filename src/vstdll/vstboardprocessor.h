/**************************************************************************
#    Copyright 2010-2012 Raphaël François
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

#ifndef VSTBOARDPROCESSOR_H
#define VSTBOARDPROCESSOR_H

#include "public.sdk/source/vst/vstaudioeffect.h"

#include <QObject>
#include <QtGui/QApplication>
#include <QTranslator>
#include "connectables/vstaudiodevicein.h"
#include "connectables/vstaudiodeviceout.h"
#include "connectables/vstmididevice.h"
#include "connectables/vstautomation.h"
#include "mainhost.h"
#include "settings.h"

#define NB_MAIN_BUSES_IN 4
#define NB_MAIN_BUSES_OUT 4
#define NB_AUX_BUSES_IN 4
#define NB_AUX_BUSES_OUT 4
#define NB_MIDI_BUSES_IN 1
#define NB_MIDI_BUSES_OUT 1

using namespace Steinberg;

class MainHostVst;
class VstBoardProcessor : public MainHost, public Vst::AudioEffect
{
    Q_OBJECT
public:
        VstBoardProcessor (QObject *parent = 0);
        virtual ~VstBoardProcessor();

        tresult PLUGIN_API initialize (FUnknown* context);
        tresult PLUGIN_API terminate ();
        tresult PLUGIN_API setBusArrangements (Vst::SpeakerArrangement* inputs, int32 numIns, Vst::SpeakerArrangement* outputs, int32 numOuts);

        tresult PLUGIN_API setActive (TBool state);
        tresult PLUGIN_API process (Vst::ProcessData& data);

        static FUnknown* createInstance (void*) {
            return (Vst::IAudioProcessor*)new VstBoardProcessor ();
        }

        tresult PLUGIN_API notify (Vst::IMessage* message);

        void SendMsg(const MsgObject &msg);

        bool addAudioIn(Connectables::VstAudioDeviceIn *dev);
        bool addAudioOut(Connectables::VstAudioDeviceOut *dev);
        bool removeAudioIn(Connectables::VstAudioDeviceIn *dev);
        bool removeAudioOut(Connectables::VstAudioDeviceOut *dev);

        tresult PLUGIN_API setupProcessing (Vst::ProcessSetup& setup);

        tresult PLUGIN_API setState (IBStream* state);
        tresult PLUGIN_API getState (IBStream* state);

protected:
        QApplication *myApp;

        QList<Connectables::VstAudioDeviceIn*>lstAudioIn;
        QList<Connectables::VstAudioDeviceOut*>lstAudioOut;
        QList<Connectables::VstMidiDevice*>lstMidiIn;
        QList<Connectables::VstMidiDevice*>lstMidiOut;
        QList<Connectables::VstAutomation*>lstVstAutomation;

        QMutex mutexDevices;

//        ParamValue delay;
//        float** buffer;
//        int32 bufferPos;

public slots:
        void Init();
};

#endif // VSTBOARDPROCESSOR_H
