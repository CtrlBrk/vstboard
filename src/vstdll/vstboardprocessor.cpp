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

#include "vstboardprocessor.h"
#include "ids.h"

#include "projectfile/projectfile.h"
#include "views/configdialog.h"
#include "settings.h"

#include "connectables/vstaudiodevicein.h"
#include "connectables/vstaudiodeviceout.h"
#include "connectables/objectfactoryvst.h"

#include "projectfile/jsonwriter.h"
#include "projectfile/jsonreader.h"

#include "msgobject.h"

#include "pluginterfaces/base/ibstream.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/vst/ivstevents.h"
#include "public.sdk/source/common/memorystream.h"

#include "../common/views/audiograph.h"

VstBoardProcessor::VstBoardProcessor () :
    MainHost(0,0),
    Vst::AudioEffect(),
#ifdef VST24SDK
    listEvnts(0),
#endif
    currentProg(0),
    currentGroup(0),
    currentBypass(false)
{
	objFactory = new Connectables::ObjectFactoryVst(this);

#if defined(_M_X64) || defined(__amd64__)
    settings = new Settings("x64/plugin/",this);
#else
    settings = new Settings("x86/plugin/",this);
#endif

    setControllerClass (VstBoardControllerUID);
    Init();
}

VstBoardProcessor::~VstBoardProcessor()
{
    Close();
#ifdef VST24SDK
    if(listEvnts) {
        free(listEvnts);
        listEvnts = 0;
    }
#endif
}


void VstBoardProcessor::Close()
{
    foreach(Connectables::VstAudioDeviceIn* dev, lstAudioIn) {
        //dev->Close();
        removeAudioIn(dev);
    }
    foreach(Connectables::VstAudioDeviceOut* dev, lstAudioOut) {
        // dev->Close();
        removeAudioOut(dev);
    }
    foreach(Connectables::VstMidiDevice* dev, lstMidiIn) {
        // dev->Close();
        removeMidiIn(dev);
    }
    foreach(Connectables::VstMidiDevice* dev, lstMidiOut) {
        // dev->Close();
        removeMidiOut(dev);
    }
    foreach(Connectables::VstAutomation* dev, lstVstAutomation) {
        // dev->Close();
        removeVstAutomation(dev);
    }
}

void VstBoardProcessor::Init()
{
    MainHost::Init();
    QObject::connect(this, SIGNAL(ChangeProg(quint16)),
            programManager, SLOT(UserChangeProg(quint16)));
    QObject::connect(this, SIGNAL(ChangeGroup(quint16)),
            programManager, SLOT(UserChangeGroup(quint16)));
    QObject::connect(this, SIGNAL(SetBypass(bool)),
            programManager, SLOT(UserSetBypass(bool)));
}

// uint32 PLUGIN_API VstBoardProcessor::getProcessContextRequirements ()
// {
//     processContextRequirements.needSystemTime ();
//     processContextRequirements.needContinousTimeSamples ();
//     processContextRequirements.needProjectTimeMusic ();
//     processContextRequirements.needBarPositionMusic ();
//     processContextRequirements.needCycleMusic ();
//     processContextRequirements.needSamplesToNextClock ();
//     processContextRequirements.needTempo ();
//     processContextRequirements.needTimeSignature ();
//     processContextRequirements.needChord ();
//     processContextRequirements.needFrameRate ();
//     processContextRequirements.needTransportState ();

//     return AudioEffect::getProcessContextRequirements ();
// }

tresult PLUGIN_API VstBoardProcessor::setIoMode (Vst::IoMode mode)
{
    return kNotImplemented;
}

tresult PLUGIN_API VstBoardProcessor::getControllerClassId (TUID classID)
{
    if (controllerClass.isValid ())
    {
        controllerClass.toTUID (classID);
        return kResultTrue;
    }
    return kResultFalse;
}

