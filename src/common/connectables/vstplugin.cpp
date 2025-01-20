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

//#ifdef VST24SDK

#include <cstring>
#include "vstplugin.h"
#include "globals.h"
#include "audiobuffer.h"
#include "mainwindow.h"
#include "vst/cvsthost.h"
#include "views/vstpluginwindow.h"
#include "commands/comaddpin.h"
#include "commands/comremovepin.h"

using namespace Connectables;

VstPlugin *VstPlugin::pluginLoading = 0;
QMap<AEffect*,VstPlugin*>VstPlugin::mapPlugins;
//View::VstShellSelect *VstPlugin::shellSelectView=0;

VstPlugin::VstPlugin(MainHost *myHost,int index, const ObjectInfo & info) :
    Object(myHost,index, info),
    CEffect(),
    editorWnd(0),
    sampleRate(44100.0),
    bufferSize(1024),
    listEvnts(0),
    savedChunk(0),
    savedChunkSize(0),
    bypass(false),
    hasEditor(false)
{
    for(int i=0;i<128;i++) {
        listValues << i;
    }
    listBypass << "On" << "Bypass" << "Mute";

}

VstPlugin::~VstPlugin()
{
    Close();
    if(savedChunk)
        delete savedChunk;
}

bool VstPlugin::Close()
{
    if(editorWnd) {
        editorWnd->disconnect();
        editorWnd->UnsetPlugin();
        disconnect(editorWnd);
        QTimer::singleShot(0,editorWnd,SLOT(close()));
        editorWnd=0;
        objMutex.lock();
        EffEditClose();
        objMutex.unlock();
    }
    mapPlugins.remove(pEffect);

    if(listEvnts) {
        free(listEvnts);
        listEvnts = 0;
    }
    return true;
}

void VstPlugin::SetSleep(bool sleeping)
{
    if(closed)
        return;

	QMutexLocker objlock(&objMutex);
    
    if(sleeping) {
        EffStopProcess();
        EffSuspend();

    } else {
        foreach(Pin *in, listAudioPinIn->listPins ) {
            if(in) {
                AudioPin *audioIn = static_cast<AudioPin*>(in);
                audioIn->GetBuffer()->ResetStackCounter();
            }
        }
        EffResume();
        EffStartProcess();
    }

    Object::SetSleep(sleeping);
}

void VstPlugin::SetBufferSize(qint32 size)
{
    if(closed)
        return;

    if(size==bufferSize)
        return;

    bool wasSleeping = GetSleep();
    if(!wasSleeping)
        SetSleep(true);

//    debug("VstPlugin::SetBufferSize %d size %ld -> %ld",index,bufferSize,size)
    Object::SetBufferSize(size);

    EffSetBlockSize((long)size);

    bufferSize = size;

    if(!wasSleeping)
        SetSleep(false);
}

void VstPlugin::SetSampleRate(float rate)
{
    sampleRate = rate;

    if(closed)
        return;

    bool wasSleeping = GetSleep();
    SetSleep(true);

    EffSetSampleRate(rate);

    SetSleep(wasSleeping);
}

