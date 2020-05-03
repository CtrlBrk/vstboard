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
#include "mainhost.h"
#include "mainwindow.h"
//#include "models/programsmodel.h"
#include "connectables/container.h"

#ifdef VST24SDK
    #include "connectables/vstplugin.h"
    int MainHost::vstUsersCounter=0;
#endif

#include "projectfile/fileversion.h"
#include "projectfile/projectfile.h"
#include "projectfile/jsonwriter.h"
#include "projectfile/jsonreader.h"
#include "views/configdialog.h"
#include "msgobject.h"


quint32 MainHost::currentFileVersion=PROJECT_AND_SETUP_FILE_VERSION;

MainHost::MainHost(Settings *settings, QObject *parent) :
    QObject(parent)
    ,updateViewTimer(0)
    ,programManager(0)
    ,objFactory(0)
    ,mainWindow(0)
#ifdef VST24SDK
        , vstHost(0)
#endif
#ifdef VSTSDK
        , vst3Host(0)
#endif
    ,settings(settings)
    ,solverNeedAnUpdate(false)
    ,solverUpdateEnabled(true)
    ,undoProgramChangesEnabled(false)
    ,globalDelay(0L)
    ,nbThreads(1)
{
    qRegisterMetaType<ConnectionInfo>("ConnectionInfo");
    qRegisterMetaType<ObjectInfo>("ObjectInfo");
    qRegisterMetaType<ObjectContainerAttribs>("ObjectContainerAttribs");
    qRegisterMetaType<MsgObject>("MsgObject");
    qRegisterMetaType<int>("ObjType::Enum");
    qRegisterMetaType<QVariant>("QVariant");
    qRegisterMetaType<AudioBuffer*>("AudioBuffer*");
    qRegisterMetaType<QVector<int> >("QVector<int>");
    qRegisterMetaType<QVector<float> >("QVector<float>");
    qRegisterMetaTypeStreamOperators<ObjectInfo>("ObjectInfo");
    qRegisterMetaTypeStreamOperators<ObjectContainerAttribs>("ObjectContainerAttribs");
}

MainHost::~MainHost()
{
    Close();
}

void MainHost::Close()
{
    static bool closed=false;
    if(closed)
        return;
    closed=true;

    updateRendererViewTimer.stop();

    EnableSolverUpdate(false);

    if(updateViewTimer) {
        updateViewTimer->stop();
		//is a child of mainhost : autodelete
        //updateViewTimer->deleteLater();
        //updateViewTimer=0;
    }

	//is a child of mainhost : autodelete
    //if(renderer) {
    //    delete renderer;
    //    renderer=0;
    //}

    hostContainer.clear();
    projectContainer.clear();
    groupContainer.clear();
    programContainer.clear();
    mainContainer.clear();

	//is a child of mainhost : autodelete
    //if(objFactory) {
    //    delete objFactory;
    //    objFactory=0;
    //}

    if(solver) {
        delete solver;
        solver=0;
    }

#ifdef VST24SDK
    vstUsersCounter--;
    if(vstUsersCounter==0)
        delete vstHost;
#endif
#ifdef VSTSDK
    //if(vst3Host)
    //    delete vst3Host;
#endif

	//is a child of mainhost : autodelete
    //if(programManager) {
        //delete programManager;
        //programManager=0;
    //}
}

void MainHost::Kill()
{
    delete this;
}

void MainHost::Init()
{
    programManager =new ProgramManager(this);
    doublePrecision=settings->GetSetting("doublePrecision",false).toBool();
	settings->SetSetting("currentDoublePrecision", doublePrecision);
    setObjectName("MainHost");

#ifdef SCRIPTENGINE
    QScriptValue scriptObj = scriptEngine.newQObject(this);
    scriptEngine.globalObject().setProperty("MainHost", scriptObj);
//    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
#endif

#ifdef VST24SDK
    if(!vst::CVSTHost::Get()) {
        vstHost = new vst::CVSTHost();
    } else {
        vstHost = vst::CVSTHost::Get();
    }
    vstUsersCounter++;
#endif
#ifdef VSTSDK
    vst3Host = new Vst3Host(this);
    connect(this,SIGNAL(SampleRateChanged(float)),
            vst3Host,SLOT(SetSampleRate(float)));
#endif

    solver = new Solver();

    sampleRate = 44100.0;
    bufferSize = 100;

    currentTempo=120;
    currentTimeSig1=4;
    currentTimeSig2=4;

    ChangeNbThreads(-1);

    renderer = new Renderer2(this);
    connect(renderer, SIGNAL(Timeout()),
            this, SLOT(OnRenderTimeout()));

    //timer
    timeFromStart.start();

    updateViewTimer = new QTimer(this);
//    updateViewTimer->start(120);
    updateViewTimer->start(50);

    connect(this,SIGNAL(SolverToUpdate()),
            this,SLOT(UpdateSolver()),
            Qt::QueuedConnection);

    updateRendererViewTimer.start(5000);
    connect(&updateRendererViewTimer, SIGNAL(timeout()),
            this,SLOT(UpdateRendererView()));
}

//we must create a basic setup before the app loop starts (any dialog can start the app loop ?)
void MainHost::Open()
{
    EnableSolverUpdate(false);
    SetupMainContainer();
    ClearSetup();
    ClearProject();
    EnableSolverUpdate(true);
}