tresult PLUGIN_API VstBoardProcessor::initialize (FUnknown* context)
{
    tresult result = AudioEffect::initialize (context);
    if (result != kResultTrue)
        return result;

    // if (auto hostApp = Steinberg::U::cast<IHostApplication> (hostContext))
    // {
    //     Vst::String128 name;
    //     if (hostApp->getName (name) == kResultTrue)
    //     {

    //     }
    // }

//    qRegisterMetaType<ConnectionInfo>("ConnectionInfo");
//    qRegisterMetaType<ObjectInfo>("ObjectInfo");
//    qRegisterMetaType<ObjectContainerAttribs>("ObjectContainerAttribs");
//    qRegisterMetaType<MsgObject>("MsgObject");
//    qRegisterMetaType<int>("ObjType::Enum");
//    qRegisterMetaType<QVariant>("QVariant");
//    qRegisterMetaType<AudioBuffer*>("AudioBuffer*");
//    qRegisterMetaType<QVector<int> >("QVector<int>");
//    qRegisterMetaTypeStreamOperators<ObjectInfo>("ObjectInfo");
//    qRegisterMetaTypeStreamOperators<ObjectContainerAttribs>("ObjectContainerAttribs");

    QCoreApplication::setOrganizationName("CtrlBrk");
    QCoreApplication::setApplicationName("VstBoard");


    Open();

    //load default setup file
    QString currentSetupFile = ConfigDialog::defaultSetupFile(settings);
    if(!currentSetupFile.isEmpty()) {
        if(!ProjectFile::LoadFromFile(this,currentSetupFile)) {
            currentSetupFile = "";
        }
    }

    //load default project file
    QString currentProjectFile = ConfigDialog::defaultProjectFile(settings);
    if(!currentProjectFile.isEmpty()) {
        if(!ProjectFile::LoadFromFile(this,currentProjectFile)) {
            currentProjectFile = "";
        }
    }

    for(int i=0; i<NB_MAIN_BUSES_IN; i++) {
        addAudioInput((char16*)QString("AudioIn%1").arg(i+1).utf16(), Vst::SpeakerArr::kStereo, Vst::kMain);
    }
    for(int i=0; i<NB_MAIN_BUSES_OUT; i++) {
        addAudioOutput((char16*)QString("AudioOut%1").arg(i+1).utf16(), Vst::SpeakerArr::kStereo, Vst::kMain);
    }

    // for(int i=0; i<NB_AUX_BUSES_IN; i++) {
    //     addAudioInput((char16*)QString("AuxIn%1").arg(i+1).utf16(), Vst::SpeakerArr::kStereo, Vst::kAux);
    // }
    // for(int i=0; i<NB_AUX_BUSES_OUT; i++) {
    //     addAudioOutput((char16*)QString("AuxOut%1").arg(i+1).utf16(), Vst::SpeakerArr::kStereo, Vst::kAux);
    // }

    for(int i=0; i<NB_MIDI_BUSES_IN; i++) {
        addEventInput((char16*)QString("MidiIn%1").arg(i+1).utf16(), 4, Vst::kMain);
    }
    for(int i=0; i<NB_MIDI_BUSES_OUT; i++) {
        addEventOutput((char16*)QString("MidiOut%1").arg(i+1).utf16(), 4, Vst::kMain);
    }

    return kResultTrue;
}

tresult PLUGIN_API VstBoardProcessor::setState (IBStream* state)
{
	int size = 0;
	state->read(&size, sizeof(int));
	//don't know how to write directly to a bytearray
	char* buf = new char[size];
	state->read(buf, size);
	
	QByteArray bArray(QByteArray::fromRawData(buf, size));
	
    // QJsonDocument loadDoc(QJsonDocument::fromBinaryData(qUncompress(bArray)));
    QJsonDocument loadDoc(QJsonDocument::fromJson(bArray));
	QJsonObject json = loadDoc.object();
	if (json.contains("proc")) {
		JsonReader::readProjectProcess(json["proc"].toObject(), this);
	}
	
    return kResultOk; 
}

tresult PLUGIN_API VstBoardProcessor::getState (IBStream* state)
{
	QJsonObject jsonObj;
	jsonObj["proc"] = JsonWriter::writeProjectProcess(this, true, true);
	QJsonDocument saveDoc(jsonObj);
    // QByteArray bArray = qCompress(saveDoc.toBinaryData());
    QByteArray bArray = saveDoc.toJson(QJsonDocument::Indented);
	int size = bArray.size();
	state->write(&size, sizeof(int));
    state->write(bArray.data(), bArray.size());

    return kResultOk; 
}