void VstPlugin::Render()
{
    if(bypass) {
        foreach(Pin *in, listAudioPinIn->listPins ) {
            if(in) {
                AudioPin *audioIn = static_cast<AudioPin*>(in);
                AudioPin *audioOut = 0;
                if(listAudioPinOut->listPins.size() > in->GetConnectionInfo().pinNumber) {
                    audioOut = static_cast<AudioPin*>(listAudioPinOut->GetPin( in->GetConnectionInfo().pinNumber ));
                }

                if(audioIn) {
                    if(audioOut) {
                        audioOut->GetBuffer()->AddToStack( audioIn->GetBuffer() );
                        audioOut->GetBuffer()->ConsumeStack();
                        audioOut->SendAudioBuffer();
                        audioOut->GetBuffer()->ResetStackCounter();
                    }
                    audioIn->GetBuffer()->ConsumeStack();
                }
            }
        }
        return;
    }

    if(closed) // || GetSleep())
        return;

    QMutexLocker lock(&objMutex);

    //midi events
    //=========================
    if(bWantMidi) {
        midiEventsMutex.lock();

        if(listVstMidiEvents.size()!=0) {
            int nbEvents = listVstMidiEvents.size();

            if(listEvnts) {
                free(listEvnts);
                listEvnts = 0;
            }

            listEvnts = (VstEvents*)malloc(sizeof(VstEvents) + sizeof(VstEvents*)*(nbEvents-2));
            listEvnts->numEvents = nbEvents;

            int cpt=0;
            foreach(VstMidiEvent *evnt, listVstMidiEvents) {
                listEvnts->events[cpt] = (VstEvent*)evnt;
                cpt++;
            }

            EffProcessEvents(listEvnts);
        }
    }

	ulong newbuffsize = 0;
    if(!listAudioPinIn->listPins.isEmpty()) {
        AudioPin* p = static_cast<AudioPin*>(listAudioPinIn->listPins.first());
        newbuffsize = p->GetBuffer()->GetSize();
    }
    if(!listAudioPinOut->listPins.isEmpty()) {
        AudioPin* p = static_cast<AudioPin*>(listAudioPinOut->listPins.first());
        if(newbuffsize < p->GetBuffer()->GetSize()) {
            newbuffsize = p->GetBuffer()->GetSize();
        }
	}

	if (newbuffsize != 0) {
		SetBufferSize(newbuffsize);
	}

	if(doublePrecision) {
        if (pEffect->flags & effFlagsCanDoubleReplacing) {
            double **tmpBufOut = new double*[listAudioPinOut->listPins.size()];

            int cpt=0;
            foreach(Pin* pin,listAudioPinOut->listPins) {
                AudioPin *audioPin = static_cast<AudioPin*>(pin);
                tmpBufOut[cpt] = (double*)audioPin->GetBuffer()->GetPointerWillBeFilled();
                cpt++;
            }

            double **tmpBufIn;
            if(listAudioPinIn->listPins.isEmpty()) {
                //no input, use outputs as fake buffers... don't know what we're supposed to do...
                tmpBufIn = tmpBufOut;
            } else {
                tmpBufIn = new double*[listAudioPinIn->listPins.size()];

                cpt=0;
                foreach(Pin* pin,listAudioPinIn->listPins) {
                    tmpBufIn[cpt] = (double*)static_cast<AudioPin*>(pin)->GetBuffer()->ConsumeStack();
                    cpt++;
                }
            }

            EffProcessDoubleReplacing(tmpBufIn, tmpBufOut, bufferSize);

            //clear buffers
            //=========================
            if(tmpBufOut!=tmpBufIn)
                delete[] tmpBufIn;

            delete[] tmpBufOut;

        } else {
            LOG("DoubleReplacing not supported");
        }
    } else {
        float **tmpBufOut = new float*[listAudioPinOut->listPins.size()];

        int cpt=0;
        foreach(Pin* pin,listAudioPinOut->listPins) {
			if (pin) {
				AudioPin *audioPin = static_cast<AudioPin*>(pin);
				tmpBufOut[cpt] = (float*)audioPin->GetBuffer()->GetPointerWillBeFilled();
			}
			else {
				LOG("no out pin");
			}
            cpt++;
        }

        float **tmpBufIn;
        if(listAudioPinIn->listPins.isEmpty()) {
            //no input, don't know what we're supposed to do...
            tmpBufIn = tmpBufOut;
        } else {
            tmpBufIn = new float*[listAudioPinIn->listPins.size()];

            cpt=0;
            foreach(Pin* pin,listAudioPinIn->listPins) {
                tmpBufIn[cpt] = (float*)static_cast<AudioPin*>(pin)->GetBuffer()->ConsumeStack();
                cpt++;
            }
        }

        //process
        //=========================

        if (pEffect->flags & effFlagsCanReplacing) {
            EffProcessReplacing(tmpBufIn, tmpBufOut, bufferSize);
        } else {
            EffProcess(tmpBufIn, tmpBufOut, bufferSize);
        }

        //clear buffers
        //=========================
        if(tmpBufOut!=tmpBufIn)
            delete[] tmpBufIn;

        delete[] tmpBufOut;
    }

    //clear midi events
    //=========================
    if (bWantMidi) {
        listVstMidiEvents.clear();
        midiEventsMutex.unlock();
    }

    //send result
    //=========================
    foreach(Pin* pin,listAudioPinOut->listPins) {
        static_cast<AudioPin*>(pin)->GetBuffer()->ConsumeStack();
        static_cast<AudioPin*>(pin)->SendAudioBuffer();
    }

    EffIdle();
}