void MainHost::SetupMainContainer()
{
    ObjectInfo info;
    info.nodeType = NodeType::container;
    info.objType = ObjType::Container;
    info.name = "mainContainer";
    info.forcedObjId = FixedObjId::mainContainer;

    mainContainer = objFactory->NewObject(info).staticCast< Connectables::Container >();
    if(!mainContainer)
        return;

    mainContainer->SetLoadingMode(true);
    mainContainer->LoadProgram(0);
    mainContainer->listenProgramChanges=false;

    mainContainer->SetLoadingMode(false);
    mainContainer->UpdateModificationTime();
    SetSolverUpdateNeeded();
}

void MainHost::SetupHostContainer()
{
    bool msgWasEnabled=false;

    if(hostContainer) {
        msgWasEnabled = hostContainer->MsgEnabled();
        mainContainer->ParkObject( hostContainer );
        hostContainer.clear();
        UpdateSolver(true);
        if(mainWindow)
            mainWindow->mySceneView->viewHost->ClearViewPrograms();
    }

    ObjectInfo info;
    info.nodeType = NodeType::container;
    info.objType = ObjType::Container;
    info.name = "hostContainer";
    info.forcedObjId = FixedObjId::hostContainer;

    hostContainer = objFactory->NewObject(info).staticCast<Connectables::Container>();
    if(!hostContainer)
        return;

    hostContainer->SetLoadingMode(true);
    mainContainer->AddObject(hostContainer);

    connect(this,SIGNAL(BufferSizeChanged(ulong)),
            hostContainer.get(),SLOT(SetBufferSize(ulong)));
    connect(this,SIGNAL(SampleRateChanged(float)),
            hostContainer.get(),SLOT(SetSampleRate(float)));

    QSharedPointer<Connectables::Object> bridge;


    //bridge in
    ObjectInfo in;
    in.name="hostBridgeIn";
    in.nodeType = NodeType::bridge;
    in.objType = ObjType::BridgeIn;
    in.forcedObjId = FixedObjId::hostContainerIn;

    bridge = objFactory->NewObject(in);
    hostContainer->AddObject( bridge );
    hostContainer->bridgeIn = bridge;

    //bridge out
    ObjectInfo out;
    out.name="hostBridgeOut";
    out.nodeType = NodeType::bridge;
    out.objType = ObjType::BridgeOut;
    out.forcedObjId = FixedObjId::hostContainerOut;

    bridge = objFactory->NewObject(out);
    hostContainer->AddObject( bridge );
    hostContainer->bridgeOut = bridge;

    //connect with groupContainer
    if(groupContainer) {
        mainContainer->ConnectObjects(groupContainer->bridgeSend, hostContainer->bridgeIn,true);
        mainContainer->ConnectObjects(hostContainer->bridgeOut, groupContainer->bridgeReturn,true);
    }

    //send bridge
    ObjectInfo send;
    send.name = "hostBridgeSend";
    send.nodeType = NodeType::bridge;
    send.objType = ObjType::BridgeSend;
    send.forcedObjId = FixedObjId::hostContainerSend;

    bridge = objFactory->NewObject(send);
    hostContainer->AddObject( bridge );
    hostContainer->bridgeSend = bridge;

    //return bridge
    ObjectInfo retrn;
    retrn.name = "hostBridgeReturn";
    retrn.nodeType = NodeType::bridge;
    retrn.objType = ObjType::BridgeReturn;
    retrn.forcedObjId = FixedObjId::hostContainerReturn;

    bridge = objFactory->NewObject(retrn);
    hostContainer->AddObject( bridge );
    hostContainer->bridgeReturn = bridge;

    //connect with projectContainer
    if(projectContainer) {
        mainContainer->ConnectObjects(hostContainer->bridgeSend, projectContainer->bridgeIn,true);
        mainContainer->ConnectObjects(projectContainer->bridgeOut, hostContainer->bridgeReturn,true);
    }
    hostContainer->listenProgramChanges=false;

    if(projectContainer) {
        hostContainer->childContainer=projectContainer;
        projectContainer->parentContainer=hostContainer;
    }

    hostContainer->SetLoadingMode(false);
    hostContainer->UpdateModificationTime();
    SetSolverUpdateNeeded();

    if(msgWasEnabled) {
        hostContainer->SetMsgEnabled(true);
        hostContainer->UpdateView();
    }

#ifdef SCRIPTENGINE
    QScriptValue scriptObj = scriptEngine.newQObject(hostContainer.get());
    scriptEngine.globalObject().setProperty("hostContainer", scriptObj);
//    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
#endif
}

