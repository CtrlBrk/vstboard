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
#include "vst.h"
#include "connectables/connectioninfo.h"
#include "connectables/objectinfo.h"
#include "mainhostvst.h"
#include "projectfile/projectfile.h"
#include "views/configdialog.h"

AudioEffect* createEffectInstance (audioMasterCallback audioMaster, bool asInstrument)
{
    return new Vst (audioMaster, asInstrument);
}

Vst::Vst (audioMasterCallback audioMaster, bool asInstrument) :
    AudioEffectX (audioMaster, 128, 128),
    myApp(0),
    myHost(0),
    myWindow(0),
    bufferSize(0),
    listEvnts(0),
    hostSendVstEvents(false),
    hostSendVstMidiEvent(false),
    hostReportConnectionChanges(false),
    hostAcceptIOChanges(false),
    hostSendVstTimeInfo(false),
    hostReceiveVstEvents(false),
    hostReceiveVstMidiEvents(false),
    hostReceiveVstTimeInfo(false),
    opened(false),
    currentHostProg(0),
    chunkData(0)
{
    setNumInputs (DEFAULT_INPUTS*2);
    setNumOutputs (DEFAULT_OUTPUTS*2);

    if(asInstrument)
        setUniqueID (uniqueIDInstrument);
    else
        setUniqueID (uniqueIDEffect);

    isSynth(asInstrument);
    canProcessReplacing(true);
    programsAreChunks(true);
    vst_strncpy (programName, "Default", kVstMaxProgNameLen);	// default program name

//    qRegisterMetaType<ConnectionInfo>("ConnectionInfo");
//    qRegisterMetaType<ObjectInfo>("ObjectInfo");
//    qRegisterMetaType<int>("ObjType::Enum");
//    qRegisterMetaType<QVariant>("QVariant");
//    qRegisterMetaType<AudioBuffer*>("AudioBuffer*");
//    qRegisterMetaType<QVector<int> >("QVector<int>");

//    qRegisterMetaTypeStreamOperators<ObjectInfo>("ObjectInfo");

    QCoreApplication::setOrganizationName("CtrlBrk");
    QCoreApplication::setApplicationName("VstBoard");

#ifdef QT_NO_DEBUG
    if(qtTranslator.load("qt_" + QLocale::system().name(), ":/translations/"))
        qApp->installTranslator(&qtTranslator);
    if(commonTranslator.load("common_" + QLocale::system().name(), ":/translations/"))
        qApp->installTranslator(&commonTranslator);
    if(myappTranslator.load("vstboard_" + QLocale::system().name(), ":/translations/"))
        qApp->installTranslator(&myappTranslator);
#endif
    myHost = new MainHostVst(this,0,"plugin/");
    if(myHost->doublePrecision)
        canDoubleReplacing(true);
    connect(myHost, SIGNAL(DelayChanged(long)),
            this, SLOT(DelayChanged(long)));

    myWindow = new MainWindowVst(myHost);
    qEditor = new Gui(this);
    setEditor(qEditor);
    qEditor->SetMainWindow(myWindow);
}

Vst::~Vst ()
{

}