bool VstPlugin::Open()
{
    {
        QMutexLocker lock(&objMutex);
        VstPlugin::pluginLoading = this;

        if(objInfo.filename.endsWith(VST_BANK_FILE_EXTENSION,Qt::CaseInsensitive) || objInfo.filename.endsWith(VST_PROGRAM_FILE_EXTENSION,Qt::CaseInsensitive)) {
            bankToLoad=objInfo.filename;
            VstInt32 i = IdFromFxb(bankToLoad);
            if(i==0) {
                LOG("plugin id not found");
                return false;
            }
            if(!FilenameFromDatabase(i,objInfo.filename)) {
                QMessageBox msg(QMessageBox::Critical,"Unknown Id",tr("Id %1 not in database, load the corresponding plugin first").arg(i));
                msg.exec();
                return false;
            }
        }

        QFileInfo inf(objInfo.filename);
        setObjectName(inf.baseName());

        if(!Load(objInfo.filename )) {
            VstPlugin::pluginLoading = 0;
            SetErrorMessage( tr("Error while loading the plugin") );
            //return true to create a dummy object
            return true;
        }

        VstPlugin::pluginLoading = 0;
    }

    if(EffGetPlugCategory() == kPlugCategShell && objInfo.id==0) {
        QList<quint32>ids;

//        MsgObject msg(FixedObjId::shellselect);
        _MSGOBJ(msg,FixedObjId::shellselect);
        msg.prop[MsgObject::Id] = GetIndex();
        msg.prop[MsgObject::ObjInfo] = QVariant::fromValue(objInfo);
        char szName[256];
        quint32 id;
        while ((id = EffGetNextShellPlugin(szName))) {
            if(ids.contains(id))
                continue;
            ids << id;
//            MsgObject plug;
            _MSGOBJ(plug,FixedObjId::ND);
            plug.prop[MsgObject::Name].toString() = szName;
            ///plug.prop[MsgObject::Name] = szName;
            plug.prop[MsgObject::Id] = id;
            msg.children << plug;
        }
        ids.clear();

        Unload();
        msgCtrl->SendMsg(msg);

        return true;
    }

    mapPlugins.insert(pEffect, this);
    closed=false;
    VstPlugin::pluginLoading = 0;
    return initPlugin();
}

void VstPlugin::ReceiveMsg(const MsgObject &msg)
{
    //reload a shell plugin with the selected id
    if(msg.prop.contains(MsgObject::ObjInfo) && objInfo.id==0) {
        objInfo = msg.prop[MsgObject::ObjInfo].value<ObjectInfo>();
        if(objInfo.id == 0) {
            myHost->undoStack.undo();
        } else {
            SetMsgEnabled(false);
            int lastProg = currentProgId;
            Object::LoadProgram(TEMP_PROGRAM);
            delete listPrograms.take(lastProg);
            Open();
            SetSleep(false);
            Object::LoadProgram(lastProg);

            SetMsgEnabled(true);
            UpdateView();
        }
        return;
    }

    if(msg.prop.contains(MsgObject::FilesToLoad)) {
        QStringList lstFiles = msg.prop[MsgObject::FilesToLoad].toStringList();
        foreach(const QString filename, lstFiles) {
            if ( filename.endsWith("fxp",Qt::CaseInsensitive) ) {
                LoadProgramFile(filename);
            } else if ( filename.endsWith("fxb",Qt::CaseInsensitive) ) {
                LoadBank(filename);
            }
        }
        return;
    }

    if(msg.prop.contains(MsgObject::Save)) {
        QString filename = msg.prop[MsgObject::Save].toString();
        if( filename.endsWith("fxp",Qt::CaseInsensitive) ) {
            SaveProgramFile(filename);
        } else if ( filename.endsWith("fxb",Qt::CaseInsensitive) ) {
            SaveBank(filename);
        }
        return;
    }

    Object::ReceiveMsg(msg);
}