void MainHost::SetupProjectContainer()
{
    bool msgWasEnabled=false;

    if(projectContainer) {
        msgWasEnabled=projectContainer->MsgEnabled();
        mainContainer->ParkObject( projectContainer );
        projectContainer.clear();
        UpdateSolver(true);
        if(mainWindow)
            mainWindow->mySceneView->viewProject->ClearViewPrograms();
    }

    timeFromStart.restart();

    ObjectInfo info;
    info.nodeType = NodeType::container;
    info.objType = ObjType::Container;
    info.name = "projectContainer";
    info.forcedObjId = FixedObjId::projectContainer;

    projectContainer = objFactory->NewObject(info).staticCast<Connectables::Container>();
    if(!projectContainer)
        return;

    projectContainer->SetLoadingMode(true);
    mainContainer->AddObject(projectContainer);

    connect(this,SIGNAL(BufferSizeChanged(ulong)),
            projectContainer.get(),SLOT(SetBufferSize(ulong)));
    connect(this,SIGNAL(SampleRateChanged(float)),
            projectContainer.get(),SLOT(SetSampleRate(float)));

    QSharedPointer<Connectables::Object> bridge;

    //bridge in
    ObjectInfo in;
    in.name="projectBridgeIn";
    in.nodeType = NodeType::bridge;
    in.objType = ObjType::BridgeIn;
    in.forcedObjId = FixedObjId::projectContainerIn;

    bridge = objFactory->NewObject(in);
    projectContainer->AddObject( bridge );
    projectContainer->bridgeIn = bridge;

    //bridge out
    ObjectInfo out;
    out.name="projectBridgeOut";
    out.nodeType = NodeType::bridge;
    out.objType = ObjType::BridgeOut;
    out.forcedObjId = FixedObjId::projectContainerOut;

    bridge = objFactory->NewObject(out);
    projectContainer->AddObject( bridge );
    projectContainer->bridgeOut = bridge;

    //connect with hostContainer
    if(hostContainer) {
        mainContainer->ConnectObjects(hostContainer->bridgeSend, projectContainer->bridgeIn,true);
        mainContainer->ConnectObjects(projectContainer->bridgeOut, hostContainer->bridgeReturn,true);
    }


    //bridge send
    ObjectInfo send;
    send.name="projectBridgeSend";
    send.nodeType = NodeType::bridge;
    send.objType = ObjType::BridgeSend;
    send.forcedObjId = FixedObjId::projectContainerSend;

    bridge = objFactory->NewObject(send);
    projectContainer->AddObject( bridge );
    projectContainer->bridgeSend = bridge;

    //bridge return
    ObjectInfo retrn;
    retrn.name="projectBridgeReturn";
    retrn.nodeType = NodeType::bridge;
    retrn.objType = ObjType::BridgeReturn;
    retrn.forcedObjId = FixedObjId::projectContainerReturn;

    bridge = objFactory->NewObject(retrn);
    projectContainer->AddObject( bridge );
    projectContainer->bridgeReturn = bridge;

    //connect with programContainer
    if(programContainer) {
        mainContainer->ConnectObjects(projectContainer->bridgeSend, programContainer->bridgeIn,true);
        mainContainer->ConnectObjects(programContainer->bridgeOut, projectContainer->bridgeReturn,true);
    }

    //connect with itself (pass-though cables)
    projectContainer->ConnectObjects(projectContainer->bridgeIn, projectContainer->bridgeSend,false);
    projectContainer->ConnectObjects(projectContainer->bridgeReturn, projectContainer->bridgeOut,false);

    projectContainer->listenProgramChanges=false;

    if(hostContainer) {
        hostContainer->childContainer=projectContainer;
        projectContainer->parentContainer=hostContainer;
    }
    if(groupContainer) {
        projectContainer->childContainer=groupContainer;
        groupContainer->parentContainer=projectContainer;
    }

    projectContainer->SetLoadingMode(false);
    projectContainer->UpdateModificationTime();
    SetSolverUpdateNeeded();

    if(msgWasEnabled) {
        projectContainer->SetMsgEnabled(true);
        projectContainer->UpdateView();
    }

#ifdef SCRIPTENGINE
    QScriptValue scriptObj = scriptEngine.newQObject(projectContainer.get());
    scriptEngine.globalObject().setProperty("projectContainer", scriptObj);
//    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
#endif
}

