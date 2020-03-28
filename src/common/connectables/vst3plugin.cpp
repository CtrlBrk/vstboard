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

#ifdef VSTSDK

#include "vst3plugin.h"
#include "mainhost.h"
#include "mainwindow.h"
#include "views/vstpluginwindow.h"

//#include "public.sdk/source/common/pluginview.h"
#include "commands/comaddpin.h"
#include "commands/comremovepin.h"

namespace Steinberg {
    FUnknown* gStandardPluginContext = 0;
}

using namespace Connectables;
using namespace Steinberg;

extern "C"
{
    typedef bool (PLUGIN_API *InitModuleProc) ();
    typedef bool (PLUGIN_API *ExitModuleProc) ();
}

Vst3Plugin::Vst3Plugin(MainHost *host, int index, const ObjectInfo &info) :
    Object(host,index,info),
    editorWnd(0),
    pluginLib(0),
    factory(0),
    processorComponent(0),
    editController(0),
    audioEffect(0),
    iConnectionPointComponent(0),
    iConnectionPointController(0),
    hasEditor(false),
    pView(0),
    bypass(false),
    progChangeParameter(-1),
    bypassParameter(FixedPinNumber::bypass)
{
    for(int i=0;i<128;i++) {
        listValues << i;
    }

    listBypass << "On" << "Bypass" << "Mute";
    listEditorVisible << "hide" << "show";
    listIsLearning << "off" << "learn" << "unlearn";
}

Vst3Plugin::~Vst3Plugin()
{
    Close();
}

bool Vst3Plugin::Open()
{
    pluginLib = new QLibrary(objInfo.filename, this);
    if(!pluginLib->load()) {
        Unload();
        return false;
    }

    InitModuleProc initProc = (InitModuleProc)pluginLib->resolve("InitDll");
    if (initProc)
    {
        if (initProc () == false)
        {
            Unload();
            return false;
        }
    }
    GetFactoryProc entryProc = (GetFactoryProc)pluginLib->resolve("GetPluginFactory");

    if(!entryProc) {
        Unload();
        return false;
    }
    factory = entryProc();

    if(objInfo.id!=FixedObjId::noContainer) {
        return initPlugin();
    }

    int countPlugins = 0;
    for (qint32 i = 0; i < factory->countClasses(); i++) {
        PClassInfo classInfo;
        factory->getClassInfo(i, &classInfo);

        if (strcmp(kVstAudioEffectClass, classInfo.category)==0) {
            objInfo.id=i;
            ++countPlugins;
        }
    }
    if(countPlugins==0) {
        SetErrorMessage( tr("no plugin found in this dll") );
        return true;
    }
    if(countPlugins>1) {
        //shell
        objInfo.id=FixedObjId::noContainer;
        MsgObject msg(FixedObjId::shellselect);
        msg.prop[MsgObject::Id] = GetIndex();
        msg.prop[MsgObject::ObjInfo] = QVariant::fromValue(objInfo);

        for (qint32 i = 0; i < factory->countClasses(); i++) {
            PClassInfo classInfo;
            if(factory->getClassInfo(i, &classInfo)==kResultOk) {
                if (strcmp(kVstAudioEffectClass, classInfo.category)==0) {
                    MsgObject plug;
                    plug.prop[MsgObject::Name] = classInfo.name;
                    plug.prop[MsgObject::Id] = i;
                    msg.children << plug;
                }
            }
        }
        msgCtrl->SendMsg(msg);
        return true;
    }
    return initPlugin();
}

void Vst3Plugin::ReceiveMsg(const MsgObject &msg)
{
    //reload a shell plugin with the selected id
    if(msg.prop.contains(MsgObject::ObjInfo) && objInfo.id==FixedObjId::noContainer) {
        objInfo = msg.prop[MsgObject::ObjInfo].value<ObjectInfo>();
        if(objInfo.id == FixedObjId::noContainer) {
            myHost->undoStack.undo();
        } else {
            SetMsgEnabled(false);
            int lastProg = currentProgId;
            Object::LoadProgram(TEMP_PROGRAM);
            delete listPrograms.take(lastProg);
            initPlugin();
            SetSleep(false);
            Object::LoadProgram(lastProg);

            SetMsgEnabled(true);
            UpdateView();
        }
        return;
    }

    Object::ReceiveMsg(msg);
}

