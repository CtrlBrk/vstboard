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

#include "commands/comaddpin.h"
#include "commands/comremovepin.h"
#include "pluginterfaces/vst/ivstmidicontrollers.h"
//#include "pluginterfaces/vst/ivsteditcontroller.h"

//#include "public.sdk/source/vst/hosting/eventlist.h"
//#include "public.sdk/source/vst/hosting/parameterchanges.h"
//#include "public.sdk/source/vst/hosting/stringconvert.h"
//#include "pluginterfaces/vst/ivsteditcontroller.h"
//#include "pluginterfaces/vst/ivstmidicontrollers.h"

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
//    Vst::IComponentHandler(),
//    Vst::IComponentHandler2(),
//    Vst::IComponentHandler3(),
//    Vst::IContextMenuTarget(),
    editorWnd(0),
    pluginLib(0),
    //factory(0),
    component(0),
    editController(0),
//    audioProcessor(0),
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
    listIsLearning << "off" << "learn" << "unlearn";

	gStandardPluginContext = myHost->vst3Host;
}

Vst3Plugin::~Vst3Plugin()
{
    Close();
}

bool Vst3Plugin::Open()
{
    std::string error;
    module = VST3::Hosting::Module::create (objInfo.filename.toStdString(), error);
    if (!module)
    {
        Unload();
        return false;
    }
	
    auto factory = module->getFactory ();

    //we already know what effectID we want
    if(objInfo.apiName!="") {
        return initPlugin();
    }

    //check if multiple audio plugins in the dll
    int countPlugins = 0;
    for (auto& classInfo : factory.classInfos ())
    {
        if (classInfo.category () == kVstAudioEffectClass) {
//			objInfo.apiName = QString::fromStdString(classInfo.ID().toString());
            ++countPlugins;
        }
    }

    //no plugin found
    if(countPlugins==0) {
        SetErrorMessage( tr("no plugin found in this dll") );
        return true;
    }

    //multiple plugins, open selection menu
    if(countPlugins>1) {
        objInfo.apiName = "";
        _MSGOBJ(msg,FixedObjId::shellselect);
        msg.prop[MsgObject::Id] = GetIndex();
        msg.prop[MsgObject::ObjInfo] = QVariant::fromValue(objInfo);

        for (auto& classInfo : factory.classInfos ())
        {
            if (classInfo.category () == kVstAudioEffectClass) {
                _MSGOBJ(plug,FixedObjId::ND);
                plug.prop[MsgObject::Name] = QString::fromStdString( classInfo.name() );
                plug.prop[MsgObject::Id] = QString::fromStdString( classInfo.ID().toString() );
                msg.children << plug;
            }

        }
        msgCtrl->SendMsg(msg);
        return true;
    }

    //only one plugin, open the first found
    return initPlugin();
}

void Vst3Plugin::ReceiveMsg(const MsgObject &msg)
{
    //reload a shell plugin with the selected id
    if(msg.prop.contains(MsgObject::ObjInfo) && objInfo.apiName=="") {
        objInfo = msg.prop[MsgObject::ObjInfo].value<ObjectInfo>();
        if(objInfo.apiName == "") {
            myHost->undoStack.undo();
        } else {
			VST3::UID id;
			id.fromString(objInfo.apiName.toStdString());
            initPlugin();
            SetSleep(false);
			myHost->SetSolverUpdateNeeded();
        }
        return;
    }

    Object::ReceiveMsg(msg);
}