void MainHost::SetupProgramContainer()
{
    bool msgWasEnabled=false;

    if(programContainer) {
        msgWasEnabled=programContainer->MsgEnabled();

        mainContainer->ParkObject( programContainer );
        programContainer.clear();
        UpdateSolver(true);
        if(mainWindow)
            mainWindow->mySceneView->viewProgram->ClearViewPrograms();
    }

    ObjectInfo info;
    info.nodeType = NodeType::container;
    info.objType = ObjType::Container;
    info.name = "programContainer";
    info.forcedObjId = FixedObjId::programContainer;

    programContainer = objFactory->NewObject(info).staticCast<Connectables::Container>();
    if(!programContainer)
        return;

    programContainer->containersParkingId = FixedObjId::programParking;
    programContainer->SetLoadingMode(true);

    programContainer->SetOptimizerFlag(true);
    mainContainer->AddObject(programContainer);

    connect(this,SIGNAL(BufferSizeChanged(ulong)),
            programContainer.get(),SLOT(SetBufferSize(ulong)));
    connect(this,SIGNAL(SampleRateChanged(float)),
            programContainer.get(),SLOT(SetSampleRate(float)));

    QSharedPointer<Connectables::Object> bridge;

    //bridge in
    ObjectInfo in;
    in.name="programBridgeIn";
    in.nodeType = NodeType::bridge;
    in.objType = ObjType::BridgeIn;
    in.forcedObjId = FixedObjId::programContainerIn;

    bridge = objFactory->NewObject(in);
    programContainer->AddObject( bridge );
    programContainer->bridgeIn = bridge;

    //bridge out
    ObjectInfo out;
    out.name="programBridgeOut";
    out.nodeType = NodeType::bridge;
    out.objType = ObjType::BridgeOut;
    out.forcedObjId = FixedObjId::programContainerOut;

    bridge = objFactory->NewObject(out);
    programContainer->AddObject( bridge );
    programContainer->bridgeOut = bridge;

    //connect with projectContainer
    if(projectContainer) {
        mainContainer->ConnectObjects(projectContainer->bridgeSend, programContainer->bridgeIn,true);
        mainContainer->ConnectObjects(programContainer->bridgeOut, projectContainer->bridgeReturn,true);
    }


    //bridge send
    ObjectInfo send;
    send.name="programBridgeSend";
    send.nodeType = NodeType::bridge;
    send.objType = ObjType::BridgeSend;
    send.forcedObjId = FixedObjId::programContainerSend;

    bridge = objFactory->NewObject(send);
    programContainer->AddObject( bridge );
    programContainer->bridgeSend = bridge;

    //bridge return
    ObjectInfo retrn;
    retrn.name="programBridgeReturn";
    retrn.nodeType = NodeType::bridge;
    retrn.objType = ObjType::BridgeReturn;
    retrn.forcedObjId = FixedObjId::programContainerReturn;

    bridge = objFactory->NewObject(retrn);
    programContainer->AddObject( bridge );
    programContainer->bridgeReturn = bridge;

    //connect with groupContainer
    if(groupContainer) {
        mainContainer->ConnectObjects(programContainer->bridgeSend, groupContainer->bridgeIn,true);
        mainContainer->ConnectObjects(groupContainer->bridgeOut, programContainer->bridgeReturn,true);
    }

    connect(programManager, SIGNAL(ProgChanged(quint32)),
            programContainer.data(), SLOT(SetProgram(quint32)));
    connect(programManager, SIGNAL(ProgDelete(quint32)),
            programContainer.data(), SLOT(RemoveProgram(quint32)));
    connect(this,SIGNAL(Rendered()),
            programContainer.data(), SLOT(PostRender()));

    if(groupContainer) {
        groupContainer->childContainer=programContainer;
        programContainer->parentContainer=groupContainer;
    }

    programContainer->SetLoadingMode(false);
    programContainer->UpdateModificationTime();
    SetSolverUpdateNeeded();

    if(msgWasEnabled) {
        programContainer->SetMsgEnabled(true);
        programContainer->UpdateView();
    }

#ifdef SCRIPTENGINE
    QScriptValue scriptObj = scriptEngine.newQObject( programContainer.get() );
    scriptEngine.globalObject().setProperty("programContainer", scriptObj);
//    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
#endif
}

void MainHost::SetupGroupContainer()
{
    bool msgWasEnabled=false;

    if(groupContainer) {
        msgWasEnabled=groupContainer->MsgEnabled();

        mainContainer->ParkObject( groupContainer );
        groupContainer.clear();
        UpdateSolver(true);
        if(mainWindow)
            mainWindow->mySceneView->viewGroup->ClearViewPrograms();
    }

    ObjectInfo info;
    info.nodeType = NodeType::container;
    info.objType = ObjType::Container;
    info.name = "groupContainer";
    info.forcedObjId = FixedObjId::groupContainer;

    groupContainer = objFactory->NewObject(info).staticCast<Connectables::Container>();
    if(!groupContainer)
        return;

    groupContainer->containersParkingId = FixedObjId::groupParking;
    groupContainer->SetLoadingMode(true);

    mainContainer->AddObject(groupContainer);

    connect(this,SIGNAL(BufferSizeChanged(ulong)),
            groupContainer.get(),SLOT(SetBufferSize(ulong)));
    connect(this,SIGNAL(SampleRateChanged(float)),
            groupContainer.get(),SLOT(SetSampleRate(float)));

    QSharedPointer<Connectables::Object> bridge;

    //bridge in
    ObjectInfo in;
    in.name="groupBridgeIn";
    in.nodeType = NodeType::bridge;
    in.objType = ObjType::BridgeIn;
    in.forcedObjId = FixedObjId::groupContainerIn;

    bridge = objFactory->NewObject(in);
    groupContainer->AddObject( bridge );
    groupContainer->bridgeIn = bridge;

    //bridge out
    ObjectInfo out;
    out.name="groupBridgeOut";
    out.nodeType = NodeType::bridge;
    out.objType = ObjType::BridgeOut;
    out.forcedObjId = FixedObjId::groupContainerOut;

    bridge = objFactory->NewObject(out);
    groupContainer->AddObject( bridge );
    groupContainer->bridgeOut = bridge;

    //connect with programContainer
    if(programContainer) {
        mainContainer->ConnectObjects(programContainer->bridgeSend, groupContainer->bridgeIn,true);
        mainContainer->ConnectObjects(groupContainer->bridgeOut, programContainer->bridgeReturn,true);
    }

    //bridge send
    ObjectInfo send;
    send.name="groupBridgeSend";
    send.nodeType = NodeType::bridge;
    send.objType = ObjType::BridgeSend;
    send.forcedObjId = FixedObjId::groupContainerSend;

    bridge = objFactory->NewObject(send);
    groupContainer->AddObject( bridge );
    groupContainer->bridgeSend = bridge;

    //bridge return
    ObjectInfo retrn;
    retrn.name="groupBridgeReturn";
    retrn.nodeType = NodeType::bridge;
    retrn.objType = ObjType::BridgeReturn;
    retrn.forcedObjId = FixedObjId::groupContainerReturn;

    bridge = objFactory->NewObject(retrn);
    groupContainer->AddObject( bridge );
    groupContainer->bridgeReturn = bridge;

    //connect with hostContainer
    if(hostContainer) {
        mainContainer->ConnectObjects(groupContainer->bridgeSend, hostContainer->bridgeIn,true);
        mainContainer->ConnectObjects(hostContainer->bridgeOut, groupContainer->bridgeReturn,true);
    }

    connect(programManager, SIGNAL(GroupChanged(quint32)),
            groupContainer.data(), SLOT(SetProgram(quint32)));
    connect(programManager, SIGNAL(GroupDelete(quint32)),
            groupContainer.data(), SLOT(RemoveProgram(quint32)));
    connect(this,SIGNAL(Rendered()),
            groupContainer.data(), SLOT(PostRender()));

    if(projectContainer) {
        projectContainer->childContainer=groupContainer;
        groupContainer->parentContainer=projectContainer;
    }
    if(programContainer) {
        groupContainer->childContainer=programContainer;
        programContainer->parentContainer=groupContainer;
    }

    groupContainer->SetLoadingMode(false);
    groupContainer->UpdateModificationTime();
    SetSolverUpdateNeeded();

    if(msgWasEnabled) {
        groupContainer->SetMsgEnabled(true);
        groupContainer->UpdateView();
    }

#ifdef SCRIPTENGINE
    QScriptValue scriptObj = scriptEngine.newQObject(groupContainer.get());
    scriptEngine.globalObject().setProperty("groupContainer", scriptObj);
//    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
#endif
}