bool VstPlugin::initPlugin()
{
//    bool wasEnabled = MsgEnabled();
//    if(wasEnabled)
//        SetMsgEnabled(false);

    {
        QMutexLocker lock(&objMutex);

        long ver = EffGetVstVersion();

        bufferSize = myHost->GetBufferSize();
        sampleRate = myHost->GetSampleRate();

        if(!(pEffect->flags & effFlagsCanDoubleReplacing))
            doublePrecision=false;

        listAudioPinIn->ChangeNumberOfPins(pEffect->numInputs);
        listAudioPinOut->ChangeNumberOfPins(pEffect->numOutputs);

        EffSetSampleRate(sampleRate);
        EffSetBlockSize(bufferSize);
        EffOpen();
        EffSetSampleRate(sampleRate);
        EffSetBlockSize(bufferSize);

        //long canSndMidiEvnt = EffCanDo(PlugCanDos::canDoSendVstMidiEvent);
        bWantMidi = (EffCanDo("receiveVstMidiEvent") == 1);
//LOG("sendtime"<<EffCanDo("sendVstTimeInfo"));

     //   long midiPrgNames = EffCanDo("midiProgramNames");
        VstPinProperties pinProp;
        EffGetInputProperties(0,&pinProp);

        //stereo input
        bool stereoIn = false;
        if(pinProp.flags & kVstPinIsStereo)
                stereoIn = true;
        //speaker arrangement
        //pinProp->arrangementType

        EffGetOutputProperties(0,&pinProp);
        //stereo output
        bool stereoOut = false;
        if(pinProp.flags & kVstPinIsStereo)
                stereoOut = true;

        //speaker arrangement
        //pinProp->arrangementType

        EffResume();
        EffSuspend();

        if(stereoIn)
        {
                EffGetInputProperties(0,&pinProp);
                EffGetInputProperties(1,&pinProp);
        }
        if(stereoOut)
        {
                EffGetOutputProperties(0,&pinProp);
                EffGetOutputProperties(1,&pinProp);
        }

        if(bWantMidi)
        {
                EffGetNumMidiInputChannels();
                EffGetNumMidiOutputChannels();
        }

        if(ver>=2000 && ver<2400)
        {
                EffConnectInput(0,1);
                if(stereoIn)
                        EffConnectInput(1,1);

                EffConnectOutput(0,1);
                if(stereoOut)
                        EffConnectOutput(1,1);
        }

        EffSetProgram(0);

        char szBuf[256] = "";
        if ((EffGetProductString(szBuf)) && (*szBuf)) {
            setObjectName( QString(szBuf) );
            objInfo.name=objectName();
        }

        if(bWantMidi) {
            listMidiPinIn->AddPin(0);
            listMidiPinOut->AddPin(0);
        }

        SetInitDelay(pEffect->initialDelay);
    }

    //create all parameters pins
    int nbParam = pEffect->numParams;
    for(int i=0;i<nbParam;i++) {
        listParameterPinIn->AddPin(i);
    }

    if(pEffect->flags & effFlagsHasEditor) {
        //editor pin
        listParameterPinIn->AddPin(FixedPinNumber::editorVisible);

        //learning pin
        listIsLearning << "off";
        listIsLearning << "learn";
        listIsLearning << "unlearn";
        listParameterPinIn->AddPin(FixedPinNumber::learningMode);
    }
    listParameterPinIn->AddPin(FixedPinNumber::vstProgNumber);
    listParameterPinIn->AddPin(FixedPinNumber::bypass);

    EffSetProgram(0);
    Object::Open();

    if(myHost->settings->GetSetting("fastEditorsOpenClose",true).toBool()) {
        CreateEditorWindow();
    }
    AddPluginToDatabase();

    if(!bankToLoad.isEmpty()) {
        QTimer::singleShot(0,this,SLOT(LoadBank()));
    }

//    if(wasEnabled) {
//        SetMsgEnabled(true);
//        UpdateView();
//    }
    return true;
}

void VstPlugin::AddPluginToDatabase()
{
   myHost->settings->SetSetting(QString("pluginsDb/%1").arg(pEffect->uniqueID),objInfo.filename);
}

bool VstPlugin::FilenameFromDatabase(VstInt32 id, QString &filename)
{
    filename=myHost->settings->GetSetting(QString("pluginsDb/%1").arg(id),"").toString();
    if(filename.isEmpty())
        return false;

    return true;
}

VstInt32 VstPlugin::IdFromFxb(const QString &fxbFile)
{
    std::string str = fxbFile.toStdString();
    return CEffect::PluginIdFromBankFile(&str);
}

void VstPlugin::RaiseEditor()
{
    if(!editorWnd || !editorWnd->isVisible())
        return;

    editorWnd->raise();
}

//void VstPlugin::TakeScreenshot()
//{
//    const QPixmap backgroundPic = editorWnd->GetScreenshot();
//    ImageCollection::Get()->AddImage( QString::number( pEffect->uniqueID ), backgroundPic );
//    modelNode->setData( QString::number( pEffect->uniqueID ), UserRoles::editorImage );
//}

void VstPlugin::CreateEditorWindow()
{
    //already done
    if(editorWnd)
        return;

    //no gui
    if((pEffect->flags & effFlagsHasEditor) == 0)
        return;

    editorWnd = new View::VstPluginWindow(myHost->mainWindow);
    editorWnd->setWindowTitle(objectName());

    if(!editorWnd->SetPlugin(this)) {
        editorWnd=0;
        OnHideEditor();
        return;
    }

    connect(this,SIGNAL(HideEditorWindow()),
            editorWnd,SLOT(hide()),
            Qt::QueuedConnection);
    connect(editorWnd, SIGNAL(Hide()),
            this, SLOT(OnEditorClosed()));
    connect(editorWnd,SIGNAL(destroyed()),
            this,SLOT(EditorDestroyed()));
    connect(this,SIGNAL(WindowSizeChange(int,int)),
            editorWnd,SLOT(SetWindowSize(int,int)));

    //no screenshot in db, create one
//    if(!ImageCollection::Get()->ImageExists(QString::number( pEffect->uniqueID ))) {
//        OnEditorVisibilityChanged(true);
//        connect(MainHost::Get()->updateViewTimer,SIGNAL(timeout()),
//                this,SLOT(EditIdle()));
//        QTimer::singleShot(100,this,SLOT(TakeScreenshot()));
//    }


}