bool Vst3Plugin::initPlugin()
{
	VST3::Optional<VST3::UID> effectID;
	if (objInfo.apiName != "")
	{
		std::string str;
		str = objInfo.apiName.toStdString();
		effectID = VST3::UID::fromString(str);
    }

    //find the selected effectID
    auto factory = module->getFactory();
    for (auto& classInfo : factory.classInfos())
	{
		if (classInfo.category() == kVstAudioEffectClass)
		{
			if (effectID)
			{
				if (*effectID != classInfo.ID())
					continue;
			}
            plugProvider = owned(new Vst::PlugProvider(factory, classInfo, true));
			break;
		}
	}

	if (!plugProvider)
	{
		if (effectID) {
			SetErrorMessage( tr("no module with id %1").arg(QString::fromStdString(effectID->toString())) );
		}
		else {
			SetErrorMessage(tr("no audio module"));
		}
		return true;
	}

    component = plugProvider->getComponent();
    if (!component)
	{
		SetErrorMessage( tr("plugin not created") );
		return true;
	}

    editController = plugProvider->getController();
    if (!editController)
    {
        SetErrorMessage("No EditController found");
        return true;
    }
    //editController->release(); // plugProvider does an addRef // do I need that ?

    initProcessData();

    if(!initController())
        return true;
    if(!initProcessor())
        return true;

   
    //midi in
    qint32 cpt=0;
    qint32 numBusEIn = component->getBusCount(Vst::kEvent, Vst::kInput);
    for (qint32 i = 0; i < numBusEIn; i++) {
        Vst::BusInfo busInfo = {0};
        if(component->getBusInfo(Vst::kEvent, Vst::kInput, i, busInfo) == kResultTrue) {
            for(qint32 j=0; j<busInfo.channelCount; j++) {
                Pin *p = listMidiPinIn->AddPin(cpt++);
                p->setObjectName( QString::fromUtf16((char16_t*)busInfo.name) );
            }
        }
    }

    //midi out
    cpt=0;
    qint32 numBusEOut = component->getBusCount(Vst::kEvent, Vst::kOutput);
    for (qint32 i = 0; i < numBusEOut; i++) {
        Vst::BusInfo busInfo = {0};
        if(component->getBusInfo(Vst::kEvent, Vst::kOutput, i, busInfo) == kResultTrue) {
            for(qint32 j=0; j<busInfo.channelCount; j++) {
                Pin *p = listMidiPinOut->AddPin(cpt++);
                p->setObjectName( QString::fromUtf16((char16_t*)busInfo.name) );
            }
        }
    }


	//connect processor with controller
    component->queryInterface(Vst::IConnectionPoint::iid, (void**)&iConnectionPointComponent);
	editController->queryInterface(Vst::IConnectionPoint::iid, (void**)&iConnectionPointController);
	if (iConnectionPointComponent && iConnectionPointController) {
		iConnectionPointComponent->connect(iConnectionPointController);
		iConnectionPointController->connect(iConnectionPointComponent);
	}

	//synchronize controller
	//shouldn't we do that regularly ?
	MemoryStream stream;
	if (component->getState(&stream)) {
		stream.seek(0, IBStream::kIBSeekSet, 0);
		editController->setComponentState(&stream);
	}

	

    /*
	if (component->initialize(myHost->vst3Host) != kResultOk) {
		SetErrorMessage( tr("plugin not initialized") );
		return true;
	}
	*/

    closed=false;
	//SetSleep(false);
    return true;
}

void Vst3Plugin::initProcessData() {
    processData.inputEvents = &inEvents;
    processData.inputParameterChanges = &inputParameterChanges;
    processData.processContext = &myHost->vst3Host->processContext;
	processData.symbolicSampleSize = doublePrecision ? Vst::kSample64 : Vst::kSample32;
    processData.numSamples = myHost->GetBufferSize();
}

bool Vst3Plugin::initProcessor()
{
    Vst::ProcessSetup setup {
        Vst::kRealtime,
        doublePrecision?Vst::kSample64:Vst::kSample32,
        static_cast<int32>(myHost->GetBufferSize()),
        myHost->GetSampleRate()
    };
    FUnknownPtr<Vst::IAudioProcessor> processor = component;
    
	//difference between that buffer sample and ProcessSetup.numSample ?
	processData.prepare(*component, myHost->GetBufferSize(), doublePrecision ? Vst::kSample64 : Vst::kSample32);
    if(processor->setupProcessing(setup)!=kResultOk) {
        LOG("no double precision ?");
        if(doublePrecision) {
            doublePrecision=false;
            setup.symbolicSampleSize = Vst::kSample32;
            if(processor->setupProcessing(setup)!=kResultOk) {
                LOG("nope, it's something else, abort");
                return false;
            }
        }
    }

    initAudioBuffers(Vst::kInput);
    initAudioBuffers(Vst::kOutput);
	


    return true;
}