void MainHost::EnableSolverUpdate(bool enable)
{
    QMutexLocker locket(&solverMutex);
    solverUpdateEnabled = enable;
}

void MainHost::ResetDelays()
{
    if(mainContainer && mainContainer->GetCurrentProgram())
        mainContainer->GetCurrentProgram()->ResetDelays();
    if(hostContainer && hostContainer->GetCurrentProgram())
        hostContainer->GetCurrentProgram()->ResetDelays();
    if(projectContainer && projectContainer->GetCurrentProgram())
        projectContainer->GetCurrentProgram()->ResetDelays();
    if(groupContainer && groupContainer->GetCurrentProgram())
        groupContainer->GetCurrentProgram()->ResetDelays();
    if(programContainer && programContainer->GetCurrentProgram())
        programContainer->GetCurrentProgram()->ResetDelays();
}

void MainHost::UpdateSolver(bool forceUpdate)
{
    solverMutex.lock();

        //update not forced, not needed or disabled : return
        if( (!solverUpdateEnabled || !solverNeedAnUpdate) && !forceUpdate) {
            solverMutex.unlock();
            return;
        }

        //disable other solver updates
        bool solverWasEnabled=solverUpdateEnabled;
        solverUpdateEnabled = false;

        //allow others to ask for a new update while we're updating
        solverNeedAnUpdate = false;

    solverMutex.unlock();

    //update the solver
    hashCables lstCables;
    if(mainContainer && mainContainer->GetCurrentProgram())
        mainContainer->GetCurrentProgram()->AddToCableList(&lstCables);
    if(hostContainer && hostContainer->GetCurrentProgram())
        hostContainer->GetCurrentProgram()->AddToCableList(&lstCables);
    if(projectContainer && projectContainer->GetCurrentProgram())
        projectContainer->GetCurrentProgram()->AddToCableList(&lstCables);
    if(groupContainer && groupContainer->GetCurrentProgram())
        groupContainer->GetCurrentProgram()->AddToCableList(&lstCables);
    if(programContainer && programContainer->GetCurrentProgram())
        programContainer->GetCurrentProgram()->AddToCableList(&lstCables);

    RenderMap rMap;
    long newDelay = solver->GetMap(objFactory->GetListObjects(), lstCables, nbThreads, rMap);
    if(newDelay!=globalDelay) {
        globalDelay=newDelay;
        emit DelayChanged(globalDelay);
    }
    SetRenderMap(rMap);
    EnableSolverUpdate(solverWasEnabled);
}

void MainHost::UpdateRendererMap()
{
    solver->UpdateCpuTimes(renderer->currentMap, nbThreads);
    renderer->SetMap(renderer->currentMap,nbThreads);
}

void MainHost::UpdateRendererView()
{
//    MsgObject msg(FixedObjId::mainWindow);
    _MSGOBJ(msg,FixedObjId::mainWindow);
    renderer->currentMap.GetInfo(msg);
    SendMsg(msg);
}

void MainHost::ChangeNbThreads(int nbTh)
{
    if(nbTh<=0 || nbTh>MAX_NB_THREADS) {
        nbTh = settings->GetSetting("NbThreads",-1).toInt();
    }

    if(nbTh<=0 || nbTh>MAX_NB_THREADS) {
#ifdef _WIN32
        SYSTEM_INFO info;
        GetSystemInfo(&info);
        nbTh = info.dwNumberOfProcessors;
#else
        nbTh = 1;
#endif
    }

    if(nbThreads == nbTh)
        return;

    nbThreads = nbTh;
    SetSolverUpdateNeeded();
}

void MainHost::SetBufferSizeMs(unsigned int ms)
{
    ulong size = ms*sampleRate/1000;
    SetBufferSize(size);
}

void MainHost::SetBufferSize(unsigned long size)
{
    if(bufferSize == size)
        return;

//    MsgObject msg(FixedObjId::mainWindow);
    _MSGOBJ(msg,FixedObjId::mainWindow);
    msg.prop[MsgObject::ObjInfo]=QString("hostBudffer:%1").arg(size);
    SendMsg(msg);

    bufferSize = size;
    emit BufferSizeChanged(bufferSize);
}