tresult PLUGIN_API VstBoardProcessor::setupProcessing (Vst::ProcessSetup& newSetup)
{
    qint32 bSize = newSetup.maxSamplesPerBlock;
    if(bufferSize != bSize) {
        SetBufferSize(bSize);
    }

    float sRate = static_cast<float>(newSetup.sampleRate);
    if(sampleRate != sRate) {
        SetSampleRate(sRate);
    }

    //newSetup.processMode;

    if (canProcessSampleSize (newSetup.symbolicSampleSize) != kResultTrue) {
        return kResultFalse;
    }

	doublePrecision = newSetup.symbolicSampleSize == Vst::kSample64 ? true : false;
	settings->SetSetting("currentDoublePrecision", doublePrecision);
    return kResultOk;
}

tresult PLUGIN_API VstBoardProcessor::terminate ()
{
    tresult result = AudioEffect::terminate ();
    if (result != kResultTrue)
        return result;

    return result;
}

tresult PLUGIN_API VstBoardProcessor::getBusArrangement (Vst::BusDirection dir, int32 busIndex, Vst::SpeakerArrangement& arr)
{
    Vst::BusList* busList = getBusList (Vst::MediaTypes::kAudio, dir);
    if (!busList || busIndex < 0 || static_cast<int32> (busList->size ()) <= busIndex)
        return kInvalidArgument;
    if (auto* audioBus = FCast<Vst::AudioBus> (busList->at (busIndex)))
    {
        arr = audioBus->getArrangement ();
        return kResultTrue;
    }
    return kResultFalse;
}

tresult PLUGIN_API VstBoardProcessor::setBusArrangements (Vst::SpeakerArrangement* inputs, int32 numIns, Vst::SpeakerArrangement* outputs, int32 numOuts)
{
    // return AudioEffect::setBusArrangements (inputs, numIns, outputs, numOuts);
    if (numIns)
    {
        if (Vst::SpeakerArr::getChannelCount (inputs[0]) != 2)
            return kResultFalse;
    }
    if (numOuts)
    {
        if (Vst::SpeakerArr::getChannelCount (outputs[0]) != 2)
            return kResultFalse;
    }
    return kResultTrue;
}

tresult PLUGIN_API VstBoardProcessor::canProcessSampleSize (int32 symbolicSampleSize)
{
    if(symbolicSampleSize==Vst::kSample32) return kResultOk;
    if(symbolicSampleSize==Vst::kSample64) return kResultOk;

    return kResultFalse;
}

tresult PLUGIN_API VstBoardProcessor::setActive (TBool state)
{
        if (state) {
            Vst::SpeakerArrangement arr;
            if (getBusArrangement (Vst::kOutput, 0, arr) != kResultTrue)
                return kResultFalse;
            int32 numChannels = Vst::SpeakerArr::getChannelCount (arr);
            if (numChannels == 0)
                return kResultFalse;

            EnableSolverUpdate(true);
        } else {
            EnableSolverUpdate(false);
        }

        return AudioEffect::setActive (state);
}

