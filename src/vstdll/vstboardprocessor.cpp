/**************************************************************************
#    Copyright 2010-2011 Raphaël François
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

#include "vstboardprocessor.h"
#include "ids.h"

#include "projectfile/projectfile.h"
#include "views/configdialog.h"
#include "settings.h"

#include "connectables/vstaudiodevicein.h"
#include "connectables/vstaudiodeviceout.h"
#include "connectables/objectfactoryvst.h"

#include "msgobject.h"

#include "pluginterfaces/base/ibstream.h"

VstBoardProcessor::VstBoardProcessor (Settings *settings,QObject *parent) :
    MainHost(settings,parent),
    Vst::AudioEffect()
{
        setControllerClass (VstBoardControllerUID);

}

VstBoardProcessor::~VstBoardProcessor()
{
    Close();
}

void VstBoardProcessor::Init()
{
    MainHost::Init();
    objFactory = new Connectables::ObjectFactoryVst(this);
}

tresult PLUGIN_API VstBoardProcessor::initialize (FUnknown* context)
{
    Init();

    tresult result = AudioEffect::initialize (context);
    if (result != kResultTrue)
        return result;

    qRegisterMetaType<ConnectionInfo>("ConnectionInfo");
    qRegisterMetaType<ObjectInfo>("ObjectInfo");
    qRegisterMetaType<MsgObject>("MsgObject");
    qRegisterMetaType<int>("ObjType::Enum");
    qRegisterMetaType<QVariant>("QVariant");
    qRegisterMetaType<AudioBuffer*>("AudioBuffer*");

    qRegisterMetaTypeStreamOperators<ObjectInfo>("ObjectInfo");

    QCoreApplication::setOrganizationName("CtrlBrk");
    QCoreApplication::setApplicationName("VstBoard");

    Open();

    //load default setup file
    QString currentSetupFile = ConfigDialog::defaultSetupFile(settings);
    if(!currentSetupFile.isEmpty()) {
        if(!ProjectFile::LoadFromFile(this,currentSetupFile))
            currentSetupFile = "";
    }

    //load default project file
    QString currentProjectFile = ConfigDialog::defaultProjectFile(settings);
    if(!currentProjectFile.isEmpty()) {
        if(!ProjectFile::LoadFromFile(this,currentProjectFile))
            currentProjectFile = "";
    }

    for(int i=0; i<NB_MAIN_BUSES_IN; i++) {
        addAudioInput(QString("AudioIn%1").arg(i+1).utf16(), Vst::SpeakerArr::kStereo, Vst::kMain, 0);
    }
    for(int i=0; i<NB_MAIN_BUSES_OUT; i++) {
        addAudioOutput(QString("AudioOut%1").arg(i+1).utf16(), Vst::SpeakerArr::kStereo, Vst::kMain, 0);
    }

    for(int i=0; i<NB_AUX_BUSES_IN; i++) {
        addAudioInput(QString("AuxIn%1").arg(i+1).utf16(), Vst::SpeakerArr::kStereo, Vst::kAux, 0);
    }
    for(int i=0; i<NB_AUX_BUSES_OUT; i++) {
        addAudioOutput(QString("AuxOut%1").arg(i+1).utf16(), Vst::SpeakerArr::kStereo, Vst::kAux, 0);
    }

    for(int i=0; i<NB_MIDI_BUSES_IN; i++) {
        addEventInput(QString("MidiIn%1").arg(i+1).utf16(), 16, Vst::kMain, 0);
    }
    for(int i=0; i<NB_MIDI_BUSES_OUT; i++) {
        addEventOutput(QString("MidiOut%1").arg(i+1).utf16(), 16, Vst::kMain, 0);
    }

    return kResultTrue;
}

tresult PLUGIN_API VstBoardProcessor::setState (IBStream* state)
{
    int32 size=0;
    if (state->read (&size, sizeof (int32)) != kResultOk)
        return kResultFalse;

#if BYTEORDER == kBigEndian
    SWAP_32 (size)
#endif

    char *buf = new char[size];
    if (state->read (buf, size) != kResultOk) {
        delete[] buf;
        return kResultFalse;
    }


    QByteArray bArray(buf,size);
    QDataStream stream( &bArray , QIODevice::ReadOnly);
    if(!ProjectFile::FromStream(this,stream)) {
        ClearSetup();
        ClearProject();
        delete[] buf;
        return kResultFalse;
    }

    delete[] buf;
    return kResultOk;
}

tresult PLUGIN_API VstBoardProcessor::getState (IBStream* state)
{
    QByteArray bArray;
    QDataStream stream( &bArray , QIODevice::WriteOnly);
    ProjectFile::ToStream(this,stream);

    int32 size =  bArray.size();
#if BYTEORDER == kBigEndian
    SWAP_32 (size)
#endif
    state->write(&size, sizeof (int32));
    state->write(bArray.data(), bArray.size());

    return kResultOk;
}

tresult PLUGIN_API VstBoardProcessor::setupProcessing (Vst::ProcessSetup& newSetup)
{
    unsigned long bSize = (unsigned long)newSetup.maxSamplesPerBlock;
    if(bufferSize != bSize) {
        SetBufferSize(bSize);
    }

    float sRate = static_cast<float>(newSetup.sampleRate);
    if(sampleRate != sRate)
        SetSampleRate(sRate);

//    if(newSetup.symbolicSampleSize!=Vst::kSample32 && newSetup.symbolicSampleSize!=Vst::kSample64) {
//        return kResultFalse;
//    }

    return kResultOk;
}

tresult PLUGIN_API VstBoardProcessor::terminate ()
{
    tresult result = AudioEffect::terminate ();
    if (result != kResultTrue)
        return result;

    return result;
}

tresult PLUGIN_API VstBoardProcessor::setBusArrangements (Vst::SpeakerArrangement* inputs, int32 numIns, Vst::SpeakerArrangement* outputs, int32 numOuts)
{
//        if (numIns == 1 && numOuts == 1 && inputs[0] == outputs[0])
                return AudioEffect::setBusArrangements (inputs, numIns, outputs, numOuts);
//        return kResultFalse;
}

tresult PLUGIN_API VstBoardProcessor::setActive (TBool state)
{
        Vst::SpeakerArrangement arr;
        if (getBusArrangement (Vst::kOutput, 0, arr) != kResultTrue)
                return kResultFalse;
        int32 numChannels = Vst::SpeakerArr::getChannelCount (arr);
        if (numChannels == 0)
                return kResultFalse;

        if (state)
        {

        }
        else
        {

        }
        return AudioEffect::setActive (state);
}

tresult PLUGIN_API VstBoardProcessor::process (Vst::ProcessData& data)
{
    unsigned long bSize = (unsigned long)data.numSamples;
    if(bufferSize != bSize) {
        SetBufferSize(bSize);
    }

    int cpt=0;
    Vst::AudioBusBuffers *buf = data.inputs;
    foreach(Connectables::VstAudioDeviceIn* dev, lstAudioIn) {
        dev->SetBuffers(buf,data.numSamples);
        ++buf;
        ++cpt;
    }

    Render();

    cpt=0;
    buf = data.outputs;
    foreach(Connectables::VstAudioDeviceOut* dev, lstAudioOut) {
        dev->GetBuffers(buf,data.numSamples);
        ++buf;
        ++cpt;
    }

    return kResultTrue;
}

tresult PLUGIN_API VstBoardProcessor::notify (Vst::IMessage* message) {
    if (!message)
        return kInvalidArgument;

    if (!strcmp (message->getMessageID (), "msg"))
    {
        const void* data;
        uint32 size;
        if (message->getAttributes ()->getBinary ("data", data, size) == kResultOk)
        {
    //        if (!strcmp (message->getMessageID (), "msglist")) {
    //            QByteArray ba((char*)data,size);
    //            ReceiveMsg(message->getMessageID(),ba);
    //            return kResultOk;
    //        }

            QByteArray br((char*)data,size);
            QDataStream str(&br, QIODevice::ReadOnly);
            MsgObject msg;
            str >> msg;
            ReceiveMsg( msg );
            return kResultOk;
        }
    }

    return AudioEffect::notify(message);
}

void VstBoardProcessor::SendMsg(const MsgObject &msg)
{
    Steinberg::OPtr<Steinberg::Vst::IMessage> message = allocateMessage();
    if (message)
    {
        message->setMessageID("msg");
        QByteArray br;
        QDataStream str(&br, QIODevice::WriteOnly);
//        LOG(msg.prop)
        str << msg;
        message->getAttributes ()->setBinary ("data", br.data(), br.size());
        sendMessage(message);
    }
}

bool VstBoardProcessor::addAudioIn(Connectables::VstAudioDeviceIn *dev)
{
    QMutexLocker l(&mutexDevices);

    if(lstAudioIn.contains(dev))
        return false;

    dev->setObjectName( QString("Audio in %1").arg( lstAudioIn.count()+1 ) );
    activateBus(Vst::kAudio, Vst::kInput, lstAudioIn.count(), true);
    lstAudioIn << dev;
    return true;
}

bool VstBoardProcessor::addAudioOut(Connectables::VstAudioDeviceOut *dev)
{
    QMutexLocker l(&mutexDevices);

    if(lstAudioOut.contains(dev))
        return false;

    dev->setObjectName( QString("Audio out %1").arg( lstAudioOut.count()+1 ) );
    activateBus(Vst::kAudio, Vst::kOutput, lstAudioOut.count(), true);
    lstAudioOut << dev;
    return true;
}

bool VstBoardProcessor::removeAudioIn(Connectables::VstAudioDeviceIn *dev)
{
    QMutexLocker l(&mutexDevices);
    int id = lstAudioIn.indexOf(dev);
    lstAudioIn.removeAt(id);
    return true;
}

bool VstBoardProcessor::removeAudioOut(Connectables::VstAudioDeviceOut *dev)
{
    QMutexLocker l(&mutexDevices);
    int id = lstAudioOut.indexOf(dev);
    lstAudioOut.removeAt(id);
    return true;
}