void MainHost::SetSampleRate(float rate)
{
    if(sampleRate == rate)
        return;

    sampleRate = rate;
    emit SampleRateChanged(sampleRate);
}

void MainHost::Render()
{

//    LOG("Render");

#ifdef VSTSDK
    CheckTempo();
#endif

    if(mainContainer)
        mainContainer->NewRenderLoop();
    if(hostContainer)
        hostContainer->NewRenderLoop();
    if(projectContainer)
        projectContainer->NewRenderLoop();
    if(groupContainer)
        groupContainer->NewRenderLoop();
    if(programContainer)
        programContainer->NewRenderLoop();

    if(renderer)
        if(!renderer->StartRender()) {
            LOG("render error");
        }

    if(solverNeedAnUpdate && solverUpdateEnabled)
        emit SolverToUpdate();

    emit Rendered();
}

void MainHost::OnRenderTimeout() {

    SetSolverUpdateNeeded();
}

#ifdef VST24SDK
void MainHost::SetTimeInfo(const VstTimeInfo *info)
{
    vstHost->SetTimeInfo(info);
}
#endif

void MainHost::SetTempo(int tempo, int sign1, int sign2)
{
#ifdef VST24SDK
    vstHost->SetTempo(tempo,sign1,sign2);
#endif
#ifdef VSTSDK
    vst3Host->SetTempo(tempo,sign1,sign2);
#endif
}

void MainHost::CheckTempo()
{
#ifdef VSTSDK
    int tempo=0;
    int sign1=0;
    int sign2=0;

    vst3Host->GetTempo(tempo,sign1,sign2);
    if(tempo!=currentTempo
       || sign1!=currentTimeSig1
       || sign2!=currentTimeSig2) {

        currentTempo=tempo;
        currentTimeSig1=sign1;
        currentTimeSig2=sign2;
        emit TempoChanged(currentTempo,currentTimeSig1,currentTimeSig2);
    }
#endif
}

void MainHost::GetTempo(int &tempo, int &sign1, int &sign2)
{
// #ifdef VSTSDK
#ifdef VST24SDK
    vstHost->GetTempo(tempo,sign1,sign2);
#else
    tempo=120;
    sign1=4;
    sign2=4;
#endif
}

//bool MainHost::LoadFile(const QString &filename)
//{
//    QFileInfo info(filename);
//    if ( info.suffix()==SETUP_FILE_EXTENSION ) {
//        return LoadSetupFile(filename);
//    }
//    if ( info.suffix()==PROJECT_FILE_EXTENSION ) {
//        return LoadProjectFile(filename);
//    }
//}

bool MainHost::LoadSetupFile(const QString &filename)
{

    if(!programManager->userWantsToUnloadSetup())
        return false;

    QString name = filename;

    if(name.isEmpty()) {
        QString lastDir = settings->GetSetting("lastSetupDir").toString();
        name = QFileDialog::getOpenFileName(mainWindow, tr("Open a Setup file"), lastDir,
                                            tr("Supported formats (*.%1 *.%2 *.%3)")
                                                .arg(SETUP_JSON_BINARY_FILE_EXTENSION)
                                                .arg(SETUP_JSON_FILE_EXTENSION)
                                                .arg(SETUP_FILE_EXTENSION)
                                            + ";;" +
                                            tr("Binary Json (*.%1)").arg(SETUP_JSON_BINARY_FILE_EXTENSION)
                                            + ";;" +
                                            tr("Json (*.%1)").arg(SETUP_JSON_FILE_EXTENSION)
                                            + ";;" +
                                            tr("Setup Files (*.%1)").arg(SETUP_FILE_EXTENSION)
                                            );
    }

    if(name.isEmpty())
        return false;

    EnableSolverUpdate(false);
    undoStack.clear();

    bool no_error = true;

    QFile file(name);
    if (file.open(QFile::ReadOnly)) {
        //old format
        if(name.endsWith(SETUP_FILE_EXTENSION, Qt::CaseInsensitive)) {
            QDataStream in(&file);
            no_error = ProjectFile::FromStream(this,in);
        } else {
        //new format
            bool binary = true;
            if(!name.endsWith(SETUP_JSON_BINARY_FILE_EXTENSION, Qt::CaseInsensitive))
                binary = false;

            no_error = JsonReader::readProjectFile(&file,this,mainWindow,binary);
        }
    } else {
        no_error = false;
        QMessageBox::warning(mainWindow, tr("VstBoard"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(name),
                                  file.errorString()));
    }

    if(no_error) {
        currentSetupFile = name;
        objFactory->ResetAllSavedId();
    } else {
        ClearSetup();
    }

//    currentFileChanged();
    EnableSolverUpdate(true);

    return no_error;
}