bool Vst3Plugin::initAudioBuffers(Vst::BusDirection dir, bool unassign)
{
//	if (!unassign) {
//		if (dir == Vst::kInput) {
//			listAudioPinIn->SetNbPins(0);
//		}
//		else {
//			listAudioPinOut->SetNbPins(0);
//		}
//	}

    qint32 cpt=0;
	qint32 numBusIn = component->getBusCount(Vst::kAudio, dir);
    for (qint32 i = 0; i < numBusIn; i++) {
        Vst::BusInfo busInfo = {0};
        if(component->getBusInfo(Vst::kAudio, dir, i, busInfo) == kResultTrue) {
			for(qint32 j=0; j<busInfo.channelCount; j++) {
                Pin *p = 0;
                if(dir==Vst::kInput) {
                    p = listAudioPinIn->GetPin(cpt,true);
//                    p = listAudioPinIn->AddPin(cpt++);
                } else {
                    p = listAudioPinOut->GetPin(cpt,true);
//                    p = listAudioPinOut->AddPin(cpt++);
                }

                p->setObjectName( QString::fromUtf16((char16_t*)busInfo.name) );
                AudioBuffer* buff = static_cast<AudioPin*>(p)->GetBuffer();
                
				if(doublePrecision) {
                    processData.setChannelBuffer64(dir, i, j, unassign ? nullptr : (double*)buff->GetPointer());
                } else {
                    processData.setChannelBuffer(dir, i, j, unassign ? nullptr : (float*)buff->GetPointer());
                }

                cpt++;
            }
        }
    }

    return true;
}