bool Vst3Plugin::initPlugin()
{
    //    PFactoryInfo factoryInfo ;
    //    factory->getFactoryInfo (&factoryInfo);
    //    LOG("Factory Info:\n\tvendor = " << factoryInfo.vendor << "\n\turl = " << factoryInfo.url << "\n\temail = " << factoryInfo.email)

    //        char8 cidString[50];
    //        FUID(classInfo.cid).toRegistryString (cidString);
    //        QString cidStr(cidString);
    //        LOG("  Class Info " << i << ":\n\tname = " << classInfo.name << "\n\tcategory = " << classInfo.category << "\n\tcid = " << cidStr);



    if(!initProcessor())
        return true;
    if(!initController())
        return true;

    return true;
}

bool Vst3Plugin::initProcessor()
{
    PClassInfo classInfo;
    if(factory->getClassInfo(objInfo.id, &classInfo)!=kResultOk) {
        SetErrorMessage( tr("can't get classInfo") );
        return true;
    }
    setObjectName(classInfo.name);
    objInfo.name=objectName();

    //create processor
    tresult result = factory->createInstance (classInfo.cid, Vst::IComponent::iid, (void**)&processorComponent);
    if (!processorComponent || (result != kResultOk)) {
        SetErrorMessage( tr("Processor not created") );
        return true;
    }
    if(processorComponent->initialize(myHost->vst3Host) != kResultOk) {
        SetErrorMessage( tr("plugin not initialized") );
        return true;
    }

    //init audio effect
    tresult idAudioEff = processorComponent->queryInterface (Vst::IAudioProcessor::iid, (void**)&audioEffect);
    if (idAudioEff == kResultTrue) {
        Vst::ProcessSetup processSetup;
        memset (&processSetup, 0, sizeof (Vst::ProcessSetup));
        processSetup.processMode = Vst::kRealtime;
        if(doublePrecision)
            processSetup.symbolicSampleSize = Vst::kSample64;
        else
            processSetup.symbolicSampleSize = Vst::kSample32;

        processSetup.maxSamplesPerBlock = myHost->GetBufferSize();
        processSetup.sampleRate = myHost->GetSampleRate();

        if(audioEffect->setupProcessing(processSetup)!= kResultOk) {
            SetErrorMessage( tr("processSetup not accepted") );
            return true;
        }
    }

    closed=false;

	processData.prepare(*processorComponent);
	if (doublePrecision)
		processData.symbolicSampleSize = Vst::kSample64;
	else
		processData.symbolicSampleSize = Vst::kSample32;
	processData.numSamples = myHost->GetBufferSize();

	if (idAudioEff == kResultTrue) {
        //init busses
        initAudioBuffers(Vst::kInput);
        initAudioBuffers(Vst::kOutput);
    }

    //midi in
    qint32 cpt=0;
    qint32 numBusEIn = processorComponent->getBusCount(Vst::kEvent, Vst::kInput);
    for (qint32 i = 0; i < numBusEIn; i++) {
        Vst::BusInfo busInfo = {0};
        if(processorComponent->getBusInfo(Vst::kEvent, Vst::kInput, i, busInfo) == kResultTrue) {
            for(qint32 j=0; j<busInfo.channelCount; j++) {
                Pin *p = listMidiPinIn->AddPin(cpt++);
                p->setObjectName( QString::fromUtf16((char16_t*)busInfo.name) );
            }
        }
    }

    //midi out
    cpt=0;
    qint32 numBusEOut = processorComponent->getBusCount(Vst::kEvent, Vst::kOutput);
    for (qint32 i = 0; i < numBusEOut; i++) {
        Vst::BusInfo busInfo = {0};
        if(processorComponent->getBusInfo(Vst::kEvent, Vst::kOutput, i, busInfo) == kResultTrue) {
            for(qint32 j=0; j<busInfo.channelCount; j++) {
                Pin *p = listMidiPinOut->AddPin(cpt++);
                p->setObjectName( QString::fromUtf16((char16_t*)busInfo.name) );
            }
        }
    }

    return true;
}