void VstPlugin::OnEditorClosed()
{
    ToggleEditor(false);
}

void VstPlugin::OnShowEditor()
{
    if(!editorWnd)
        CreateEditorWindow();

    if(!editorWnd)
        return;

    if(editorWnd->isVisible())
        return;

    editorWnd->show();
//    editorWnd->raise();
    connect(myHost->updateViewTimer,SIGNAL(timeout()),
            this,SLOT(EditIdle()));
    editorWnd->LoadAttribs(currentViewAttr);
}

void VstPlugin::OnHideEditor()
{
    if(!editorWnd)
        return;

    editorWnd->SaveAttribs(currentViewAttr);

    if(myHost->settings->GetSetting("fastEditorsOpenClose",true).toBool()) {
        disconnect(myHost->updateViewTimer,SIGNAL(timeout()),
            this,SLOT(EditIdle()));
        emit HideEditorWindow();
    } else {
        editorWnd->disconnect();
        editorWnd->UnsetPlugin();
        disconnect(editorWnd);
        QTimer::singleShot(0,editorWnd,SLOT(close()));
        editorWnd=0;
        objMutex.lock();
        EffEditClose();
        objMutex.unlock();
    }
//    if(!editorWnd)
//        CreateEditorWindow();

//    if(!editorWnd->isVisible())
//        return;

//    editorWnd->SaveAttribs();

//    disconnect(myHost->updateViewTimer,SIGNAL(timeout()),
//            this,SLOT(EditIdle()));
//    emit HideEditorWindow();
}

void VstPlugin::SetContainerAttribs(const ObjectContainerAttribs &attr)
{
    Object::SetContainerAttribs(attr);

    if(editorWnd) {
        if(currentViewAttr.editorVisible != editorWnd->isVisible()) {
            editorWnd->setVisible(currentViewAttr.editorVisible);
        }
        if(currentViewAttr.editorVisible)
            editorWnd->LoadAttribs(currentViewAttr);
    }
}

void VstPlugin::GetContainerAttribs(ObjectContainerAttribs &attr)
{
    currentViewAttr.editorVisible=false;
    if(editorWnd) {
        if(editorWnd->isVisible()) {
            editorWnd->SaveAttribs(currentViewAttr);
            currentViewAttr.editorVisible=true;
        }
    }

    Object::GetContainerAttribs(attr);
}

void VstPlugin::EditorDestroyed()
{
    editorWnd = 0;
//    MainHost::GetModel()->setData(modelIndex, false, UserRoles::editorVisible);
    listParameterPinIn->listPins.value(FixedPinNumber::editorVisible)->SetVisible(false);
}

void VstPlugin::EditIdle()
{
    if(editorWnd && editorWnd->isVisible())
        EffEditIdle();
}

QString VstPlugin::GetParameterName(ConnectionInfo pinInfo)
{
    if(closed)
        return "";

    if(pEffect && pinInfo.pinNumber < pEffect->numParams){
        QString s(EffGetParamName(pinInfo.pinNumber));
        char str[256]={0};
        EffGetParamDisplay(pinInfo.pinNumber,str);
        s.append(":");
        s.append(str);
        return s;
    }
    LOG("parameter id out of range"<<pinInfo.pinNumber);
    return "";
}

void VstPlugin::MidiMsgFromInput(long msg)
{
    if(closed)
        return;

    VstMidiEvent *evnt = new VstMidiEvent;
    memset(evnt, 0, sizeof(VstMidiEvent));
    evnt->type = kVstMidiType;
    evnt->byteSize = sizeof(VstMidiEvent);
    memcpy(evnt->midiData, &msg, sizeof(evnt->midiData));
    evnt->flags = kVstMidiEventIsRealtime;

    midiEventsMutex.lock();
    listVstMidiEvents << evnt;
    midiEventsMutex.unlock();
}

