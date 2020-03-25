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

#ifdef VSTSDK
    #include "connectables/vstplugin.h"
    int MainHost::vstUsersCounter=0;
#endif

#include "projectfile/fileversion.h"
#include "projectfile/projectfile.h"
#include "views/configdialog.h"
#include "msgobject.h"


quint32 MainHost::currentFileVersion=PROJECT_AND_SETUP_FILE_VERSION;

MainHost::MainHost(Settings *settings, QObject *parent) :
    QObject(parent)
    ,updateViewTimer(0)
    ,programManager(0)
    ,objFactory(0)
    ,mainWindow(0)
    #ifdef VSTSDK
        , vstHost(0)
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
        delete updateViewTimer;
        updateViewTimer=0;
    }

    if(renderer) {
        delete renderer;
        renderer=0;
    }

    hostContainer.clear();
    projectContainer.clear();
    groupContainer.clear();
    programContainer.clear();
    mainContainer.clear();

    if(objFactory) {
        delete objFactory;
        objFactory=0;
    }

    if(solver) {
        delete solver;
        solver=0;
    }

#ifdef VSTSDK
    vstUsersCounter--;
    if(vstUsersCounter==0)
        delete vstHost;

    if(vst3Host)
        delete vst3Host;
#endif

    if(programManager) {
        delete programManager;
        programManager=0;
    }
}

void MainHost::Kill()
{
    delete this;
}

void MainHost::Init()
{
    programManager =new ProgramManager(this);
    doublePrecision=settings->GetSetting("doublePrecision",false).toBool();

    setObjectName("MainHost");

#ifdef SCRIPTENGINE
    scriptEngine = new QScriptEngine(this);
    QScriptValue scriptObj = scriptEngine->newQObject(this);
    scriptEngine->globalObject().setProperty("MainHost", scriptObj);
#endif

#ifdef VSTSDK
    if(!vst::CVSTHost::Get()) {
        vstHost = new vst::CVSTHost();
    } else {
        vstHost = vst::CVSTHost::Get();
    }

    vst3Host = new Vst3Host();

    vstUsersCounter++;
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
    updateViewTimer->start(120);

    connect(this,SIGNAL(SolverToUpdate()),
            this,SLOT(UpdateSolver()),
            Qt::QueuedConnection);

    updateRendererViewTimer.start(5000);
    connect(&updateRendererViewTimer, SIGNAL(timeout()),
            this,SLOT(UpdateRendererView()));
}

void MainHost::Open()
{

    EnableSolverUpdate(false);

    SetupMainContainer();
    SetupHostContainer();
    SetupProjectContainer();
    SetupProgramContainer();
    SetupGroupContainer();

    EnableSolverUpdate(true);
    programManager->BuildDefaultPrograms();
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
    MsgObject msg(FixedObjId::mainWindow);
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


#ifdef VSTSDK
void MainHost::SetTimeInfo(const VstTimeInfo *info)
{
    vstHost->SetTimeInfo(info);
}
#endif

void MainHost::SetTempo(int tempo, int sign1, int sign2)
{
#ifdef VSTSDK
    vstHost->SetTempo(tempo,sign1,sign2);
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
#ifdef VSTSDK
    vstHost->GetTempo(tempo,sign1,sign2);
#else
    tempo=120;
    sign1=4;
    sign2=4;
#endif
}

void MainHost::LoadFile(const QString &filename)
{
    QFileInfo info(filename);
    if ( info.suffix()==SETUP_FILE_EXTENSION ) {
        LoadSetupFile(filename);
    }
    if ( info.suffix()==PROJECT_FILE_EXTENSION ) {
        LoadProjectFile(filename);
    }
}

void MainHost::LoadSetupFile(const QString &filename)
{
    if(!programManager->userWantsToUnloadSetup())
        return;

    QString name = filename;

    if(name.isEmpty()) {
        QString lastDir = settings->GetSetting("lastSetupDir").toString();
        name = QFileDialog::getOpenFileName(mainWindow, tr("Open a Setup file"), lastDir, tr("Setup Files (*.%1)").arg(SETUP_FILE_EXTENSION));
    }

    if(name.isEmpty())
        return;

    undoStack.clear();

    if(ProjectFile::LoadFromFile(this,name)) {
        currentSetupFile = name;
    } else {
        ClearSetup();
    }

    currentFileChanged();
}

void MainHost::LoadProjectFile(const QString &filename)
{
    if(!programManager->userWantsToUnloadProject())
        return;

    QString name = filename;

    if(name.isEmpty()) {
        QString lastDir = settings->GetSetting("lastProjectDir").toString();
        name = QFileDialog::getOpenFileName(mainWindow, tr("Open a Project file"), lastDir, tr("Project Files (*.%1)").arg(PROJECT_FILE_EXTENSION));
    }

    if(name.isEmpty())
        return;

    undoStack.clear();

    if(ProjectFile::LoadFromFile(this,name)) {
        currentProjectFile = name;
    } else {
        ClearProject();
    }

    currentFileChanged();
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
    QString filename;

    if(currentSetupFile.isEmpty() || saveAs) {
        QString lastDir = settings->GetSetting("lastSetupDir").toString();
        filename = QFileDialog::getSaveFileName(mainWindow, tr("Save Setup"), lastDir, tr("Setup Files (*.%1)").arg(SETUP_FILE_EXTENSION));

        if(filename.isEmpty())
            return false;

        if(!filename.endsWith(SETUP_FILE_EXTENSION, Qt::CaseInsensitive)) {
            filename += ".";
            filename += SETUP_FILE_EXTENSION;
        }
    } else {
        filename = currentSetupFile;
    }

    if(ProjectFile::SaveToSetupFile(this,filename)) {
        currentSetupFile = filename;
        currentFileChanged();
    }

    return true;
}

bool MainHost::SaveProjectFile(bool saveAs)
{
    QString filename;

    if(currentProjectFile.isEmpty() || saveAs) {
        QString lastDir = settings->GetSetting("lastProjectDir").toString();
        filename = QFileDialog::getSaveFileName(mainWindow, tr("Save Project"), lastDir, tr("Project Files (*.%1)").arg(PROJECT_FILE_EXTENSION));

        if(filename.isEmpty())
            return false;

        if(!filename.endsWith(PROJECT_FILE_EXTENSION, Qt::CaseInsensitive)) {
            filename += ".";
            filename += PROJECT_FILE_EXTENSION;
        }
    } else {
        filename = currentProjectFile;
    }

    if(ProjectFile::SaveToProjectFile(this,filename)) {
        currentProjectFile = filename;
        currentFileChanged();
    }
    return true;
}

void MainHost::currentFileChanged()
{
    MsgObject msg(FixedObjId::mainWindow);
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