bool Vst3Plugin::initAudioBuffers(Vst::BusDirection dir)
{
	if (dir == Vst::kInput) {
		listAudioPinIn->setObjectName(0);
	}
	else {
		listAudioPinOut->SetNbPins(0);
	}

    qint32 cpt=0;
    qint32 numBusIn = processorComponent->getBusCount(Vst::kAudio, dir);
    for (qint32 i = 0; i < numBusIn; i++) {
        Vst::BusInfo busInfo = {0};
        if(processorComponent->getBusInfo(Vst::kAudio, dir, i, busInfo) == kResultTrue) {
            for(qint32 j=0; j<busInfo.channelCount; j++) {
                Pin *p = 0;
                if(dir==Vst::kInput) {
                    p = listAudioPinIn->AddPin(cpt++);
                } else {
                    p = listAudioPinOut->AddPin(cpt++);
                }
                p->setObjectName( QString::fromUtf16((char16_t*)busInfo.name) );
                AudioBuffer* buff = static_cast<AudioPin*>(p)->GetBuffer();
                if(doublePrecision) {
                    processData.setChannelBuffer64(dir, i, j, (double*)buff->GetPointer());
                } else {
                    processData.setChannelBuffer(dir, i, j, (float*)buff->GetPointer());
                }
            }
        }
    }

    return true;
}

bool Vst3Plugin::initController()
{
    //create controller
    if (processorComponent->queryInterface (Vst::IEditController::iid, (void**)&editController) != kResultTrue) {
        FUID controllerCID;
        bool initOk=false;
        if (processorComponent->getControllerClassId (controllerCID) == kResultTrue && controllerCID.isValid ()) {
            tresult result = factory->createInstance (controllerCID, Vst::IEditController::iid, (void**)&editController);
            if (editController && (result == kResultOk)) {
                initOk = (editController->initialize (myHost->vst3Host) == kResultOk);
            }
        }

        if(!initOk) {
            SetErrorMessage( tr("can't init controller") );
            return false;
        }
    }

    //init editor window
    if(myHost->settings->GetSetting("fastEditorsOpenClose",true).toBool()) {
        hasEditor = CreateEditorWindow();
    }

    //bypassParameter = FixedPinNumber::bypass;
    progChangeParameter = -1;

    //init parameters
    qint32 numParameters = editController->getParameterCount ();
    for (qint32 i = 0; i < numParameters; i++) {
        Vst::ParameterInfo paramInfo = {0};
        tresult result = editController->getParameterInfo (i, paramInfo);
        if (result == kResultOk) {
            if(paramInfo.flags & Vst::ParameterInfo::kIsBypass) {
                //the plugin's bypass pin
                //don't creat it, we use our own
                bypassParameter = i;
            } else {
                if(paramInfo.flags & Vst::ParameterInfo::kIsProgramChange) {
                    progChangeParameter = i;
                }

                if(paramInfo.flags & Vst::ParameterInfo::kIsReadOnly) {
                    listParameterPinOut->AddPin(i);
                } else {
                    listParameterPinIn->AddPin(i);
                }
            }
        }
    }

    // set the host handler
    if(editController->setComponentHandler(this) != kResultOk) {
        SetErrorMessage( tr("can't set comp. handler") );
        return true;
    }

    //connect processor with controller
    processorComponent->queryInterface (Vst::IConnectionPoint::iid, (void**)&iConnectionPointComponent);
    editController->queryInterface (Vst::IConnectionPoint::iid, (void**)&iConnectionPointController);
    if (iConnectionPointComponent && iConnectionPointController) {
        iConnectionPointComponent->connect (iConnectionPointController);
        iConnectionPointController->connect (iConnectionPointComponent);
    }

    //synchronize controller
    MemoryStream stream;
    if (processorComponent->getState (&stream)) {
        stream.seek(0,IBStream::kIBSeekSet,0);
        editController->setComponentState (&stream);
    }



    //editor pin
    if(hasEditor) {
        listParameterPinIn->AddPin(FixedPinNumber::editorVisible);
    }

    //learning pin
    listParameterPinIn->AddPin(FixedPinNumber::learningMode);

//    listParameterPinIn->AddPin(FixedPinNumber::vstProgNumber);

//    if(bypassParameter==FixedPinNumber::bypass) {
        listParameterPinIn->AddPin(FixedPinNumber::bypass);
//    }

    return true;
}

void Vst3Plugin::SaveProgram()
{
    if(!currentProgram)// || !currentProgram->IsDirty())
        return;

    MemoryStream state;
    if(processorComponent->getState(&state)!=kResultOk) {
        LOG("error saving state")
        return;
    }
    state.seek(0,IBStream::kIBSeekSet,0);
    QByteArray ba(state.getData(), state.getSize());
    currentProgram->listOtherValues.insert(0, ba );

    Object::SaveProgram();
}