bool MainHost::LoadProjectFile(const QString &filename)
{
    if(!programManager->userWantsToUnloadProject())
        return false;

    QString name = filename;

    if(name.isEmpty()) {
        QString lastDir = settings->GetSetting("lastProjectDir").toString();
        name = QFileDialog::getOpenFileName(mainWindow, tr("Open a Project file"), lastDir,
                                            tr("Supported formats (*.%1 *.%2 *.%3)")
                                                .arg(PROJECT_JSON_BINARY_FILE_EXTENSION)
                                                .arg(PROJECT_JSON_FILE_EXTENSION)
                                                .arg(SETUP_FILE_EXTENSION)
                                            + ";;" +
                                            tr("Binary Json (*.%1)").arg(PROJECT_JSON_BINARY_FILE_EXTENSION)
                                            + ";;" +
                                            tr("Json (*.%1)").arg(PROJECT_JSON_FILE_EXTENSION)
                                            + ";;" +
                                            tr("Project Files (*.%1)").arg(PROJECT_FILE_EXTENSION)
                                            );
    }

    if(name.isEmpty())
        return false;

    EnableSolverUpdate(false);
    undoStack.clear();

    bool no_error = true;

    QFile file(name);
    if (file.open(QFile::ReadOnly)) {
        //old format
        if(name.endsWith(PROJECT_FILE_EXTENSION, Qt::CaseInsensitive)) {
            QDataStream in(&file);
            no_error = ProjectFile::FromStream(this,in);
        } else {
        //new format
            bool binary = true;
            if(!name.endsWith(PROJECT_JSON_BINARY_FILE_EXTENSION, Qt::CaseInsensitive))
                binary = false;

			no_error = JsonReader::readProjectFile(&file, this, mainWindow, binary);
        }
    } else {
        no_error = false;
        QMessageBox::warning(mainWindow, tr("VstBoard"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(name),
                                  file.errorString()));
    }

    if(no_error) {
        currentProjectFile = name;
        objFactory->ResetAllSavedId();
    } else {
        ClearProject();
    }

    currentFileChanged();
    EnableSolverUpdate(true);

    return no_error;
}

void MainHost::ReloadProject()
{
    if(currentProjectFile.isEmpty())
        return;

    undoStack.clear();

    ProjectFile::LoadFromFile(this,currentProjectFile);
}

void MainHost::ReloadSetup()
{
    if(currentSetupFile.isEmpty())
        return;

    undoStack.clear();
}

void MainHost::ClearSetup()
{
    if(!programManager->userWantsToUnloadSetup())
        return;

    undoStack.clear();

    EnableSolverUpdate(false);
    SetupHostContainer();
    EnableSolverUpdate(true);
    if(mainWindow)
        mainWindow->viewConfig->LoadFromRegistry();

    currentSetupFile = "";
    currentFileChanged();
}

void MainHost::ClearProject()
{
    if(!programManager->userWantsToUnloadProject())
        return;

    undoStack.clear();

    EnableSolverUpdate(false);
    SetupProjectContainer();
    SetupProgramContainer();
    SetupGroupContainer();
    EnableSolverUpdate(true);

    programManager->BuildDefaultPrograms();

    currentProjectFile = "";
    currentFileChanged();
}

bool MainHost::SaveSetupFile(bool saveAs)
{
    QString name;

    if(currentSetupFile.isEmpty() || saveAs) {
        QString lastDir = settings->GetSetting("lastSetupDir").toString();
        name = QFileDialog::getSaveFileName(mainWindow, tr("Save Setup"), lastDir,
                                                tr("Binary Json (*.%1)").arg(SETUP_JSON_BINARY_FILE_EXTENSION)
                                                + ";;" +
                                                tr("Json (*.%1)").arg(SETUP_JSON_FILE_EXTENSION)
                                                + ";;" +
                                                tr("Setup Files (*.%1)").arg(SETUP_FILE_EXTENSION)
                                                );
        if(name.isEmpty())
            return false;
    } else {
        name = currentSetupFile;
    }

    //add default extension id needed
    if(!name.endsWith(SETUP_FILE_EXTENSION, Qt::CaseInsensitive) &&
            !name.endsWith(SETUP_JSON_FILE_EXTENSION, Qt::CaseInsensitive)  &&
            !name.endsWith(SETUP_JSON_BINARY_FILE_EXTENSION, Qt::CaseInsensitive)) {
        name += ".";
        name += SETUP_JSON_BINARY_FILE_EXTENSION;
    }

    EnableSolverUpdate(false);

    bool no_error = true;

    QFile file(name);
    if (file.open(QFile::WriteOnly)) {

        //old format
        if(name.endsWith(SETUP_FILE_EXTENSION, Qt::CaseInsensitive)) {
            QDataStream out(&file);
            no_error = ProjectFile::ToStream(this,out,SETUP_FILE_KEY);
        } else {
        //new format
            bool binary = true;
            if(!name.endsWith(SETUP_JSON_BINARY_FILE_EXTENSION, Qt::CaseInsensitive))
                binary = false;

            no_error = JsonWriter::writeProjectFile(&file,this, mainWindow,false,true,binary);
        }

    } else {
        no_error = false;
        QMessageBox::warning(mainWindow, tr("VstBoard"),
                     tr("Cannot write file %1:\n%2.")
                     .arg(QDir::toNativeSeparators(name),
                          file.errorString()));
    }

    if(no_error) {
        currentSetupFile = name;
    }

    currentFileChanged();
    EnableSolverUpdate(true);
    return no_error;
}