void VstPlugin::processEvents(VstEvents* events)
{
    if(closed)
        return;

    VstEvent *evnt=0;

    for(int i=0; i<events->numEvents; i++) {
        evnt=events->events[i];
        if( evnt->type==kVstMidiType) {
            VstMidiEvent *midiEvnt = (VstMidiEvent*)evnt;

            long msg;
            memcpy(&msg, midiEvnt->midiData, sizeof(midiEvnt->midiData));
            MidiPinOut *pin = static_cast<MidiPinOut*>(listMidiPinOut->GetPin(0,true));
            pin->SendMsg(PinMessage::MidiMsg, &msg);
        } else {
            LOG("other vst event");
        }
    }
}

void VstPlugin::UserRemovePin(const ConnectionInfo &info)
{
    if(info.type!=PinType::Parameter)
        return;

    if(info.direction!=PinDirection::Input)
        return;

    if(!info.isRemoveable)
        return;

    if(listParameterPinIn->listPins.contains(info.pinNumber))
        static_cast<ParameterPinIn*>(listParameterPinIn->listPins.value(info.pinNumber))->SetVisible(false);
    OnProgramDirty();
}

void VstPlugin::UserAddPin(const ConnectionInfo &info)
{
    if(info.type!=PinType::Parameter)
        return;

    if(info.direction!=PinDirection::Input)
        return;

    if(listParameterPinIn->listPins.contains(info.pinNumber))
        static_cast<ParameterPinIn*>(listParameterPinIn->listPins.value(info.pinNumber))->SetVisible(true);
    OnProgramDirty();
}

VstIntPtr VstPlugin::OnMasterCallback(long opcode, long index, long value, void *ptr, float opt, long /*currentReturnCode*/)
{
    switch(opcode) {
        case audioMasterAutomate : //0
            //create the parameter pin if needed
            {
                ParameterPin *pin = static_cast<ParameterPin*>(listParameterPinIn->listPins.value(index,0));
                if(!pin)
                    return 0L;

                switch(GetLearningMode()) {
                case LearningMode::unlearn :
                    if(pin->GetVisible())
                        myHost->undoStack.push( new ComRemovePin(myHost, pin->GetConnectionInfo()) );
                    break;

                case LearningMode::learn :
                    if(!pin->GetVisible())
                        myHost->undoStack.push( new ComAddPin(myHost, pin->GetConnectionInfo()) );

                case LearningMode::off :
                    pin->ChangeOutputValue(opt,true);

                }
            }
            break;

        case audioMasterCurrentId : //2
            return objInfo.id;

        case audioMasterIdle : //3
            QApplication::processEvents();
            break;

        case audioMasterPinConnected : { //4
            Pin *p=0;
            if(value==0) {
                p = listAudioPinIn->GetPin(index);
            } else {
                p = listAudioPinOut->GetPin(index);
            }

            //pin in not created
            if(!p)
                return 1L;

            /// \todo check if the pin is really connected
            return 0L;
        }

        case audioMasterWantMidi : //6
            bWantMidi=true;
            return true;

        case audioMasterProcessEvents : //8
            processEvents((VstEvents*)ptr);
            return 1L;

        case audioMasterGetNumAutomatableParameters : //11
            return 128L;

        case audioMasterIOChanged : //13
            if(!pEffect)
                return 0L;
            SetInitDelay(pEffect->initialDelay);
            listAudioPinIn->ChangeNumberOfPins(pEffect->numInputs);
            listAudioPinOut->ChangeNumberOfPins(pEffect->numOutputs);
            return  1L;

        case audioMasterNeedIdle : //14
            EffIdle();
            return 1L;

        case audioMasterSizeWindow : //15
            emit WindowSizeChange((int)index,(int)value);
            break;

        case audioMasterGetSampleRate : //16
            return sampleRate;

        case audioMasterGetInputLatency : //18
//            LOG("input latency");
        case audioMasterGetOutputLatency : //19
//            LOG("output latency");
            return 0L;
        case audioMasterUpdateDisplay : //42
        {
            if(!pEffect)
                return 1L;
            QMap<quint16,Pin*>::iterator i = listParameterPinIn->listPins.begin();
            while(i!=listParameterPinIn->listPins.end()) {
                if(i.key()<pEffect->numParams) {
                    ParameterPin *pin = static_cast<ParameterPin*>(i.value());
                    pin->ChangeOutputValue( EffGetParameter(i.key()), true );
                    pin->setObjectName( EffGetParamName(i.key()) );
                }
                ++i;
            }
            return 1L;
        }
            break;
        case audioMasterBeginEdit : //43
        case audioMasterEndEdit : //44
            return 1L;
    }

    return 0L;
}