void Vst3Plugin::LoadProgram(int prog)
{

    if(prog==currentProgId && currentProgram)
        return;

    bool msgWasEnabled=MsgEnabled();
    SetMsgEnabled(false);

    Object::LoadProgram(prog);

    QByteArray ba = currentProgram->listOtherValues.value(0,QByteArray()).toByteArray();
    if(ba.isEmpty())
        return;

    MemoryStream state;
    int32 nbWritten=0;
    state.write(ba.data(),ba.size(),&nbWritten);
    if(nbWritten!=ba.size()) {
        LOG("stream error")
        return;
    }

    if(processorComponent) {
        state.seek(0,IBStream::kIBSeekSet,0);
        processorComponent->setState(&state);
    }
    if(editController) {
        state.seek(0,IBStream::kIBSeekSet,0);
        editController->setState(&state);
    }

    if(msgWasEnabled)
        SetMsgEnabled(true);
}

bool Vst3Plugin::CreateEditorWindow()
{
    QMutexLocker l(&objMutex);

    //already done
    if(editorWnd)
        return true;

    pView = editController->createView("editor");
    if(!pView) {
        LOG("view not created")
        return false;
    }
//    uint32 r1 = pView->addRef();

#if WINDOWS
    if(pView->isPlatformTypeSupported(kPlatformTypeHWND)!=kResultTrue) {
        LOG("platform not supported (HWND)")
        return false;
    }

#elif MAC
        #if MAC_CARBON
    if(pView->isPlatformTypeSupported(kPlatformTypeHIView)!=kResultTrue) {
        LOG("platform not supported (HIView)")
        return false;
    }
        #endif

        #if MAC_COCOA
    if(pView->isPlatformTypeSupported(kPlatformTypeNSView)!=kResultTrue) {
        LOG("platform not supported (NSView)")
        return false;
    }
        #endif
#endif

    editorWnd = new View::VstPluginWindow(myHost->mainWindow);
    editorWnd->SetPlugin(this);
    editorWnd->setWindowTitle(objectName());

    tresult result;

    result = pView->setFrame(editorWnd);
    if(result!=kResultOk) {
        LOG("frame not set")
    }
    result = pView->attached((void*)editorWnd->GetWinId(),kPlatformString);
    if(result!=kResultOk) {
        LOG("editor not attached")
    }

    ViewRect size;
    if(pView->getSize(&size)!=kResultOk) {
        LOG("can't get size")
//        return false;
    }

    editorWnd->SetWindowSize(size.getWidth(),size.getHeight());

    if(pView->onSize(&size)!=kResultOk) {
        LOG("can't set size")
//        return false;
    }

//    result = pView->canResize();
//    if(result == kResultTrue) {
//        LOG("can resize")
//        ViewRect rec(0,0,600,600);
//        result = pView->checkSizeConstraint(&rec);
//        if(result == kResultTrue) {
////            editorWnd->SetWindowSize(rec.getWidth(),rec.getHeight());
//            result = pView->onSize(&rec);
//        }
//    }

    connect(this,SIGNAL(HideEditorWindow()),
            editorWnd,SLOT(hide()),
            Qt::QueuedConnection);
    connect(editorWnd, SIGNAL(Hide()),
            this, SLOT(OnEditorClosed()));
    connect(editorWnd,SIGNAL(destroyed()),
            this,SLOT(EditorDestroyed()));
    connect(this,SIGNAL(WindowSizeChange(int,int)),
            editorWnd,SLOT(SetWindowSize(int,int)));

    return true;
}

void Vst3Plugin::OnShowEditor()
{
    if(!editorWnd)
        CreateEditorWindow();

    if(!editorWnd)
        return;

    if(editorWnd->isVisible())
        return;

    editorWnd->show();
    editorWnd->LoadAttribs(currentViewAttr);
}

void Vst3Plugin::OnHideEditor()
{
    if(!editorWnd)
        return;

    editorWnd->SaveAttribs(currentViewAttr);

    if(myHost->settings->GetSetting("fastEditorsOpenClose",true).toBool()) {
        emit HideEditorWindow();
    } else {
        RemoveGui();
        editorWnd->disconnect();
        editorWnd->UnsetPlugin();
        disconnect(editorWnd);
        QTimer::singleShot(0,editorWnd,SLOT(close()));
        editorWnd=0;

    }
}

void Vst3Plugin::OnEditorClosed()
{
    ToggleEditor(false);
}

void Vst3Plugin::EditorDestroyed()
{
    editorWnd=0;
    RemoveGui();
    listParameterPinIn->listPins.value(FixedPinNumber::editorVisible)->SetVisible(false);
}

void Vst3Plugin::SetContainerAttribs(const ObjectContainerAttribs &attr)
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