bool Vst3Plugin::initController()
{

    // set the host handler
    if(editController->setComponentHandler(this) != kResultOk) {
        SetErrorMessage( tr("can't set comp. handler") );
        return true;
    }

    //bypassParameter = FixedPinNumber::bypass;
    progChangeParameter = -1;

	//init editor window
	if (myHost->settings->GetSetting("fastEditorsOpenClose", true).toBool()) {
		hasEditor = CreateEditorWindow();
	}
	if (hasEditor) {
		listParameterPinIn->AddPin(FixedPinNumber::editorVisible);
		listParameterPinIn->AddPin(FixedPinNumber::learningMode);
	}

	listParameterPinIn->AddPin(FixedPinNumber::bypass);

    //init parameters
    qint32 numParameters = editController->getParameterCount ();
    for (qint32 i = 0; i < numParameters; i++) {
        Vst::ParameterInfo paramInfo = {0};
        tresult result = editController->getParameterInfo (i, paramInfo);
        if (result == kResultOk) {
            if(paramInfo.flags & Vst::ParameterInfo::kIsBypass) {
                //the plugin's bypass pin
                //don't create it, we use our own
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

    FUnknownPtr<Vst::IMidiMapping> midiMapping = editController;
    if (midiMapping)
        midiCCMapping = initMidiCtrlerAssignment (component, midiMapping);



    return true;
}

void Vst3Plugin::SaveProgram()
{
    if(!currentProgram)// || !currentProgram->IsDirty())
        return;

    if(component) {
        MemoryStream stateCom;
        if(component->getState(&stateCom)!=kResultOk) {
            LOG("error saving comp state")
        } else {
    //		stateCom.seek(0, IBStream::kIBSeekSet, 0);
            QByteArray ba(stateCom.getData(), stateCom.getSize());
            currentProgram->listOtherValues.insert(0, ba);
        }
    }

    if(editController) {
        MemoryStream stateCtrl;
        if (editController->getState(&stateCtrl) != kResultOk) {
            LOG("error saving ctrl state")
        }
        else {
    //		stateCtrl.seek(0, IBStream::kIBSeekSet, 0);
            QByteArray ba(stateCtrl.getData(), stateCtrl.getSize());
            currentProgram->listOtherValues.insert(1, ba);
        }
    }

    Object::SaveProgram();
}

void Vst3Plugin::LoadProgram(int prog)
{

    if(prog==currentProgId && currentProgram)
        return;

    bool msgWasEnabled=MsgEnabled();
    SetMsgEnabled(false);

    Object::LoadProgram(prog);
	{
        QMutexLocker lock(&objMutex);
        try {
		if (component) {
			QByteArray compstate = currentProgram->listOtherValues.value(0, QByteArray()).toByteArray();
			if (!compstate.isEmpty()) {
				MemoryStream state;
				int32 s = 0;
				state.write(compstate.data(), compstate.size(), &s);
				//state.seek(0, IBStream::kIBSeekSet, 0);
				if (component->setState(&state) != kResultOk) {
					LOG("err loading component state");
				}
			}
		}

		if (editController) {
			QByteArray ctrlstate = currentProgram->listOtherValues.value(1, QByteArray()).toByteArray();
			if (!ctrlstate.isEmpty()) {
				MemoryStream state;
				int32 s = 0;
				state.write(ctrlstate.data(), ctrlstate.size(), &s);
				//state.seek(0, IBStream::kIBSeekSet, 0);
				if (editController->setState(&state) != kResultOk) {
					LOG("err loading controller state");
				}
			}
		}
        }
        catch(std::exception& e) {
            LOG(e.what())
        } catch(...) {
            LOG("eeexption")
        }
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

    pView = editController->createView(Vst::ViewType::kEditor);
    if(!pView) {
        LOG("view not created")
        return false;
    }
    //uint32 r1 = pView->addRef();
	
	

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

    if(pView->setFrame(editorWnd) !=kResultOk) {
        LOG("frame not set")
    }

	HWND w = (HWND)editorWnd->GetWinId();
    if(pView->attached(w, kPlatformTypeHWND) !=kResultOk) {
		LOG("editor not attached");
		//will retry :
		editorWnd->deleteLater();
		editorWnd = 0;
		return true;
    }

    ViewRect size;
    if(pView->getSize(&size)!=kResultOk) {
		LOG("can't get size");
    }

    editorWnd->SetWindowSize(size.getWidth(),size.getHeight());

    if(pView->onSize(&size)!=kResultOk) {
		LOG("can't set size");
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

    if(processData.numSamples == static_cast<int32>(size))
        return;
    if (size == 0)
        return;

    Object::SetBufferSize(size);

    initProcessData();
    initProcessor();


//    SetSleep(true);
//    SetSleep(false);
}

void Vst3Plugin::SetSampleRate(float rate)
{
    SetSleep(true);
    SetSleep(false);
}

void Vst3Plugin::SetSleep(bool sleeping)
{
    if(closed)
        return;
    if(sleeping == GetSleep())
        return;

    FUnknownPtr<Vst::IAudioProcessor> processor = component;
    if (!processor)
        return;

    {
        QMutexLocker l(&objMutex);


        if(sleeping) {
            processor->setProcessing(false);
            component->setActive(false);

        } else {
            foreach(Pin *in, listAudioPinIn->listPins ) {
                AudioPin *audioIn = static_cast<AudioPin*>(in);
                audioIn->GetBuffer()->ResetStackCounter();
            }

            Vst::ProcessSetup setup {
                Vst::kRealtime,
                doublePrecision?Vst::kSample64:Vst::kSample32,
                static_cast<int32>(myHost->GetBufferSize()),
                myHost->GetSampleRate()
            };
            if (processor->setupProcessing (setup) == kResultOk) {
                component->setActive(true);
                processor->setProcessing(true);
            }
        }
    }
	LOG("vst3 setsleep " << sleeping)
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
	
	//TODO:probably need a better cleanup 
	//processData;
	initAudioBuffers(Vst::kInput,true);
	initAudioBuffers(Vst::kOutput,true);

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
        //editorWnd=0;
    }

	plugProvider.reset();
	module.reset();

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
	/*
    if(audioProcessor) {
        audioProcessor->release();
    }

    if(plugInstance) {
        plugInstance->setActive(false);
        plugInstance->terminate();
        plugInstance->release();
        plugInstance=0;
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
	*/
	/*
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
	*/


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

	{
		QMutexLocker lock(&objMutex);

		foreach(Pin *pin, listAudioPinIn->listPins) {
			static_cast<AudioPin*>(pin)->GetBuffer()->ConsumeStack();
		}
		foreach(Pin *pin, listAudioPinOut->listPins) {
			static_cast<AudioPin*>(pin)->GetBuffer()->GetPointerWillBeFilled();
		}


		//  input buffers
		qint32 cpt = 0;
		qint32 numBusIn = component->getBusCount(Vst::kAudio, Vst::kInput);
		//processData.numInputs = numBusIn;
		for (qint32 busIndex = 0; busIndex < numBusIn; busIndex++) {
			Vst::BusInfo busInfo = { 0 };
			if (component->getBusInfo(Vst::kAudio, Vst::kInput, busIndex, busInfo) == kResultTrue) {
				for (qint32 channelIndex = 0; channelIndex < busInfo.channelCount; channelIndex++) {
					Pin *pin = listAudioPinIn->GetPin(cpt++);
					if (pin) {
						Vst::AudioBusBuffers* bus = &processData.inputs[busIndex];
						if (bus) {
                            if(doublePrecision) {
                                bus->channelBuffers64[channelIndex] = (double*)static_cast<AudioPin*>(pin)->GetBuffer()->ConsumeStack();
                            } else {
                                bus->channelBuffers32[channelIndex] = (float*)static_cast<AudioPin*>(pin)->GetBuffer()->ConsumeStack();
                            }
						}
					}
					else {
						//	return;
					}
				}
			}
		}


		//    output buffers
		cpt = 0;
		qint32 numBusOut = component->getBusCount(Vst::kAudio, Vst::kOutput);
		//processData.numOutputs = numBusOut;
		for (qint32 busIndex = 0; busIndex < numBusOut; busIndex++) {
			Vst::BusInfo busInfo = { 0 };
			if (component->getBusInfo(Vst::kAudio, Vst::kOutput, busIndex, busInfo) == kResultTrue) {
				for (qint32 channelIndex = 0; channelIndex < busInfo.channelCount; channelIndex++) {
					Pin *pin = listAudioPinOut->GetPin(cpt++);
					if (pin) {
						Vst::AudioBusBuffers* bus = &processData.outputs[busIndex];
						if (bus) {
                            if(doublePrecision) {
                                bus->channelBuffers64[channelIndex] = (double*)static_cast<AudioPin*>(pin)->GetBuffer()->GetPointerWillBeFilled();
                            } else {
                                bus->channelBuffers32[channelIndex] = (float*)static_cast<AudioPin*>(pin)->GetBuffer()->GetPointerWillBeFilled();
                            }
						}
					}
					else {
						//	return;
					}
				}
			}
		}

        try
		{
			QMutexLocker l(&paramLock);

			FUnknownPtr<Vst::IAudioProcessor> processor = component;
			tresult result = processor->process(processData);
			if (result != kResultOk) {
				LOG("error while processing")
			}

			inEvents.clear();
			inputParameterChanges.clearQueue();
        } catch(...) {
            LOG("ex")
        }

		//output params
		if (processData.outputParameterChanges) {
			Vst::ParamValue value;
			int32 sampleOffset;
			for (int32 i = 0; i < processData.outputParameterChanges->getParameterCount(); i++) {
				Vst::IParamValueQueue* queue = processData.outputParameterChanges->getParameterData(i);
				if (queue) {
					if (queue->getPoint(queue->getPointCount() - 1, sampleOffset, value) == kResultTrue) {
						performEdit(queue->getParameterId(), value);
					}
				}
			}
		}

		//send result
		//=========================
		foreach(Pin* pin, listAudioPinOut->listPins) {
			static_cast<AudioPin*>(pin)->GetBuffer()->ConsumeStack();
			static_cast<AudioPin*>(pin)->SendAudioBuffer();
		}
	}
}

//from miditovst
OptionalEvent Vst3Plugin::midiToEvent (uint8_t status, uint8_t channel, uint8_t midiData0, uint8_t midiData1)
{
    Event new_event = {};
    if (status == MidiConst::noteOn || status == MidiConst::noteOff)
    {
        if (status == MidiConst::noteOff) // note off
        {
            new_event.noteOff.noteId = -1;
            new_event.type = Event::kNoteOffEvent;
            new_event.noteOff.channel = channel;
            new_event.noteOff.pitch = midiData0;
            new_event.noteOff.velocity = midiData1 / 127.f;
            return std::move (new_event);
        }
        else if (status == MidiConst::noteOn) // note on
        {
            new_event.noteOn.noteId = -1;
            new_event.type = Event::kNoteOnEvent;
            new_event.noteOn.channel = channel;
            new_event.noteOn.pitch = midiData0;
            new_event.noteOn.velocity = midiData1 / 127.f;
            return std::move (new_event);
        }
    }
    //--- -----------------------------
    else if (status == MidiConst::aftertouch)
    {
        new_event.type = Event::kPolyPressureEvent;
        new_event.polyPressure.channel = channel;
        new_event.polyPressure.pitch = midiData0;
        new_event.polyPressure.pressure = midiData1 / 127.f;
        return std::move (new_event);
    }

    return {};
}

//from miditovst
OptionParamChange Vst3Plugin::midiToParameter (uint8_t status, uint8_t channel, uint8_t midiData1, uint8_t midiData2, const ToParameterIdFunc& toParamID)
{
    if (!toParamID)
        return {};

    ParameterChange paramChange;
    if (status == MidiConst::ctrl) // controller
    {
        paramChange.first = toParamID (channel, midiData1);
        if (paramChange.first != kNoParamId)
        {
            paramChange.second = (double)midiData2 * kMidiScaler;
            return std::move (paramChange);
        }
    }
    else if (status == MidiConst::pitchbend)
    {
        paramChange.first = toParamID (channel, kPitchBend);
        if (paramChange.first != kNoParamId)
        {
            const double kPitchWheelScaler = 1. / (double)0x3FFF;

            const int32 ctrl = (midiData1 & kDataMask) | (midiData2 & kDataMask) << 7;
            paramChange.second = kPitchWheelScaler * (double)ctrl;
            return std::move (paramChange);
        };
    }
    else if (status == MidiConst::aftertouch)
    {
        paramChange.first = toParamID (channel, kAfterTouch);
        if (paramChange.first != kNoParamId)
        {
            paramChange.second = (ParamValue) (midiData1 & kDataMask) * kMidiScaler;
            return std::move (paramChange);
        };
    }
    else if (status == MidiConst::prog)
    {
        // TODO
    }

    return {};
}

bool Vst3Plugin::isPortInRange (int32 port, int32 channel) const
{
    return port < kMaxMidiMappingBusses && !midiCCMapping[port][channel].empty ();
}

// From Vst2Wrapper
MidiCCMapping Vst3Plugin::initMidiCtrlerAssignment (IComponent* component, IMidiMapping* midiMapping)
{
    MidiCCMapping midiCCMapping {};

    if (!midiMapping || !component)
        return midiCCMapping;

    int32 busses = std::min<int32> (component->getBusCount (kEvent, kInput), kMaxMidiMappingBusses);

    if (midiCCMapping[0][0].empty ())
    {
        for (int32 b = 0; b < busses; b++)
            for (int32 i = 0; i < kMaxMidiChannels; i++)
                midiCCMapping[b][i].resize (kCountCtrlNumber);
    }

    ParamID paramID;
    for (int32 b = 0; b < busses; b++)
    {
        for (int16 ch = 0; ch < kMaxMidiChannels; ch++)
        {
            for (int32 i = 0; i < kCountCtrlNumber; i++)
            {
                paramID = kNoParamId;
                if (midiMapping->getMidiControllerAssignment (b, ch, (CtrlNumber)i, paramID) == kResultTrue)
                {
                    // TODO check if tag is associated to a parameter
                    midiCCMapping[b][ch][i] = paramID;
                }
                else
                    midiCCMapping[b][ch][i] = kNoParamId;
            }
        }
    }
    return midiCCMapping;
}

bool Vst3Plugin::processParamChange (uint8_t status, uint8_t channel, uint8_t midiData1, uint8_t midiData2, int32 port)
{
    auto paramMapping = [port, this] (int32 channel, uint8_t data1) -> ParamID {
        if (!isPortInRange (port, channel))
            return kNoParamId;

        return midiCCMapping[port][channel][data1];
    };

    auto paramChange =
        midiToParameter (status, channel, midiData1, midiData2, paramMapping);
    if (paramChange)
    {
        int32 index = 0;
        IParamValueQueue* queue = inputParameterChanges.addParameterData ((*paramChange).first, index);
        if (queue)
        {
            const int32 sampleOffset = 0;
            if (queue->addPoint (sampleOffset, (*paramChange).second, index) != kResultOk)
            {
                assert (false && "Parameter point was not added to ParamValueQueue!");
            }
        }

        return true;
    }

    return false;
}

void Vst3Plugin::MidiMsgFromInput(long msg)
{
    if(closed)
        return;

    uint8 command = MidiStatus(msg) & MidiConst::codeMask;
    int8 chan = MidiStatus(msg) & MidiConst::channelMask;

//    Vst::Event event = {0};
    auto event = midiToEvent (command, chan, MidiData1(msg), MidiData2(msg));
    if (event) {
        event->flags = Vst::Event::kIsLive;
        QMutexLocker l(&paramLock);
        inEvents.addEvent(*event);

//        vstEvent->busIndex = port;
//            if (eventList.addEvent (*vstEvent) != kResultOk)
        return;
    }
    int32 port=0;
    processParamChange (command, chan, MidiData1(msg), MidiData2(msg), port);
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

		Vst::ParamID pId = -1;

        //send value to the gui
		if (pinInfo.pinNumber < FIXED_PIN_STARTINDEX) {
			Vst::ParameterInfo paramInfo;
			tresult result = editController->getParameterInfo(pinInfo.pinNumber, paramInfo);
			if (result == kResultOk) {
				pId = paramInfo.id;
				editController->setParamNormalized(pId, value);
			}
			
		}

        //send value to the processor
       // {
          //  QMutexLocker l(&paramLock);

            //send our own bypass
            if(pinInfo.pinNumber == FixedPinNumber::bypass) {
                pId = bypassParameter;
                value = bypass?1.0f:.0f;
            }

            //don't know what those indexes are yet
            int32 pindex = 0;
            Vst::IParamValueQueue* queue = inputParameterChanges.addParameterData(pId, pindex);
            int32 qindex = 0;
            const int32 sampleOffset = 0;
            queue->addPoint(sampleOffset, value, qindex);


//            if(pinInfo.pinNumber < FIXED_PIN_STARTINDEX) {
//                listParamChanged.insert(pId,value);
//            }
//            if(pinInfo.pinNumber == FixedPinNumber::bypass) {
//                listParamChanged.insert(bypassParameter,bypass?1.0f:.0f);
//            }



       // }

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

tresult PLUGIN_API Vst3Plugin::createInstance (TUID cid, TUID _iid, void** obj)
{
    return kResultOk;
}
//tresult PLUGIN_API Vst3Plugin::notifyUnitSelection (UnitID unitId)
//{

//}
//tresult PLUGIN_API notifyProgramListChange (Vst::ProgramListID listId, int32 programIndex)
//{

//}
tresult PLUGIN_API Vst3Plugin::queryInterface (const TUID iid, void** obj)
{
    QUERY_INTERFACE (iid, obj, Vst::IComponentHandler::iid, Vst::IComponentHandler)
    QUERY_INTERFACE (iid, obj, Vst::IComponentHandler2::iid, Vst::IComponentHandler2)

    //context menus
    //QUERY_INTERFACE (iid, obj, Vst::IComponentHandler3::iid, Vst::IComponentHandler3)

    //bus activate
    //QUERY_INTERFACE (iid, obj, Vst::IComponentHandlerBusActivation::iid, Vst::IComponentHandlerBusActivation)
	char8 cidString[50];
	FUID fuid = FUID::fromTUID(iid);
	fuid.toRegistryString(cidString);
	QString cidStr(cidString);
	LOG(cidStr)

    return kNoInterface;
}


//what are those ? pointer ref counter probbly...
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
	int pinNumber = -1;
//	Vst::ParameterInfo paramInfo;
//	if (editController->para getParameter InfoByTag(id, paramInfo) == kResultOk) {

    Vst::String128 str;
    editController->getParamStringByValue(id,0,str);
	
    qint32 numParameters = editController->getParameterCount ();
    for (qint32 i = 0; i < numParameters; i++) {
        Vst::ParameterInfo paramInfo = {0};
        tresult result = editController->getParameterInfo (i, paramInfo);
        if (result == kResultOk) {
//LOG(QString("%1 %2").arg(paramInfo.id).arg(QString::fromStdWString( paramInfo.title)));
if(paramInfo.id == id) {
    pinNumber = i;
}

        }
    }


	//Vst::Parameter* parameter = editController->getParameterObject(id);
	//if (parameter) {
	//	Vst::ParameterInfo paramInfo = parameter->getInfo();
//		pinNumber = paramInfo.id;
//	} else {
//		return kResultFalse;
//	}
		
    if(!listParameterPinIn->GetPin(pinNumber)) {
        listParameterPinIn->AddPin(pinNumber);
    }
    pin = static_cast<ParameterPin*>(listParameterPinIn->GetPin(pinNumber));


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
tresult PLUGIN_API Vst3Plugin::restartComponent (int32 flags)
{
	if (flags & Vst::kReloadComponent) {
		//reload plugin
		return kNotImplemented;
	}

	if (flags & Vst::kIoChanged) {
		return kNotImplemented;
	}
	if (flags & Vst::kParamValuesChanged) {
		return kNotImplemented;
	}
	if (flags & Vst::kLatencyChanged) {
		return kNotImplemented;
	}
	if (flags & Vst::kParamTitlesChanged) {
		return kNotImplemented;
	}
	if (flags & Vst::kMidiCCAssignmentChanged) {
		return kNotImplemented;
	}
	if (flags & Vst::kNoteExpressionChanged) {
		return kNotImplemented;
	}
	if (flags & Vst::kIoTitlesChanged) {
		return kNotImplemented;
	}
	if (flags & Vst::kPrefetchableSupportChanged) {
		return kNotImplemented;
	}
	if (flags & Vst::kRoutingInfoChanged) {
		return kNotImplemented;
	}

    return kResultOk;
}
tresult PLUGIN_API Vst3Plugin::setDirty(TBool /*state*/)
{
    OnProgramDirty();
    return kResultOk;
}
tresult PLUGIN_API Vst3Plugin::requestOpenEditor (FIDString /*name*/)
{
    ToggleEditor(true);
    return kResultOk;
}
tresult PLUGIN_API Vst3Plugin::startGroupEdit ()
{
    return kNotImplemented;
}

tresult PLUGIN_API Vst3Plugin::finishGroupEdit ()
{
    return kNotImplemented;
}
Vst::IContextMenu* PLUGIN_API Vst3Plugin::createContextMenu (IPlugView* plugView, const Vst::ParamID* paramID)
{
//    View::Vst3ContextMenu* menu = new View::Vst3ContextMenu(myHost->mainWindow);
//    return menu;
    return 0;
}
tresult PLUGIN_API Vst3Plugin::executeMenuItem (int32 tag)
{
    return kNotImplemented;
}

tresult PLUGIN_API Vst3Plugin::requestBusActivation (Vst::MediaType type, Vst::BusDirection dir, int32 index, TBool state)
{
    return kNotImplemented;
}

void Vst3Plugin::fromJson(QJsonObject &json)
{
    Object::fromJson(json);

//    savedState = json["processorState"].toString().toUtf8();
//    if(component && savedState.size()!=0) {
//        MemoryStream state;
//		int32 s;
//        state.write(savedState.data(), savedState.size(),&s);
//		LOG(QString("%1 statesize %2").arg(objInfo.name).arg(s));
//        state.seek(0,IBStream::kIBSeekSet,0);
////TODO
//		component->setState(&state);
//    }
}

void Vst3Plugin::toJson(QJsonObject &json) const
{
    Object::toJson(json);

    //MemoryStream state;
    //if(component && component->getState(&state)==kResultOk) {
		//TSize s = state.getSize();
		//LOG(QString("%1 statesize %2").arg(objInfo.name).arg(s));
		//QByteArray ba(state.getData(), state.getSize());
		//LOG(QString::fromStdString(ba.toStdString()));
		//json["processorState"] = QString::fromStdString(ba.toStdString());
		
		/*
		state.seek(0,IBStream::kIBSeekSet,0);
        int32 len=-1;
        char *buf=new char[1024];
        QByteArray bArray;
		
        while(len!=0) {
            if(state.read(buf,1024,&len)!=kResultOk)
                len=0;
            bArray.append(buf,len);
        }
        json["processorState"] = QString(bArray);
		*/
    //} else {
    //    LOG("error saving state");
   //     json["processorState"] = QString(savedState);
    //}
}

QDataStream & Vst3Plugin::toStream(QDataStream & out) const
{
    Object::toStream(out);

    if(!component) {
        out << savedState;
        return out;
    }

    MemoryStream state;
    if(component->getState(&state)!=kResultOk) {
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

    if(component && savedState.size()!=0) {
        MemoryStream state;
        state.write(savedState.data(),savedState.size(),0);
        state.seek(0,IBStream::kIBSeekSet,0);
        component->setState(&state);
    }
    return true;
}


#endif