void VstPlugin::OnParameterChanged(ConnectionInfo pinInfo, float value)
{
    Object::OnParameterChanged(pinInfo,value);

    if(closed)
        return;

    if(pinInfo.direction == PinDirection::Input) {
//        //editor pin
//        if(pinInfo.pinNumber==FixedPinNumber::editorVisible) {
//            int val = static_cast<ParameterPinIn*>(listParameterPinIn->listPins.value(FixedPinNumber::editorVisible))->GetIndex();
//            LOG("editor"<<val);
//            switch(val) {
//                case 0:
//                    QTimer::singleShot(0, this, SLOT(OnCloseEditor()));
//                    return;
//                case 1:
//                    QTimer::singleShot(0, this, SLOT(OnHideEditor()));
//                    return;
//                case 2:
//                    QTimer::singleShot(0, this, SLOT(OnShowEditor()));
//                    return;
//            }
//        }

        if(pinInfo.pinNumber==FixedPinNumber::vstProgNumber) {
            //program pin
            EffSetProgram( static_cast<ParameterPinIn*>(listParameterPinIn->listPins.value(FixedPinNumber::vstProgNumber))->GetStepIndex() );
            onVstProgramChanged();
            return;
        }

        if(pinInfo.pinNumber<pEffect->numParams) {
            //plugins just don't respect anything...
//            if(EffCanBeAutomated(pinInfo.pinNumber)!=1) {
//                LOG("vst parameter can't be automated"<<pinInfo.pinNumber);
//                return;
//            }
            EffSetParameter(pinInfo.pinNumber,value);
        }

        if(pinInfo.pinNumber==FixedPinNumber::bypass) {
            QString val = static_cast<ParameterPinIn*>(listParameterPinIn->listPins.value(pinInfo.pinNumber))->GetVariantValue().toString();
            if(val=="On") {
                SetSleep(false);
                bypass=false;
            }
            if(val=="Bypass") {
                SetSleep(true);
                SetSleep(false);
                bypass=true;
            }
            if(val=="Mute") {
                SetSleep(true);
                bypass=false;
            }
        }
    }
}

void VstPlugin::LoadBank()
{
    if(bankToLoad.endsWith(VST_BANK_FILE_EXTENSION,Qt::CaseInsensitive))
        LoadBank(bankToLoad);
    if(bankToLoad.endsWith(VST_PROGRAM_FILE_EXTENSION,Qt::CaseInsensitive))
        LoadProgramFile(bankToLoad);
    bankToLoad.clear();
}

/**
  Load FXB file
  \param filename the file name
  \return true on success
  */
bool VstPlugin::LoadBank(const QString &filename)
{
    std::string str = filename.toStdString();
    if(!CEffect::LoadBank(&str))
        return false;
    onVstProgramChanged();

    currentBankFile=filename;

    MSGOBJ();
    msg.prop[MsgObject::Load] = currentBankFile;
    msgCtrl->SendMsg(msg);

    return true;
}


/**
  Save FXB file
  \param filename the file name
  \return true on success
  */
void VstPlugin::SaveBank(const QString &filename)
{
    std::string str = filename.toStdString();
    if(!CEffect::SaveBank(&str))
        return;

    currentBankFile=filename;

    MSGOBJ();
    msg.prop[MsgObject::Load] = currentBankFile;
    msgCtrl->SendMsg(msg);
}

/**
  Load FXP file
  \param filename the file name
  \return true on success
  */
bool VstPlugin::LoadProgramFile(const QString &filename)
{
    std::string str = filename.toStdString();
    if(!CEffect::LoadProgram(&str))
        return false;
    onVstProgramChanged();

    currentBankFile=filename;

    MSGOBJ();
    msg.prop[MsgObject::Load] = currentBankFile;
    msgCtrl->SendMsg(msg);

    return true;
}


/**
  Save FXP file
  \param filename the file name
  \return true on success
  */
void VstPlugin::SaveProgramFile(const QString &filename)
{
    std::string str = filename.toStdString();
    if(!CEffect::SaveProgram(&str))
        return;

    currentBankFile=filename;

    MSGOBJ();
    msg.prop[MsgObject::Load] = currentBankFile;
    msgCtrl->SendMsg(msg);
}

/**
  Send program's values to each parameter pin
  */
void VstPlugin::onVstProgramChanged()
{
    for(int i=0; i<pEffect->numParams; i++) {
        ParameterPinIn *pin = static_cast<ParameterPinIn*>(listParameterPinIn->GetPin(i,false));
        if(pin) {
            pin->ChangeValue(EffGetParameter(i));
        }
    }
}