void Vst3Plugin::GetContainerAttribs(ObjectContainerAttribs &attr)
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

void Vst3Plugin::RemoveGui()
{
//    objMutex.lock();
    if(!pView)
        return;

    tresult result;
    result = pView->removed();
    if(result!=kResultOk) {
        LOG("view not removed")
    }
    result = pView->setFrame(0);
    if(result!=kResultOk) {
        LOG("frame not removed")
    }
    pView=0;

//    objMutex.unlock();
}


void Vst3Plugin::SetBufferSize(unsigned long size)
{
    if(closed)
        return;

    bool wasSleeping = GetSleep();
    if(!wasSleeping)
        SetSleep(true);

    Object::SetBufferSize(size);

	processData.numSamples = size;

    if(!wasSleeping)
        SetSleep(false);
}

void Vst3Plugin::SetSleep(bool sleeping)
{
    if(closed)
        return;

    Lock();

    if(sleeping) {
        audioEffect->setProcessing(false);
        processorComponent->setActive(false);

    } else {
        foreach(Pin *in, listAudioPinIn->listPins ) {
            AudioPin *audioIn = static_cast<AudioPin*>(in);
            audioIn->GetBuffer()->ResetStackCounter();
        }
        processorComponent->setActive(true);
        audioEffect->setProcessing(true);
    }

    Unlock();

    Object::SetSleep(sleeping);
}

bool Vst3Plugin::Close()
{
    SetSleep(true);
    closed=true;
    Unload();
    return true;
}

void Vst3Plugin::Unload()
{
    QMutexLocker l(&objMutex);

    if (iConnectionPointComponent && iConnectionPointController) {
        iConnectionPointComponent->disconnect (iConnectionPointController);
        iConnectionPointController->disconnect (iConnectionPointComponent);
    }
    if(iConnectionPointComponent)
        iConnectionPointComponent->release();
    if(iConnectionPointController)
        iConnectionPointController->release();

    RemoveGui();

    if(editorWnd) {
        editorWnd->disconnect();
        editorWnd->UnsetPlugin();
        disconnect(editorWnd);
//        QTimer::singleShot(0,editorWnd,SLOT(close()));
        editorWnd->close();
        editorWnd=0;
    }


//    Vst::IConnectionPoint* iConnectionPointComponent = 0;
//    Vst::IConnectionPoint* iConnectionPointController = 0;
//    processorComponent->queryInterface (Vst::IConnectionPoint::iid, (void**)&iConnectionPointComponent);
//    editController->queryInterface (Vst::IConnectionPoint::iid, (void**)&iConnectionPointController);
//    if (iConnectionPointComponent && iConnectionPointController) {
//        iConnectionPointComponent->disconnect(iConnectionPointController);
//        iConnectionPointComponent->release();
//        iConnectionPointController->disconnect (iConnectionPointComponent);
//        iConnectionPointController->release();
//    }

    if(audioEffect) {
        audioEffect->release();
    }

    if(processorComponent) {
        processorComponent->setActive(false);
        processorComponent->terminate();
        processorComponent->release();
        processorComponent=0;
    }

    if(editController) {
        editController->terminate();
        editController->release();
        editController=0;
    }

    if(factory) {
        factory->release();
        factory=0;
    }


    if(pluginLib) {
        ExitModuleProc exitProc = (ExitModuleProc)pluginLib->resolve("ExitDll");
        if (exitProc) {
            if(!exitProc()) {
                LOG("exitProc error")
            }
        }

        if(pluginLib->isLoaded())
            if(!pluginLib->unload()) {
//                LOG("can't unload plugin"<< objInfo.filename);
            }
        delete pluginLib;
        pluginLib=0;
    }



}