void Vst::open()
{
//     char str[64];
//     getHostVendorString(str);
//     getHostProductString(str);
//     VstInt32 hostVer = getHostVendorVersion();

     hostSendVstEvents = (bool)canHostDo("sendVstEvents");
     hostSendVstMidiEvent = (bool)canHostDo("sendVstMidiEvent");
     hostSendVstTimeInfo = (bool)canHostDo("sendVstTimeInfo");
     hostReceiveVstEvents = (bool)canHostDo("receiveVstEvents");
     hostReceiveVstMidiEvents = (bool)canHostDo("receiveVstMidiEvents");
     hostReceiveVstTimeInfo = (bool)canHostDo("receiveVstTimeInfo");
     hostReportConnectionChanges = (bool)canHostDo("reportConnectionChanges");
     hostAcceptIOChanges = (bool)canHostDo("acceptIOChanges");

     //reaper returns false :-/
     char str[64];
     getHostProductString(str);
     if(!strcmp(str,"REAPER")) {
        hostAcceptIOChanges = true;
     }

//     long hostAsyncProcessing = canHostDo("asyncProcessing");
//     long hostOffline = canHostDo("offline");
//     long hostSupplyIdle = canHostDo("supplyIdle");
//     long hostSupportShell = canHostDo("supportShell");
//     long hostOpenFileSelector = canHostDo("openFileSelector");
//     long hostEditFile = canHostDo("editFile");
//     long hostCloseFileSelector = canHostDo("closeFileSelector");

     if(hostReceiveVstTimeInfo) {
        connect(myHost,SIGNAL(TempoChanged()),
             this,SLOT(OnMainHostTempoChange()));
    }

     myHost->Open();

     //load default setup file
     QString currentSetupFile = ConfigDialog::defaultSetupFile(myHost);
     if(!currentSetupFile.isEmpty()) {
         if(!ProjectFile::LoadFromFile(myHost,currentSetupFile))
             currentSetupFile = "";
     }

     //load default project file
     QString currentProjectFile = ConfigDialog::defaultProjectFile(myHost);
     if(!currentProjectFile.isEmpty()) {
         if(!ProjectFile::LoadFromFile(myHost,currentProjectFile))
             currentProjectFile = "";
     }

    opened=true;
    myWindow->readSettings();
    myWindow->LoadDefaultFiles();
}

void Vst::close()
{
    foreach(Connectables::VstAudioDeviceIn* dev, lstAudioIn) {
        removeAudioIn(dev);
    }
    foreach(Connectables::VstAudioDeviceOut* dev, lstAudioOut) {
        removeAudioOut(dev);
    }

    opened=false;

    if(myHost)
        myHost->mainWindow=0;
    myWindow->setParent(0);
    qEditor->SetMainWindow(0);
    delete myWindow;
    if(myHost) {
        delete myHost;
        myHost=0;
    }
}

bool Vst::addAudioIn(Connectables::VstAudioDeviceIn *dev)
{
    QMutexLocker l(&mutexDevices);

    if(lstAudioIn.contains(dev))
        return true;

    if(!hostAcceptIOChanges && lstAudioIn.count()>=DEFAULT_INPUTS)
        return false;

    lstAudioIn << dev;
    dev->setObjectName( QString("Vst audio in %1").arg(lstAudioIn.count()) );
    setNumInputs(lstAudioIn.count()*2);
    ioChanged();
    return true;
}

bool Vst::addAudioOut(Connectables::VstAudioDeviceOut *dev)
{
    QMutexLocker l(&mutexDevices);

    if(lstAudioOut.contains(dev))
        return true;

    if(!hostAcceptIOChanges && lstAudioOut.count()>=DEFAULT_OUTPUTS)
        return false;

    lstAudioOut << dev;
    dev->setObjectName( QString("Vst audio out %1").arg(lstAudioOut.count()) );
    setNumOutputs(lstAudioOut.count()*2);
    ioChanged();
    return true;
}

bool Vst::removeAudioIn(Connectables::VstAudioDeviceIn *dev)
{
    QMutexLocker l(&mutexDevices);
    lstAudioIn.removeAll(dev);
    setNumInputs(lstAudioIn.count()*2);
    if(hostAcceptIOChanges && opened)
        ioChanged();
    return true;
}

bool Vst::removeAudioOut(Connectables::VstAudioDeviceOut *dev)
{
    QMutexLocker l(&mutexDevices);
    lstAudioOut.removeAll(dev);
    setNumOutputs(lstAudioOut.count()*2);
    if(hostAcceptIOChanges && opened)
        ioChanged();
    return true;
}

void Vst::addMidiIn(Connectables::VstMidiDevice *dev)
{
    lstMidiIn << dev;
 }

void Vst::addMidiOut(Connectables::VstMidiDevice *dev)
{
    lstMidiOut << dev;
}

void Vst::removeMidiIn(Connectables::VstMidiDevice *dev)
{
    lstMidiIn.removeAll(dev);
}

void Vst::removeMidiOut(Connectables::VstMidiDevice *dev)
{
    lstMidiOut.removeAll(dev);
}

void Vst::addVstAutomation(Connectables::VstAutomation *dev)
{
    lstVstAutomation << dev;
}

void Vst::removeVstAutomation(Connectables::VstAutomation *dev)
{
    lstVstAutomation.removeAll(dev);
}