Pin* VstPlugin::CreatePin(const ConnectionInfo &info)
{
    //if the plugin has a gui, the pins can be learned and the name can change
    hasEditor = (!pEffect || (pEffect->flags & effFlagsHasEditor) == 0)?false:true;

    Pin *newPin = Object::CreatePin(info);
    if(newPin)
        return newPin;

    pinConstructArgs args(info);
    args.parent = this;
    args.bufferSize = myHost->GetBufferSize();
    args.doublePrecision = doublePrecision;


    if(info.type == PinType::Parameter && info.direction == PinDirection::Input) {

        ParameterPin *pin=0;

        switch(info.pinNumber) {
            case FixedPinNumber::vstProgNumber :
                args.name = tr("prog");
                args.listValues = &listValues;
                args.defaultVariantValue = 0;
                return PinFactory::MakePin(args);
//                return new ParameterPinIn(this,info.pinNumber,0,&listValues,"prog");
            case FixedPinNumber::learningMode :
                if(!hasEditor && !IsInError())
                    return 0;
                args.name = tr("Learn");
                args.listValues = &listIsLearning;
                args.defaultVariantValue = "off";
                return PinFactory::MakePin(args);
//                return new ParameterPinIn(this,info.pinNumber,"off",&listIsLearning,tr("Learn"));
            case FixedPinNumber::bypass :
                args.name = tr("Bypass");
                args.listValues = &listBypass;
                args.defaultVariantValue = "On";
                return PinFactory::MakePin(args);
//                return new ParameterPinIn(this,info.pinNumber,"On",&listBypass);
            default :
                if(!closed) {
                    args.name = EffGetParamName(info.pinNumber);
                    args.value = EffGetParameter(info.pinNumber);
                }
                args.visible = !hasEditor;
                return PinFactory::MakePin(args);

//                if(closed) {
//                    pin = new ParameterPinIn(this,info.pinNumber,0,"",true,hasEditor);
//                } else {
//                    pin = new ParameterPinIn(this,info.pinNumber,EffGetParameter(info.pinNumber),EffGetParamName(info.pinNumber),hasEditor,hasEditor);
//                }
//                pin->SetDefaultVisible(!hasEditor);
                return pin;
        }
    }

    return 0;
}

void VstPlugin::fromJson(QJsonObject &json)
{
    Object::fromJson(json);

    if(savedChunk) {
        delete savedChunk;
        savedChunk=0;
        savedChunkSize=0;
    }

    if(json.contains("chunk")) {
        QByteArray ba = QByteArray::fromHex( json["chunk"].toString().toUtf8() );

        savedChunkSize = ba.size();
        savedChunk = new char[savedChunkSize];
        std::memcpy(savedChunk,ba.constData(),savedChunkSize);

        if(pEffect && (pEffect->flags & effFlagsProgramChunks)) {
            EffSetChunk(savedChunk ,savedChunkSize);
        }
    }

}

void VstPlugin::toJson(QJsonObject &json) const
{
    Object::toJson(json);

    //if plugin in error, save last known chunk
    if(!errorMessage.isEmpty() && savedChunk) {
        QByteArray ba(QByteArray::fromRawData(savedChunk, savedChunkSize));
        json["chunk"] = QString(ba.toHex());

    //else save current chunk
    } else if(pEffect && (pEffect->flags & effFlagsProgramChunks)) {
        void *ptr=0;
        long size = EffGetChunk(&ptr,false);
        QByteArray ba(QByteArray::fromRawData((char*)ptr, size));
        json["chunk"] = QString(ba.toHex());
    }
}

QDataStream & VstPlugin::toStream(QDataStream & out) const
{
    Object::toStream(out);

    if(!errorMessage.isEmpty() && savedChunk) {
        out << savedChunkSize;
        out.writeRawData(savedChunk, savedChunkSize);
    } else if(pEffect && (pEffect->flags & effFlagsProgramChunks)) {
        void *ptr=0;
        quint32 size = (quint32)EffGetChunk(&ptr,false);
        out << size;
        out.writeRawData((char*)ptr, size);
    } else {
        out << (quint32)0;
    }
    return out;
}

bool VstPlugin::fromStream(QDataStream & in)
{
    if(!Object::fromStream(in))
        return false;

    in >> savedChunkSize;

    if(savedChunk) {
        delete savedChunk;
        savedChunk=0;
    }

    if(savedChunkSize!=0) {
        savedChunk = new char[savedChunkSize];
        in.readRawData(savedChunk,savedChunkSize);

        if(pEffect && (pEffect->flags & effFlagsProgramChunks)) {
            EffSetChunk(savedChunk,savedChunkSize);
        }
    }
    return true;
}

void VstPlugin::GetInfos(MsgObject &msg)
{
    Object::GetInfos(msg);
    msg.prop[MsgObject::Load] = currentBankFile;
}

//#endif