void Vst3Plugin::Render()
{
    if(bypass) {
        foreach(Pin *in, listAudioPinIn->listPins ) {
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
        return;
    }

    if(closed) // || GetSleep())
        return;

    if(!audioEffect)
        return;

    QMutexLocker lock(&objMutex);

    foreach(Pin *pin, listAudioPinIn->listPins) {
        static_cast<AudioPin*>(pin)->GetBuffer()->ConsumeStack();
     }
    foreach(Pin *pin, listAudioPinOut->listPins) {
        static_cast<AudioPin*>(pin)->GetBuffer()->GetPointerWillBeFilled();
    }


//  input buffers
    qint32 cpt=0;
    qint32 numBusIn = processorComponent->getBusCount(Vst::kAudio, Vst::kInput);
	processData.numInputs = numBusIn;
	for (qint32 busIndex = 0; busIndex < numBusIn; busIndex++) {
        Vst::BusInfo busInfo = {0};
        if(processorComponent->getBusInfo(Vst::kAudio, Vst::kInput, busIndex, busInfo) == kResultTrue) {
            for(qint32 channelIndex=0; channelIndex < busInfo.channelCount; channelIndex++) {
                Pin *pin = listAudioPinIn->GetPin(cpt++);
				if (pin) {
					float *buf = (float*)static_cast<AudioPin*>(pin)->GetBuffer()->ConsumeStack();
					Vst::AudioBusBuffers* bus = &processData.inputs[busIndex];
					if (bus) {
						bus->channelBuffers32[channelIndex] = buf;
					}
				}
				else {
					return;
				}
            }
        }
    }
	

//    output buffers
    cpt=0;
    qint32 numBusOut = processorComponent->getBusCount(Vst::kAudio, Vst::kOutput);
	processData.numOutputs = numBusOut;
    for (qint32 busIndex = 0; busIndex < numBusOut; busIndex++) {
        Vst::BusInfo busInfo = {0};
        if(processorComponent->getBusInfo(Vst::kAudio, Vst::kOutput, busIndex, busInfo) == kResultTrue) {
            for(qint32 channelIndex=0; channelIndex<busInfo.channelCount; channelIndex++) {
                Pin *pin = listAudioPinOut->GetPin(cpt++);
				if (pin) {
					float *buf = (float*)static_cast<AudioPin*>(pin)->GetBuffer()->GetPointerWillBeFilled();
					Vst::AudioBusBuffers* bus = &processData.outputs[busIndex];
					if (bus) {
						bus->channelBuffers32[channelIndex] = buf;
					}
				}
				else {
					return;
				}
            }
        }
    }
	

    Vst::ParameterChanges vstParamChanges;
    {
        QMutexLocker l(&paramLock);

        QMap<qint32,float>::const_iterator i = listParamChanged.constBegin();
        while(i!=listParamChanged.constEnd()) {

            //translate pin number to parameter id
            Vst::ParamID pId=0;
            Vst::ParameterInfo paramInfo;
            tresult result = editController->getParameterInfo (i.key(), paramInfo);
            if (result == kResultOk) {
                pId = paramInfo.id;
            }

            int32 idx=0;
            Vst::IParamValueQueue* queue = vstParamChanges.addParameterData(pId, idx);
            int32 pIdx=0;
            queue->addPoint(0, i.value(), pIdx);
            ++i;
        }
        listParamChanged.clear();
    }
    processData.inputParameterChanges = &vstParamChanges;

    Vst::ParameterChanges vstParamOutChanges;
    processData.outputParameterChanges = &vstParamOutChanges;

    processData.processContext = &myHost->vst3Host->processContext;

//    audioEffect->setProcessing (true);
    tresult result = audioEffect->process (processData);
    if (result != kResultOk) {
        LOG("error while processing")
    }

    //output params
    if(processData.outputParameterChanges) {
        Vst::ParamValue value;
        int32 sampleOffset;
        for (int32 i = 0; i < processData.outputParameterChanges->getParameterCount (); i++) {
            Vst::IParamValueQueue* queue = processData.outputParameterChanges->getParameterData (i);
            if (queue) {
                if (queue->getPoint (queue->getPointCount()-1, sampleOffset, value) == kResultTrue) {
                    performEdit( queue->getParameterId(), value);
                }
            }
        }
    }

//    audioEffect->setProcessing (false);

//    for(int32 i=0; i<vstParamChanges.getParameterCount(); i++) {
//        delete vstParamChanges.getParameterData(i);
//    }
//    vstParamChanges.clearQueue();
//    listParamQueue.clear();

    //send result
    //=========================
    foreach(Pin* pin,listAudioPinOut->listPins) {
        static_cast<AudioPin*>(pin)->GetBuffer()->ConsumeStack();
        static_cast<AudioPin*>(pin)->SendAudioBuffer();
    }
}

void Vst3Plugin::MidiMsgFromInput(long msg)
{
    if(closed)
        return;

    int command = MidiStatus(msg) & MidiConst::codeMask;

    switch(command) {
//        case MidiConst::ctrl: {
//            ChangeValue(MidiData1(msg),MidiData2(msg));
//            break;
//        }
        case MidiConst::prog :
            if(progChangeParameter!=-1) {
                Pin* p = listParameterPinIn->GetPin(progChangeParameter,false);
                if(p) {
                    static_cast<ParameterPin*>(p)->ChangeValue( (int)MidiData1(msg) );
                }
            }
            break;

//        case MidiConst::noteOn : {
//            ChangeValue(para_velocity, MidiData2(msg) );
//            ChangeValue(para_notes+MidiData1(msg), MidiData2(msg) );
//            ChangeValue(para_notepitch, MidiData1(msg) );
//            break;
//        }
//        case MidiConst::noteOff : {
//            ChangeValue(para_notepitch, MidiData1(msg) );
//            ChangeValue(para_notes+MidiData1(msg), MidiData2(msg) );
//            break;
//        }
//        case MidiConst::pitchbend : {
//            ChangeValue(para_pitchbend, MidiData2(msg) );
//            break;
//        }
//        case MidiConst::chanpressure : {
//            ChangeValue(para_chanpress, MidiData1(msg) );
//            break;
//        }
//        case MidiConst::aftertouch : {
//            ChangeValue(para_velocity, MidiData1(msg) );
//            ChangeValue(para_aftertouch, MidiData2(msg) );
//        }
    }

//    Vst::Event *evnt = new Vst::Event;
//    memset(evnt, 0, sizeof(Vst::Event));
//    evnt->type = Vst::Event::kNoteOnEvent;
//    evnt->byteSize = sizeof(VstMidiEvent);
//    memcpy(evnt->midiData, &msg, sizeof(evnt->midiData));
//    evnt->flags = kVstMidiEventIsRealtime;

//    midiEventsMutex.lock();
//    listVstMidiEvents << evnt;
//    midiEventsMutex.unlock();
}

void Vst3Plugin::OnParameterChanged(ConnectionInfo pinInfo, float value)
{
    Object::OnParameterChanged(pinInfo,value);

    if(closed)
        return;

    if(pinInfo.direction == PinDirection::Input) {
        if(pinInfo.pinNumber==FixedPinNumber::vstProgNumber) {
            return;
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

        //send value to the gui
        editController->setParamNormalized(pinInfo.pinNumber, value);

        //send value to the processor
        {
            QMutexLocker l(&paramLock);
            if(pinInfo.pinNumber < FIXED_PIN_STARTINDEX) {
                listParamChanged.insert(pinInfo.pinNumber,value);
            }
            //send our own bypass
            if(pinInfo.pinNumber == FixedPinNumber::bypass) {
                listParamChanged.insert(bypassParameter,bypass?1.0f:.0f);
            }
        }

    }
}

Pin* Vst3Plugin::CreatePin(const ConnectionInfo &info)
{
//    if(info.type == PinType::Audio) {
//        return new AudioPin(this,info.direction,info.pinNumber,myHost->GetBufferSize(),doublePrecision,true);
//    }

    Pin *newPin = Object::CreatePin(info);
    if(newPin)
        return newPin;

    if(info.type == PinType::Parameter) {

        if(info.pinNumber==progChangeParameter) {
            return new ParameterPinIn(this,info.pinNumber,0,&listValues,"prog");
        }

        if(info.pinNumber==FixedPinNumber::bypass) {
            return new ParameterPinIn(this,info.pinNumber,"On",&listBypass);
        }

        switch(info.pinNumber) {
            case FixedPinNumber::editorVisible :
                if(!hasEditor && !IsInError())
                    return 0;
                return new ParameterPinIn(this,info.pinNumber,"show",&listEditorVisible,tr("Editor"));

            case FixedPinNumber::learningMode :
                if(!hasEditor && !IsInError())
                    return 0;
                return new ParameterPinIn(this,info.pinNumber,"off",&listIsLearning,tr("Learn"));

            default : {
                ParameterPin *pin=0;
                Vst::ParameterInfo paramInfo = {0};
                tresult result = editController->getParameterInfo (info.pinNumber, paramInfo);
                if(result != kResultOk)
                    return 0;

                if(info.direction==PinDirection::Output) {
                    pin = new ParameterPinOut(this,info.pinNumber,(float)paramInfo.defaultNormalizedValue,QString::fromUtf16((char16_t*)paramInfo.title),hasEditor,hasEditor);
                } else {
                    pin = new ParameterPinIn(this,info.pinNumber,(float)paramInfo.defaultNormalizedValue,QString::fromUtf16((char16_t*)paramInfo.title),hasEditor,hasEditor);
                }
                if(!hasEditor || (paramInfo.flags & Vst::ParameterInfo::kIsReadOnly))
                    pin->SetDefaultVisible(true);
                else
                    pin->SetDefaultVisible(false);

                return pin;
            }
        }
    }

    return 0;
}


void Vst3Plugin::UserRemovePin(const ConnectionInfo &info)
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

void Vst3Plugin::UserAddPin(const ConnectionInfo &info)
{
    if(info.type!=PinType::Parameter)
        return;

    if(info.direction!=PinDirection::Input)
        return;

    if(listParameterPinIn->listPins.contains(info.pinNumber))
        static_cast<ParameterPinIn*>(listParameterPinIn->listPins.value(info.pinNumber))->SetVisible(true);
    OnProgramDirty();
}

tresult PLUGIN_API Vst3Plugin::queryInterface (const TUID iid, void** obj)
{
    char8 cidString[50];
    FUID(iid).toRegistryString (cidString);
    QString cidStr(cidString);
    LOG(cidStr)

    QUERY_INTERFACE (iid, obj, Vst::IComponentHandler::iid, Vst::IComponentHandler)
//    QUERY_INTERFACE (iid, obj, Vst::IComponentHandler2::iid, Vst::IComponentHandler2)
//    QUERY_INTERFACE (iid, obj, FUnknown::iid, Vst::IComponentHandler)
    *obj = 0;
    return kNoInterface;
}

uint32 PLUGIN_API Vst3Plugin::addRef ()
{
    return 1;
}
uint32 PLUGIN_API Vst3Plugin::release ()
{
    return 0;
}
tresult PLUGIN_API Vst3Plugin::beginEdit (Vst::ParamID /*id*/)
{
    return kResultOk;
}
tresult PLUGIN_API Vst3Plugin::performEdit (Vst::ParamID id, Vst::ParamValue valueNormalized)
{
    ParameterPin *pin = 0;
    if(listParameterPinIn->GetPin(id)) {
        pin = static_cast<ParameterPin*>(listParameterPinIn->GetPin(id));
    }
//    if(listParameterPinOut->GetPin(id)) {
//        pin = static_cast<ParameterPin*>(listParameterPinOut->GetPin(id));
//    }
    if(!pin) {
        LOG("parameter not found")
        return kInvalidArgument;
    }

    switch(GetLearningMode()) {
        case LearningMode::unlearn :
            if(pin->GetVisible())
                myHost->undoStack.push( new ComRemovePin(myHost, pin->GetConnectionInfo()) );
            break;

        case LearningMode::learn :
            if(!pin->GetVisible())
                myHost->undoStack.push( new ComAddPin(myHost, pin->GetConnectionInfo()) );

        case LearningMode::off :
            pin->ChangeOutputValue(static_cast<float>(valueNormalized));

    }

    return kResultOk;
}
tresult PLUGIN_API Vst3Plugin::endEdit (Vst::ParamID /*id*/)
{
    return kResultOk;
}
tresult PLUGIN_API Vst3Plugin::restartComponent (int32 /*flags*/)
{
    return kResultOk;
}

QDataStream & Vst3Plugin::toStream(QDataStream & out) const
{
    Object::toStream(out);

    if(!processorComponent) {
        out << savedState;
        return out;
    }

    MemoryStream state;
    if(processorComponent->getState(&state)!=kResultOk) {
        LOG("error saving state")
        out << savedState;
        return out;
    }

    state.seek(0,IBStream::kIBSeekSet,0);
    int32 len=-1;
    char *buf=new char[1024];
    QByteArray bArray;
    while(len!=0) {
        if(state.read(buf,1024,&len)!=kResultOk)
            len=0;
        bArray.append(buf,len);
    }
    out << bArray;
    return out;
}

bool Vst3Plugin::fromStream(QDataStream & in)
{
    if(!Object::fromStream(in))
        return false;

    in >> savedState;

    if(processorComponent && savedState.size()!=0) {
        MemoryStream state;
        state.write(savedState.data(),savedState.size(),0);
        state.seek(0,IBStream::kIBSeekSet,0);
        processorComponent->setState(&state);
    }
    return true;
}

//tresult PLUGIN_API Vst3Plugin::setDirty (TBool state)
//{

//}

//tresult PLUGIN_API Vst3Plugin::requestOpenEditor (FIDString name)
//{

//}

//tresult PLUGIN_API Vst3Plugin::startGroupEdit ()
//{

//}

//tresult PLUGIN_API Vst3Plugin::finishGroupEdit ()
//{

//}

#endif