tresult PLUGIN_API VstBoardProcessor::process (Vst::ProcessData& data)
{
    //timer
    if(data.processContext) {
        vst3Host->SetTimeInfo(data.processContext);
//        vst3Host->GetTimeInfo(&vstHost->vstTimeInfo);
    }

    //param changes
    Vst::IParameterChanges* paramChanges = data.inputParameterChanges;
    if (paramChanges)
    {
        int32 numParamsChanged = paramChanges->getParameterCount ();
        for (int32 i = 0; i < numParamsChanged; i++)
        {
            Vst::IParamValueQueue* paramQueue = paramChanges->getParameterData (i);
            if (paramQueue)
            {
                int32 offsetSamples;
                double value;
                int32 numPoints = paramQueue->getPointCount ();

                if (paramQueue->getPoint (numPoints - 1,  offsetSamples, value) == kResultTrue) {
//                    LOG(paramQueue->getParameterId () << value)
                    bool bypass=false;

                    switch (paramQueue->getParameterId ())
                    {
                        case paramByPass:
                        // LOG("bypass"<<value)
                            bypass = value > 0;
                            emit SetBypass( bypass );
                            break;
                        case paramProgChange:
//                            LOG("prgChn"<<value)
                            emit ChangeProg( value*128 );
                            break;
                        case paramGroupChange:
                            emit ChangeGroup( value*128 );
                            break;
                        default:
                            foreach(Connectables::VstAutomation *dev, lstVstAutomation) {
                                dev->ValueFromHost(paramQueue->getParameterId(),value);
                            }
                            break;
                    }
                }
            }
        }
    }

    //notes on/off
    Vst::IEventList* eventList = data.inputEvents;
    if (eventList)
    {
        int32 numEvent = eventList->getEventCount ();
        for (int32 i = 0; i < numEvent; i++)
        {
            Vst::Event event;
            if (eventList->getEvent (i, event) == kResultOk)
            {
				if (event.busIndex < lstMidiIn.count()) {
					lstMidiIn[event.busIndex]->EventFromInput(event);
				}
            }
        }
    }

    int32 bSize = data.numSamples;
    if(bSize>0 && bufferSize != bSize) {
        SetBufferSize(bSize);
    }

    //get audio from input
    if (data.numSamples > 0) {
        Vst::AudioBusBuffers *buf = data.inputs;
        foreach(Connectables::VstAudioDeviceIn* dev, lstAudioIn) {
            if(doublePrecision) {
                dev->SetBuffersD(buf->channelBuffers64,data.numSamples);
            } else {
                dev->SetBuffers(buf->channelBuffers32,data.numSamples);
            }
            ++buf;
        }
    }

    Render();

    //put audio to outputs
    if (data.numSamples > 0) {
        Vst::AudioBusBuffers* buf = data.outputs;
        foreach(Connectables::VstAudioDeviceOut* dev, lstAudioOut) {
            if(doublePrecision) {
                dev->GetBuffersD(buf->channelBuffers64,data.numSamples);
            } else {
                dev->GetBuffers(buf->channelBuffers32,data.numSamples);
            }
            ++buf;
        }
    }

    //output params
    Vst::IParameterChanges* paramOutChanges = data.outputParameterChanges;
    if(paramOutChanges) {
        quint16 prg = programManager->GetCurrentMidiProg();
        if(prg!=currentProg) {
            int32 index = 0;
            Vst::IParamValueQueue* paramQueue = paramOutChanges->addParameterData(paramProgChange, index);
            if(paramQueue) {
                int32 index2 = 0;
                paramQueue->addPoint(0, (Vst::ParamValue)prg/128, index2);
            }
            currentProg=prg;
        }
        quint16 grp = programManager->GetCurrentMidiGroup();
        if(grp!=currentGroup) {
            int32 index = 0;
            Vst::IParamValueQueue* paramQueue = paramOutChanges->addParameterData(paramGroupChange, index);
            if(paramQueue) {
                int32 index2 = 0;
                paramQueue->addPoint(0, (Vst::ParamValue)grp/128, index2);
            }
            currentGroup=grp;
        }
        bool bypass = (programManager->GetCurrentBypassState()>.5);
        if(bypass!=currentBypass) {
            int32 index = 0;
            Vst::IParamValueQueue* paramQueue = paramOutChanges->addParameterData(paramByPass, index);
            if(paramQueue) {
                int32 index2 = 0;
                Vst::ParamValue setBypass = bypass ? 1.0 : 0.0;
                setBypass = (Vst::ParamValue)bypass;
                paramQueue->addPoint(0, setBypass, index2);
            }
            currentBypass=bypass;
        }
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

bool VstBoardProcessor::addMidiIn(Connectables::VstMidiDevice *dev)
{
    lstMidiIn << dev;
    return true;
}

bool VstBoardProcessor::addMidiOut(Connectables::VstMidiDevice *dev)
{
    lstMidiOut << dev;
    return true;
}

bool VstBoardProcessor::removeMidiIn(Connectables::VstMidiDevice *dev)
{
    lstMidiIn.removeAll(dev);
    return true;
}

bool VstBoardProcessor::removeMidiOut(Connectables::VstMidiDevice *dev)
{
    lstMidiOut.removeAll(dev);
    return true;
}

bool VstBoardProcessor::addAudioIn(Connectables::VstAudioDeviceIn *dev)
{
    QMutexLocker l(&mutexDevices);

    if(lstAudioIn.contains(dev))
        return false;

    if(lstAudioIn.count() == NB_MAIN_BUSES_IN)
        return false;

    dev->setObjectName( QString("Audio in %1").arg( lstAudioIn.count()+1 ) );
    lstAudioIn << dev;
    return true;
}

bool VstBoardProcessor::addAudioOut(Connectables::VstAudioDeviceOut *dev)
{
    QMutexLocker l(&mutexDevices);

    if(lstAudioOut.contains(dev))
        return false;

    if(lstAudioOut.count() == NB_MAIN_BUSES_OUT)
        return false;

    dev->setObjectName( QString("Audio out %1").arg( lstAudioOut.count()+1 ) );
    lstAudioOut << dev;
    return true;
}

bool VstBoardProcessor::removeAudioIn(Connectables::VstAudioDeviceIn *dev)
{
    QMutexLocker l(&mutexDevices);
    lstAudioIn.removeAll(dev);
    return true;
}

bool VstBoardProcessor::removeAudioOut(Connectables::VstAudioDeviceOut *dev)
{
    QMutexLocker l(&mutexDevices);
    lstAudioOut.removeAll(dev);
    return true;
}


void VstBoardProcessor::addVstAutomation(Connectables::VstAutomation *dev)
{
    lstVstAutomation << dev;
}

void VstBoardProcessor::removeVstAutomation(Connectables::VstAutomation *dev)
{
    lstVstAutomation.removeAll(dev);
}

#ifdef VST24SDK
VstInt32 VstBoardProcessor::processEvents(VstEvents* events)
{
    if(!events)
        return 0;

    VstEvent *evnt=0;

    for(int i=0; i<events->numEvents; i++) {
        evnt=events->events[i];
        if( evnt->type==kVstMidiType) {
            VstMidiEvent *midiEvnt = (VstMidiEvent*)evnt;

            foreach(Connectables::VstMidiDevice *dev, lstMidiIn) {
                long msg;
                memcpy(&msg, midiEvnt->midiData, sizeof(midiEvnt->midiData));
                dev->midiQueue << msg;
            }
        } else {
            LOG("other vst event");
        }
    }

    return 1;
}

bool VstBoardProcessor::processOutputEvents()
{
    //free last buffer
    if(listEvnts) {
        free(listEvnts);
        listEvnts = 0;
    }

    int cpt=0;
    foreach(Connectables::VstMidiDevice *dev, lstMidiOut) {
		foreach(long msg, dev->midiQueue) {

			//allocate a new buffer
			if (!listEvnts)
				listEvnts = (VstEvents*)malloc(sizeof(VstEvents) + sizeof(VstEvents*)*(VST_EVENT_BUFFER_SIZE - 2));
			if (!listEvnts) {
				LOG("malloc error!");
				return false;
			}
            VstMidiEvent *evnt = new VstMidiEvent;
            memset(evnt, 0, sizeof(VstMidiEvent));
            evnt->type = kVstMidiType;
            evnt->flags = kVstMidiEventIsRealtime;
            evnt->byteSize = sizeof(VstMidiEvent);
            //memcpy(evnt->midiData, &msg.message, sizeof(evnt->midiData));
            memcpy(evnt->midiData, &msg, sizeof(evnt->midiData));
            listEvnts->events[cpt]=(VstEvent*)evnt;
            cpt++;
        }
        dev->midiQueue.clear();
    }

    if(cpt>0) {
        listEvnts->numEvents=cpt;
//        sendVstEventsToHost(listEvnts);
        return true;
    }
    return false;
}
#endif