VstInt32 Vst::getNumMidiInputChannels()
{
    return 15;
}

VstInt32 Vst::getNumMidiOutputChannels()
{
    return 15;
}

void Vst::setProgramName (char* name)
{
    if(!name)
        return;
    vst_strncpy (programName, name, kVstMaxProgNameLen);
}

void Vst::getProgramName (char* name)
{
    if(!name)
        return;
    vst_strncpy (name, QString("prog%1").arg(currentHostProg).toAscii(), kVstMaxProgNameLen);
}

void Vst::setProgram(VstInt32 program)
{
    currentHostProg=program;
    emit HostChangedProg((int)currentHostProg);
}

VstInt32 Vst::getProgram()
{
    return currentHostProg;
}

void Vst::setParameter (VstInt32 index, float value)
{
    mutexParam.lock();
    listParameters.insert(index,value);
    mutexParam.unlock();

    foreach(Connectables::VstAutomation *dev, lstVstAutomation) {
        dev->ValueFromHost(index,value);
    }
}

float Vst::getParameter (VstInt32 index)
{
    QMutexLocker m(&mutexParam);
    return listParameters.value(index,.0f);
}

void Vst::getParameterName (VstInt32 index, char* label)
{
    if(!label)
        return;
    vst_strncpy (label, QString("p%1").arg(index).toAscii(), kVstMaxParamStrLen);
}

void Vst::getParameterDisplay (VstInt32 index, char* text)
{
    if(!text)
        return;
    QMutexLocker m(&mutexParam);
    float val=.0f;
    if(listParameters.contains(index))
        val=listParameters.value(index);
    float2string (val, text, kVstMaxParamStrLen);
}

void Vst::updateParameter(int index, float value)
{
    mutexParam.lock();
    listParameters.insert(index,value);
    mutexParam.unlock();

    if(!beginEdit(index))
        return;
    setParameterAutomated(index,value);
    endEdit(index);
}

void Vst::getParameterLabel (VstInt32 index, char* label)
{
    if(!label)
        return;
    vst_strncpy (label, "", kVstMaxParamStrLen);
}

bool Vst::getEffectName (char* name)
{
    if(!name)
        return false;
    vst_strncpy (name, "VstBoard", kVstMaxEffectNameLen);
    return true;
}

bool Vst::getProductString (char* text)
{
    if(!text)
        return false;
    vst_strncpy (text, "VstBoard", kVstMaxProductStrLen);
    return true;
}

bool Vst::getVendorString (char* text)
{
    if(!text)
        return false;
    vst_strncpy (text, "CtrlBrk", kVstMaxVendorStrLen);
    return true;
}

VstInt32 Vst::getVendorVersion ()
{
    return 0002;
}

VstInt32 Vst::canDo(char* text)
{
    if(!text)
        return 0;

//     "offline", plug-in supports offline functions (offlineNotify, offlinePrepare, offlineRun)
//     "midiProgramNames", plug-in supports function getMidiProgramName ()
//     "bypass", plug-in supports function setBypass ()

    if (
            (!strcmp(text, "sendVstEvents")) ||
            (!strcmp(text, "sendVstMidiEvent")) ||
            (!strcmp(text, "receiveVstEvents")) ||
            (!strcmp(text, "receiveVstMidiEvent")) ||
            (!strcmp(text, "receiveVstTimeInfo"))
         )
         return 1;

    return 0;

}