bool MainHost::SaveProjectFile(bool saveAs)
{
    QString name;

    if(currentProjectFile.isEmpty() || saveAs) {
        QString lastDir = settings->GetSetting("lastProjectDir").toString();
        name = QFileDialog::getSaveFileName(mainWindow, tr("Save Project"), lastDir,
                                                tr("Binary Json (*.%1)").arg(PROJECT_JSON_BINARY_FILE_EXTENSION)
                                                + ";;" +
                                                tr("Json (*.%1)").arg(PROJECT_JSON_FILE_EXTENSION)
                                                + ";;" +
                                                tr("Project Files (*.%1)").arg(PROJECT_FILE_EXTENSION)
                                                );
        if(name.isEmpty())
            return false;
    } else {
        name = currentProjectFile;
    }

    //add default extension id needed
    if(!name.endsWith(PROJECT_FILE_EXTENSION, Qt::CaseInsensitive) &&
        !name.endsWith(PROJECT_JSON_FILE_EXTENSION, Qt::CaseInsensitive) &&
        !name.endsWith(PROJECT_JSON_BINARY_FILE_EXTENSION, Qt::CaseInsensitive) ) {
        name += ".";
        name += PROJECT_JSON_BINARY_FILE_EXTENSION;
    }

    EnableSolverUpdate(false);

    bool no_error = true;

    QFile file(name);
    if (file.open(QFile::WriteOnly)) {
        //old format
        if(name.endsWith(PROJECT_FILE_EXTENSION, Qt::CaseInsensitive)) {
            QDataStream out(&file);
            no_error = ProjectFile::ToStream(this,out,PROJECT_FILE_KEY);
        } else {
        //new format
            bool binary = true;
            if(!name.endsWith(PROJECT_JSON_BINARY_FILE_EXTENSION, Qt::CaseInsensitive))
                binary = false;

            no_error = JsonWriter::writeProjectFile(&file,this,mainWindow,true,false,binary);
        }
    } else {
        no_error = false;
        QMessageBox::warning(mainWindow, tr("VstBoard"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(name),
                                  file.errorString()));
    }

    if(no_error) {
        currentProjectFile = name;
    }

    currentFileChanged();
    EnableSolverUpdate(true);
    return no_error;
}

void MainHost::currentFileChanged()
{
//    MsgObject msg(FixedObjId::mainWindow);
    _MSGOBJ(msg,FixedObjId::mainWindow);
    msg.prop[MsgObject::Setup]=currentSetupFile;
    msg.prop[MsgObject::Project]=currentProjectFile;
    SendMsg(msg);
}

void MainHost::ReceiveMsg(const MsgObject &msg)
{

    if(msg.objIndex == FixedObjId::mainHost) {

        if(msg.prop.contains(MsgObject::Undo)) {
            undoStack.undo();
            return;
        }

        if(msg.prop.contains(MsgObject::Redo)) {
            undoStack.redo();
            return;
        }

        if(msg.prop.contains(MsgObject::Project)) {
            switch(msg.prop[MsgObject::Project].toInt()) {
                case MsgObject::Load :
                    LoadProjectFile();
                    break;
                case MsgObject::Clear :
                    ClearProject();
                    break;
                case MsgObject::Save :
                    SaveProjectFile();
                    break;
                case MsgObject::SaveAs :
                    SaveProjectFile(true);
                    break;
            }
            return;
        }

        if(msg.prop.contains(MsgObject::Setup)) {
            switch(msg.prop[MsgObject::Setup].toInt()) {
                case MsgObject::Load :
                    LoadSetupFile();
                    break;
                case MsgObject::Clear :
                    ClearSetup();
                    break;
                case MsgObject::Save :
                    SaveSetupFile();
                    break;
                case MsgObject::SaveAs :
                    SaveSetupFile(true);
                    break;
            }
            return;
        }


        return;
    }

    if(msg.objIndex == FixedObjId::renderer) {
        if(msg.prop.contains(MsgObject::GetUpdate)) {
            UpdateRendererMap();
        }
        if(msg.prop.contains(MsgObject::Update)) {
            if(msg.prop[MsgObject::Update].toBool()) {
                UpdateRendererView();
                updateRendererViewTimer.start(5000);
            } else {
                updateRendererViewTimer.stop();
            }
        }
        return;
    }

    //intercept project and setup files
    if(msg.prop.contains(MsgObject::FilesToLoad)) {
        QStringList lstFiles = msg.prop[MsgObject::FilesToLoad].toStringList();
        foreach(const QString filename, lstFiles) {
            QFileInfo info;
            info.setFile( filename );
            if ( info.isFile() && info.isReadable() ) {
                QString fileType(info.suffix().toLower());

                //setup file
                if ( fileType==SETUP_FILE_EXTENSION ) {
                    LoadSetupFile(filename);
                    continue;
                }

                //project file
                if ( fileType==PROJECT_FILE_EXTENSION ) {
                    LoadProjectFile(filename);
                    continue;
                }

            }
        }
     }

    //send msg to dest object
    if(listObj.contains(msg.objIndex)) {
        listObj[msg.objIndex]->ReceiveMsg(msg);
    }
}

void MainHost::GetRenderMap(RenderMap &map)
{
    map=renderer->currentMap;
}

void MainHost::SetRenderMap(const RenderMap &map)
{
    renderer->SetMap(map,nbThreads);
    if(updateRendererViewTimer.isActive())
        UpdateRendererView();
}

void MainHost::UpdateView()
{
    if( hostContainer)
        hostContainer->UpdateView();
    if(projectContainer)
        projectContainer->UpdateView();
    if(groupContainer)
        groupContainer->UpdateView();
    if(programContainer)
        programContainer->UpdateView();
}