VstInt32 Vst::processEvents(VstEvents* events)
{
    if(!events || !hostSendVstEvents || !hostSendVstMidiEvent)
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

void Vst::processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames)
{
    if(!inputs || !outputs || sampleFrames<=0)
        return;

    if(hostSendVstTimeInfo)
        myHost->SetTimeInfo( getTimeInfo( -1));

    if(bufferSize!=sampleFrames) {
        myHost->SetBufferSize((unsigned long)sampleFrames);
        bufferSize=sampleFrames;
    }

    int cpt=0;
    foreach(Connectables::VstAudioDeviceIn* dev, lstAudioIn) {
        dev->SetBuffers(inputs,cpt,sampleFrames);
    }

    myHost->Render();

    cpt=0;
    foreach(Connectables::VstAudioDeviceOut* dev, lstAudioOut) {
        dev->GetBuffers(outputs,cpt,sampleFrames);
    }

    if(hostReceiveVstEvents || hostReceiveVstMidiEvents) {
        //free last buffer
        if(listEvnts) {
            free(listEvnts);
            listEvnts = 0;
        }

        cpt=0;
        foreach(Connectables::VstMidiDevice *dev, lstMidiOut) {
            foreach(long msg, dev->midiQueue) {

                //allocate a new buffer
                if(!listEvnts)
                    listEvnts = (VstEvents*)malloc(sizeof(VstEvents) + sizeof(VstEvents*)*(VST_EVENT_BUFFER_SIZE-2));

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
            sendVstEventsToHost(listEvnts);
        }
    }
}

void Vst::processDoubleReplacing (double** inputs, double** outputs, VstInt32 sampleFrames)
{
    if(!inputs || !outputs || sampleFrames<=0)
        return;

    if(hostSendVstTimeInfo)
        myHost->SetTimeInfo( getTimeInfo( -1));

    if(bufferSize!=sampleFrames) {
        myHost->SetBufferSize((unsigned long)sampleFrames);
        bufferSize=sampleFrames;
    }

    int cpt=0;
    foreach(Connectables::VstAudioDeviceIn* dev, lstAudioIn) {
        dev->SetBuffersD(inputs,cpt,sampleFrames);
    }

    myHost->Render();

    cpt=0;
    foreach(Connectables::VstAudioDeviceOut* dev, lstAudioOut) {
        dev->GetBuffersD(outputs,cpt,sampleFrames);
    }

    if(hostReceiveVstEvents || hostReceiveVstMidiEvents) {
        //free last buffer
        if(listEvnts) {
            free(listEvnts);
            listEvnts = 0;
        }

        cpt=0;
        foreach(Connectables::VstMidiDevice *dev, lstMidiOut) {
            foreach(long msg, dev->midiQueue) {

                //allocate a new buffer
                if(!listEvnts)
                    listEvnts = (VstEvents*)malloc(sizeof(VstEvents) + sizeof(VstEvents*)*(VST_EVENT_BUFFER_SIZE-2));

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
            sendVstEventsToHost(listEvnts);
        }
    }
}

void Vst::setSampleRate(float sampleRate)
{
    if(sampleRate<=0)
        return;
    myHost->SetSampleRate(sampleRate);
}

void Vst::setBlockSize (VstInt32 blockSize)
{
    if(blockSize<=0)
        return;
    bufferSize=blockSize;
    myHost->SetBufferSize((unsigned long)blockSize);
}

void Vst::suspend()
{
    deleteChunkData();
}

void Vst::resume()
{
    deleteChunkData();
}

void Vst::OnMainHostTempoChange()
{
    if (!audioMaster)
        return;

    audioMaster (&cEffect, audioMasterSetTime, 0, 0, &myHost->vstHost->vstTimeInfo, 0);
}

void Vst::deleteChunkData()
{
    if(chunkData) {
        delete[] chunkData;
        chunkData=0;
    }
}

VstInt32 Vst::setChunk ( void* data, VstInt32 byteSize, bool isPreset)
{
    if(!data || byteSize<=0 || isPreset)
        return 0;
    QByteArray tmpStream;
    tmpStream.setRawData((const char*)data,byteSize);
    QDataStream tmp( &tmpStream , QIODevice::ReadOnly);
    if(!ProjectFile::FromStream(myHost,tmp)) {
        myHost->ClearSetup();
        myHost->ClearProject();
    }
    return 0;
}

VstInt32 Vst::getChunk (void** data, bool isPreset)
{
    if(!data || isPreset)
        return 0;
    QByteArray tmpStream;
    QDataStream tmp( &tmpStream , QIODevice::WriteOnly);
    ProjectFile::ToStream(myHost,tmp);

    chunkData = new char[tmpStream.size()];
    memcpy(chunkData, tmpStream.data(), tmpStream.size());
    *data=chunkData;
    return tmpStream.size();
}

void Vst::DelayChanged(long samples)
{
    setInitialDelay( static_cast<VstInt32>(samples) );
    ioChanged();
}
*/
