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
#include "mainhost.h"
#include "mainwindow.h"
#include "models/programsmodel.h"
#include "connectables/objects/container.h"

#ifdef VSTSDK
    #include "connectables/objects/vstplugin.h"
    int MainHost::vstUsersCounter=0;
#endif

#include "projectfile/fileversion.h"
#include "projectfile/projectfile.h"
#include "views/configdialog.h"
#include "events.h"

EngineThread::EngineThread(QObject *parent) :
    QThread(parent)
{
    setObjectName("EngineThread");
    start(QThread::HighPriority);
//    start(QThread::TimeCriticalPriority);
}

EngineThread::~EngineThread()
{
    LOG("stop thread"<<objectName()<<(int)currentThreadId());
    quit();
    wait(1000);
}

void EngineThread::run()
{
    LOG("start thread"<<objectName()<<(int)currentThreadId());
    exec();
}

quint32 MainHost::currentFileVersion=PROJECT_AND_SETUP_FILE_VERSION;

MainHost::MainHost(QObject *parent, QString settingsGroup) :
    QObject(parent),
    MetaTransporter(),
    solver(new PathSolver(this)),
    objFactory(0),
    mainWindow(0),
    solverNeedAnUpdate(false),
    solverUpdateEnabled(true),
    mutexListCables(new DMutex(QMutex::Recursive)),
    settingsGroup(settingsGroup),
    undoProgramChangesEnabled(false),
    undoStack(new QUndoStack(this)),
    renderer(0),
    programsModel(0),
    updateViewTimer(0),
    doublePrecision(false)

{
    setObjectName("MainHost");

    SET_MUTEX_NAME(mutexRender,"mutexRender mainHost");
    SET_MUTEX_NAME((*mutexListCables),"mutexListCables mainHost");
    SET_MUTEX_NAME(mutexSolver,"mutexSolver mainHost");

    sampleRate = 44100.0;
    bufferSize = 100;

    currentTempo=120;
    currentTimeSig1=4;
    currentTimeSig2=4;

    doublePrecision=GetSetting("doublePrecision",false).toBool();
    MetaTransporter::autoUpdate=false;

#ifdef VSTSDK
    vstHost=0;
#endif

#ifdef SCRIPTENGINE
    scriptEngine = new QScriptEngine(this);
    QScriptValue scriptObj = scriptEngine->newQObject(this);
    scriptEngine->globalObject().setProperty("MainHost", scriptObj);
#endif
}

MainHost::~MainHost()
{
    RemoveAllListeners();

    EnableSolverUpdate(false);

    updateViewTimer->stop();
    delete updateViewTimer;
    updateViewTimer=0;

    mutexListCables->lock();
    workingListOfCables.clear();
    mutexListCables->unlock();

    solver->Resolve(workingListOfCables, renderer);
    delete renderer;
    renderer=0;

    mainContainer.clear();
    hostContainer.clear();
    projectContainer.clear();
    groupContainer.clear();
    programContainer.clear();

    delete objFactory;

#ifdef VSTSDK
    vstUsersCounter--;
    if(vstUsersCounter==0 && vstHost)
        delete vstHost;
#endif

    delete mutexListCables;
}

void MainHost::InitThread()
{
#ifdef VSTSDK
    if(!vst::CVSTHost::Get())
        vstHost = new vst::CVSTHost();
    else
        vstHost = vst::CVSTHost::Get();

    vstUsersCounter++;
#endif

    renderer = new Renderer(this);

    //timer
    timeFromStart.start();

    updateViewTimer = new QTimer(this);
    updateViewTimer->start(40);

    connect(this,SIGNAL(SolverToUpdate()),
            this,SLOT(UpdateSolver()),
            Qt::QueuedConnection);

    programsModel = new ProgramsModel(this);
}

void MainHost::SetMainWindow(MainWindow *win)
{
    mainWindow=win;

    EnableSolverUpdate(false);
    SetupMainContainer();
    SetupHostContainer();
    SetupProjectContainer();
    SetupProgramContainer();
    SetupGroupContainer();

    EnableSolverUpdate(true);
    programsModel->BuildDefaultModel();
}

bool MainHost::event(QEvent *event)
{
    switch(event->type()) {
        case Events::typeCommand : {
            Events::command *e = static_cast<Events::command*>(event);
            undoStack->push(e->cmd);
            return true;
        }
        case Events::typeValChanged : {
            Events::valChanged *e = static_cast<Events::valChanged*>(event);
            switch(e->objInfo.Type()) {
                case MetaTypes::object : {
                    QSharedPointer<Connectables::Object>obj = objFactory->GetObjectFromId(e->objInfo.ObjId());
                    if(!obj)
                        return true;
                    obj->SetMeta(e->type,e->value);
                    return true;
                }
                case MetaTypes::pin: {
                    Connectables::Pin *pin = objFactory->GetPin(e->objInfo);
                    if(!pin)
                        return true;

                    Connectables::ParameterPin* p = static_cast<Connectables::ParameterPin*>(pin);
                    p->SetMeta(e->type,e->value);
                    switch(e->type) {
                        case MetaInfos::Value :
                        case MetaInfos::LimitInMin :
                        case MetaInfos::LimitInMax :
                        case MetaInfos::LimitOutMin :
                        case MetaInfos::LimitOutMax :
                            p->ChangeValue(p->Meta(MetaInfos::Value).toFloat());
                    }
                    return true;
                }
            }
        }
    }

    return QObject::event(event);
}

void MainHost::SetupMainContainer()
{
    MetaInfo info(MetaTypes::container);
    info.SetName("mainContainer");
    info.SetObjId(FixedObjIds::mainContainer);

    mainContainer = objFactory->NewObject(info).staticCast< Connectables::Container >();
    if(mainContainer.isNull())
        return;

    mainContainer->SetLoadingMode(true);

//    mainContainer->LoadProgram(0);
//    QStandardItem *item = mainContainer->GetFullItem();
//    model->invisibleRootItem()->appendRow(item);
//    mainContainer->modelIndex=item->index();
//    mainContainer->GetFullItem();
    mainContainer->Resume();
    Events::sendObj *event = new Events::sendObj(mainContainer->info(), Events::typeNewObj);
    PostEvent(event);

    mainContainer->parked=false;
    mainContainer->listenProgramChanges=false;

    mainContainer->SetLoadingMode(false);
    mainContainer->UpdateModificationTime();
    SetSolverUpdateNeeded();
}

void MainHost::SetupHostContainer()
{
    if(!hostContainer.isNull()) {
        mainContainer->ParkObject( hostContainer );
        hostContainer.clear();
        UpdateSolver(true);
        if(mainWindow)
            mainWindow->mySceneView->viewHost->ClearViewPrograms();
    }

    MetaInfo info(MetaTypes::container);
    info.SetName("hostContainer");
    info.SetObjId(FixedObjIds::hostContainer);

    hostContainer = objFactory->NewObject(info).staticCast<Connectables::Container>();
    if(hostContainer.isNull())
        return;

    hostContainer->SetLoadingMode(true);

//    hostContainer->LoadProgram(0);
    mainContainer->AddObject(hostContainer);

    QSharedPointer<Connectables::Object> bridge;


    //bridge in
    MetaInfo in(MetaTypes::bridge);
    in.SetName("in");
    in.SetMeta(MetaInfos::Direction, Directions::Input);
    in.SetObjId(FixedObjIds::hostContainerIn);

    bridge = objFactory->NewObject(in);
//    bridge->SetBridgePinsInVisible(false);
    hostContainer->AddObject( bridge );

    hostContainer->bridgeIn = bridge;

    //bridge out
    MetaInfo out(MetaTypes::bridge);
    out.SetName("out");
    out.SetMeta(MetaInfos::Direction, Directions::Output);
    out.SetObjId(FixedObjIds::hostContainerOut);

    bridge = objFactory->NewObject(out);
//    bridge->SetBridgePinsOutVisible(false);
    hostContainer->AddObject( bridge );

    hostContainer->bridgeOut = bridge;

    //connect with groupContainer
    if(!groupContainer.isNull()) {
        mainContainer->ConnectObjects(groupContainer->bridgeSend, hostContainer->bridgeIn,true);
        mainContainer->ConnectObjects(hostContainer->bridgeOut, groupContainer->bridgeReturn,true);
    }

    //send bridge
    MetaInfo send(MetaTypes::bridge);
    send.SetName("send");
    send.SetMeta(MetaInfos::Direction, Directions::Send);
    send.SetObjId(FixedObjIds::hostContainerSend);

    bridge = objFactory->NewObject(send);
//    bridge->SetBridgePinsOutVisible(false);
    hostContainer->AddObject( bridge );

    hostContainer->bridgeSend = bridge;

    //return bridge
    MetaInfo retrn(MetaTypes::bridge);
    retrn.SetName("return");
    retrn.SetMeta(MetaInfos::Direction, Directions::Return);
    retrn.SetObjId(FixedObjIds::hostContainerReturn);

    bridge = objFactory->NewObject(retrn);
//    bridge->SetBridgePinsInVisible(false);
    hostContainer->AddObject( bridge );

    hostContainer->bridgeReturn = bridge;

    //connect with projectContainer
    if(!projectContainer.isNull()) {
        mainContainer->ConnectObjects(hostContainer->bridgeSend, projectContainer->bridgeIn,true);
        mainContainer->ConnectObjects(projectContainer->bridgeOut, hostContainer->bridgeReturn,true);
    }
    hostContainer->listenProgramChanges=false;

    if(projectContainer)
        hostContainer->childContainer=projectContainer;

    hostContainer->SetLoadingMode(false);
    hostContainer->UpdateModificationTime();
    SetSolverUpdateNeeded();
}

void MainHost::SetupProjectContainer()
{
    if(!projectContainer.isNull()) {
        mainContainer->ParkObject( projectContainer );
        projectContainer.clear();
        UpdateSolver(true);
        if(mainWindow)
            mainWindow->mySceneView->viewProject->ClearViewPrograms();
    }

    timeFromStart.restart();

    MetaInfo info(MetaTypes::container);
    info.SetName("projectContainer");
    info.SetObjId(FixedObjIds::projectContainer);

    projectContainer = objFactory->NewObject(info).staticCast<Connectables::Container>();
    if(projectContainer.isNull())
        return;

    projectContainer->SetLoadingMode(true);

//    projectContainer->LoadProgram(0);
    mainContainer->AddObject(projectContainer);

    QSharedPointer<Connectables::Object> bridge;

    //bridge in
    MetaInfo in(MetaTypes::bridge);
    in.SetName("in");
    in.SetMeta(MetaInfos::Direction, Directions::Input);
    in.SetObjId(FixedObjIds::projectContainerIn);

    bridge = objFactory->NewObject(in);
//    bridge->SetBridgePinsInVisible(false);
    projectContainer->AddObject( bridge );

    projectContainer->bridgeIn = bridge;

    //bridge out
    MetaInfo out(MetaTypes::bridge);
    out.SetName("out");
    out.SetMeta(MetaInfos::Direction, Directions::Output);
    out.SetObjId(FixedObjIds::projectContainerOut);

    bridge = objFactory->NewObject(out);
//    bridge->SetBridgePinsOutVisible(false);
    projectContainer->AddObject( bridge );

    projectContainer->bridgeOut = bridge;

    //connect with hostContainer
    if(!hostContainer.isNull()) {
        mainContainer->ConnectObjects(hostContainer->bridgeSend, projectContainer->bridgeIn,true);
        mainContainer->ConnectObjects(projectContainer->bridgeOut, hostContainer->bridgeReturn,true);
    }


    //bridge send
    MetaInfo send(MetaTypes::bridge);
    send.SetName("send");
    send.SetMeta(MetaInfos::Direction, Directions::Send);
    send.SetObjId(FixedObjIds::projectContainerSend);

    bridge = objFactory->NewObject(send);
//    bridge->SetBridgePinsOutVisible(false);
    projectContainer->AddObject( bridge );

    projectContainer->bridgeSend = bridge;

    //bridge return
    MetaInfo retrn(MetaTypes::bridge);
    retrn.SetName("return");
    retrn.SetMeta(MetaInfos::Direction, Directions::Return);
    retrn.SetObjId(FixedObjIds::projectContainerReturn);

    bridge = objFactory->NewObject(retrn);
//    bridge->SetBridgePinsInVisible(false);
    projectContainer->AddObject( bridge );

    projectContainer->bridgeReturn = bridge;

    //connect with programContainer
    if(!programContainer.isNull()) {
        mainContainer->ConnectObjects(projectContainer->bridgeSend, programContainer->bridgeIn,true);
        mainContainer->ConnectObjects(programContainer->bridgeOut, projectContainer->bridgeReturn,true);
    }

    //connect with itself (pass-though cables)
    projectContainer->ConnectObjects(projectContainer->bridgeIn, projectContainer->bridgeSend,false);
    projectContainer->ConnectObjects(projectContainer->bridgeReturn, projectContainer->bridgeOut,false);

    projectContainer->listenProgramChanges=false;

    if(hostContainer)
        hostContainer->childContainer=projectContainer;
    if(groupContainer)
        projectContainer->childContainer=groupContainer;

    projectContainer->SetLoadingMode(false);
    projectContainer->UpdateModificationTime();
    SetSolverUpdateNeeded();
}

void MainHost::SetupProgramContainer()
{
    if(!programContainer.isNull()) {
        mainContainer->ParkObject( programContainer );
        programContainer.clear();
        UpdateSolver(true);
        if(mainWindow)
            mainWindow->mySceneView->viewProgram->ClearViewPrograms();
    }

    MetaInfo info(MetaTypes::container);
    info.SetName("programContainer");
    info.SetObjId(FixedObjIds::programContainer);

    programContainer = objFactory->NewObject(info).staticCast<Connectables::Container>();
    if(programContainer.isNull())
        return;

    programContainer->SetLoadingMode(true);

    programContainer->SetOptimizerFlag(true);
//    programContainer->LoadProgram(0);
    mainContainer->AddObject(programContainer);

    QSharedPointer<Connectables::Object> bridge;

    //bridge in
    MetaInfo in(MetaTypes::bridge);
    in.SetName("in");
    in.SetMeta(MetaInfos::Direction, Directions::Input);
    in.SetObjId(FixedObjIds::programContainerIn);

    bridge = objFactory->NewObject(in);
//    bridge->SetBridgePinsInVisible(false);
    programContainer->AddObject( bridge );

    programContainer->bridgeIn = bridge;

    //bridge out
    MetaInfo out(MetaTypes::bridge);
    out.SetName("out");
    out.SetMeta(MetaInfos::Direction, Directions::Output);
    out.SetObjId(FixedObjIds::programContainerOut);

    bridge = objFactory->NewObject(out);
//    bridge->SetBridgePinsOutVisible(false);
    programContainer->AddObject( bridge );

    programContainer->bridgeOut = bridge;

    //connect with projectContainer
    if(!projectContainer.isNull()) {
        mainContainer->ConnectObjects(projectContainer->bridgeSend, programContainer->bridgeIn,true);
        mainContainer->ConnectObjects(programContainer->bridgeOut, projectContainer->bridgeReturn,true);
    }


    //bridge send
    MetaInfo send(MetaTypes::bridge);
    send.SetName("send");
    send.SetMeta(MetaInfos::Direction, Directions::Send);
    send.SetObjId(FixedObjIds::programContainerSend);

    bridge = objFactory->NewObject(send);
//    bridge->SetBridgePinsOutVisible(false);
    programContainer->AddObject( bridge );

    programContainer->bridgeSend = bridge;

    //bridge return
    MetaInfo retrn(MetaTypes::bridge);
    retrn.SetName("return");
    retrn.SetMeta(MetaInfos::Direction, Directions::Return);
    retrn.SetObjId(FixedObjIds::programContainerReturn);

    bridge = objFactory->NewObject(retrn);
//    bridge->SetBridgePinsInVisible(false);
    programContainer->AddObject( bridge );

    programContainer->bridgeReturn = bridge;

    //connect with groupContainer
    if(!groupContainer.isNull()) {
        mainContainer->ConnectObjects(programContainer->bridgeSend, groupContainer->bridgeIn,true);
        mainContainer->ConnectObjects(groupContainer->bridgeOut, programContainer->bridgeReturn,true);
    }

    connect(programsModel, SIGNAL(ProgChanged(QModelIndex,bool)),
            programContainer.data(), SLOT(SetProgram(QModelIndex,bool)));
    connect(programsModel, SIGNAL(ProgDelete(int)),
            programContainer.data(), SLOT(RemoveProgram(int)));

    emit programParkingModelChanged(&programContainer->parkModel);

    if(groupContainer) {
        groupContainer->childContainer=programContainer;
        programContainer->parentContainer=groupContainer;
    }

    programContainer->SetLoadingMode(false);
    programContainer->UpdateModificationTime();
    SetSolverUpdateNeeded();
}

void MainHost::SetupGroupContainer()
{
    if(!groupContainer.isNull()) {
        mainContainer->ParkObject( groupContainer );
        groupContainer.clear();
        UpdateSolver(true);
        if(mainWindow)
            mainWindow->mySceneView->viewGroup->ClearViewPrograms();
    }

    MetaInfo info(MetaTypes::container);
    info.SetName("groupContainer");
    info.SetObjId(FixedObjIds::groupContainer);

    groupContainer = objFactory->NewObject(info).staticCast<Connectables::Container>();
    if(groupContainer.isNull())
        return;

    groupContainer->SetLoadingMode(true);

//    groupContainer->LoadProgram(0);
    mainContainer->AddObject(groupContainer);

    QSharedPointer<Connectables::Object> bridge;

    //bridge in
    MetaInfo in(MetaTypes::bridge);
    in.SetName("in");
    in.SetMeta(MetaInfos::Direction, Directions::Input);
    in.SetObjId(FixedObjIds::groupContainerIn);

    bridge = objFactory->NewObject(in);
//    bridge->SetBridgePinsInVisible(false);
    groupContainer->AddObject( bridge );

    groupContainer->bridgeIn = bridge;

    //bridge out
    MetaInfo out(MetaTypes::bridge);
    out.SetName("out");
    out.SetMeta(MetaInfos::Direction, Directions::Output);
    out.SetObjId(FixedObjIds::groupContainerOut);

    bridge = objFactory->NewObject(out);
//    bridge->SetBridgePinsOutVisible(false);
    groupContainer->AddObject( bridge );

    groupContainer->bridgeOut = bridge;

    //connect with programContainer
    if(!programContainer.isNull()) {
        mainContainer->ConnectObjects(programContainer->bridgeSend, groupContainer->bridgeIn,true);
        mainContainer->ConnectObjects(groupContainer->bridgeOut, programContainer->bridgeReturn,true);
    }

    //bridge send
    MetaInfo send(MetaTypes::bridge);
    send.SetName("send");
    send.SetMeta(MetaInfos::Direction, Directions::Send);
    send.SetObjId(FixedObjIds::groupContainerSend);

    bridge = objFactory->NewObject(send);
//    bridge->SetBridgePinsOutVisible(false);
    groupContainer->AddObject( bridge );

    groupContainer->bridgeSend = bridge;

    //bridge return
    MetaInfo retrn(MetaTypes::bridge);
    retrn.SetName("return");
    retrn.SetMeta(MetaInfos::Direction, Directions::Return);
    retrn.SetObjId(FixedObjIds::groupContainerReturn);

    bridge = objFactory->NewObject(retrn);
//    bridge->SetBridgePinsInVisible(false);
    groupContainer->AddObject( bridge );

    groupContainer->bridgeReturn = bridge;

    //connect with hostContainer
    if(!hostContainer.isNull()) {
        mainContainer->ConnectObjects(groupContainer->bridgeSend, hostContainer->bridgeIn,true);
        mainContainer->ConnectObjects(hostContainer->bridgeOut, groupContainer->bridgeReturn,true);
    }

    connect(programsModel, SIGNAL(GroupChanged(QModelIndex,bool)),
            groupContainer.data(), SLOT(SetProgram(QModelIndex,bool)));
    connect(programsModel, SIGNAL(GroupDelete(int)),
            groupContainer.data(), SLOT(RemoveProgram(int)));

    emit groupParkingModelChanged(&groupContainer->parkModel);

    if(projectContainer)
        projectContainer->childContainer=groupContainer;
    if(programContainer) {
        groupContainer->childContainer=programContainer;
        programContainer->parentContainer=groupContainer;
    }

    groupContainer->SetLoadingMode(false);
    groupContainer->UpdateModificationTime();
    SetSolverUpdateNeeded();
}

bool MainHost::EnableSolverUpdate(bool enable)
{
    mutexSolver.lock();
    bool ret = solverUpdateEnabled;
    solverUpdateEnabled = enable;
    mutexSolver.unlock();
    return ret;
}

void MainHost::UpdateSolver(bool forceUpdate)
{
    if(!renderer)
        return;

    mutexSolver.lock();

        //update not forced, not needed or disabled : return
        if( (!solverUpdateEnabled || !solverNeedAnUpdate) && !forceUpdate) {
            mutexSolver.unlock();
            return;
        }

        //disable other solver updates
        bool solverWasEnabled=solverUpdateEnabled;
        solverUpdateEnabled = false;

        //allow others to ask for a new update while we're updating
        solverNeedAnUpdate = false;

    mutexSolver.unlock();

    //if forced : lock rendering
    if(forceUpdate) {
        mutexRender.lock();
    } else {
        //not forced : do it later if we can't do it now
        if(!mutexRender.tryLock()) {
            //can't lock, ask for a ne update
            SetSolverUpdateNeeded();
            EnableSolverUpdate(solverWasEnabled);
            return;
        }
    }

    //update the solver
    mutexListCables->lock();
        solver->Resolve(workingListOfCables, renderer);
    mutexListCables->unlock();

    mutexRender.unlock();
    EnableSolverUpdate(solverWasEnabled);
}

void MainHost::ChangeNbThreads(int nbThreads)
{
    if(!renderer)
        return;

    if(nbThreads<=0) {
#ifdef _WIN32
        SYSTEM_INFO info;
        GetSystemInfo(&info);
        nbThreads = info.dwNumberOfProcessors;
#else
        nbThreads = 1;
#endif
    }

    mutexRender.lock();
    renderer->SetNbThreads(nbThreads);
    mutexRender.unlock();
    SetSolverUpdateNeeded();

}

void MainHost::SendMsg(const MetaInfo &senderPin,const PinMessage::Enum msgType,void *data)
{
    QMutexLocker lock(mutexListCables);

    mapCables::const_iterator i = workingListOfCables.constFind(senderPin);
    while (i != workingListOfCables.constEnd()  && i.key() == senderPin) {
        Connectables::Pin *pin = objFactory->GetPin( i.value() );
        if(!pin) {
            LOG("unknown pin");
            return;
        }

        pin->ReceiveMsg(msgType,data);
        ++i;
    }
}

void MainHost::SetBufferSizeMs(unsigned int ms)
{
    ulong size = ms*sampleRate/1000;
    SetBufferSize(size);
}

void MainHost::SetBufferSize(unsigned long size)
{
    qDebug()<<"MainHost::SetBufferSize"<<size;
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

//void MainHost::OnNewRenderingOrder(orderedNodes * renderLines)
//{
//    emit NewSolver(renderLines);
//}

void MainHost::Render(unsigned long samples)
{
    if(samples==0)
        samples=bufferSize;

#ifdef VSTSDK
    CheckTempo();
#endif

    if(!renderer)
        return;

    mutexRender.lock();

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

    renderer->StartRender();
    mutexRender.unlock();

    if(solverNeedAnUpdate && solverUpdateEnabled)
        emit SolverToUpdate();
}

void MainHost::OnCableAdded(Connectables::Cable *cab)
{
    mutexListCables->lock();
    workingListOfCables.insert(cab->GetInfoOut(),cab->GetInfoIn());
    mutexListCables->unlock();
}

void MainHost::OnCableRemoved(Connectables::Cable *cab)
{
    mutexListCables->lock();
    workingListOfCables.remove(cab->GetInfoOut(),cab->GetInfoIn());
    mutexListCables->unlock();
}

void MainHost::SetTimeInfo(const VstTimeInfo *info)
{
#ifdef VSTSDK
    vstHost->SetTimeInfo(info);
//    CheckTempo();
#endif
}

void MainHost::SetTempo(int tempo, int sign1, int sign2)
{
#ifdef VSTSDK
    vstHost->SetTempo(tempo,sign1,sign2);
//    CheckTempo();
#endif
}

void MainHost::CheckTempo()
{
#ifdef VSTSDK
    int tempo=0;
    int sign1=0;
    int sign2=0;

    vstHost->GetTempo(tempo,sign1,sign2);
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

void MainHost::SetSetting(QString name, QVariant value)
{
    settings.setValue(settingsGroup + name,value);
}

QVariant MainHost::GetSetting(QString name, QVariant defaultVal)
{
    return settings.value(settingsGroup + name,defaultVal);
}

bool MainHost::SettingDefined(QString name)
{
    return settings.contains(settingsGroup + name);
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
    if(!programsModel->userWantsToUnloadSetup())
        return;

    QString name = filename;

    if(name.isEmpty()) {
        mainWindow->OpenFileDialog(tr("Open a Setup file"),GetSetting("lastSetupDir").toString(),tr("Setup Files (*.%1)").arg(SETUP_FILE_EXTENSION));
        name = mainWindow->GetLastSelectedFile();
    }

    if(name.isEmpty())
        return;

    undoStack->clear();

    if(ProjectFile::LoadFromFile(this,name)) {
        ConfigDialog::AddRecentSetupFile(name,this);
        currentSetupFile = name;
    } else {
        ConfigDialog::RemoveRecentSetupFile(name,this);
        ClearSetup();
    }
    emit currentFileChanged();
}

void MainHost::LoadProjectFile(const QString &filename)
{
    if(!programsModel->userWantsToUnloadProject())
        return;

    QString name = filename;

    if(name.isEmpty()) {
        mainWindow->OpenFileDialog(tr("Open a Project file"),GetSetting("lastProjectDir").toString(),tr("Project Files (*.%1)").arg(PROJECT_FILE_EXTENSION));
        name = mainWindow->GetLastSelectedFile();
    }

    if(name.isEmpty())
        return;

    undoStack->clear();

    if(ProjectFile::LoadFromFile(this,name)) {
        ConfigDialog::AddRecentProjectFile(name,this);
        currentProjectFile = name;
    } else {
        ConfigDialog::RemoveRecentProjectFile(name,this);
        ClearProject();
    }
    emit currentFileChanged();
}

void MainHost::ReloadProject()
{
    if(currentProjectFile.isEmpty())
        return;

    undoStack->clear();

    ProjectFile::LoadFromFile(this,currentProjectFile);
}

void MainHost::ReloadSetup()
{
    if(currentSetupFile.isEmpty())
        return;

    undoStack->clear();

    ConfigDialog::AddRecentSetupFile(currentSetupFile,this);
}

void MainHost::ClearSetup()
{
    if(!programsModel->userWantsToUnloadSetup())
        return;

    undoStack->clear();

    EnableSolverUpdate(false);
    SetupHostContainer();
    EnableSolverUpdate(true);
    if(mainWindow)
        mainWindow->viewConfig->LoadFromRegistry();

    ConfigDialog::AddRecentSetupFile("",this);
    currentSetupFile = "";
    emit currentFileChanged();
}

void MainHost::ClearProject()
{
    if(!programsModel->userWantsToUnloadProject())
        return;

    undoStack->clear();

    EnableSolverUpdate(false);
    SetupProjectContainer();
    SetupProgramContainer();
    SetupGroupContainer();
    EnableSolverUpdate(true);

    programsModel->BuildDefaultModel();

    ConfigDialog::AddRecentProjectFile("",this);
    currentProjectFile = "";
    emit currentFileChanged();
}

void MainHost::SaveSetupFile(bool saveAs)
{
    QString filename;

    if(currentSetupFile.isEmpty() || saveAs) {
        if(!mainWindow)
            return;

        mainWindow->SaveFileDialog(tr("Save Setup"),GetSetting("lastSetupDir").toString(),tr("Setup Files (*.%1)").arg(SETUP_FILE_EXTENSION));
        filename = mainWindow->GetLastSelectedFile();

        if(filename.isEmpty())
            return;

        if(!filename.endsWith(SETUP_FILE_EXTENSION, Qt::CaseInsensitive)) {
            filename += ".";
            filename += SETUP_FILE_EXTENSION;
        }
    } else {
        filename = currentSetupFile;
    }

    if(ProjectFile::SaveToSetupFile(this,filename)) {
        SetSetting("lastSetupDir",QFileInfo(filename).absolutePath());
        ConfigDialog::AddRecentSetupFile(filename,this);
        currentSetupFile = filename;
        emit currentFileChanged();
    }
}

void MainHost::SaveProjectFile(bool saveAs)
{
    QString filename;

    if(currentProjectFile.isEmpty() || saveAs) {
        if(!mainWindow)
            return;

        mainWindow->SaveFileDialog(tr("Save Project"),GetSetting("lastProjectDir").toString(),tr("Project Files (*.%1)").arg(PROJECT_FILE_EXTENSION));
        filename = mainWindow->GetLastSelectedFile();

        if(filename.isEmpty())
            return;

        if(!filename.endsWith(PROJECT_FILE_EXTENSION, Qt::CaseInsensitive)) {
            filename += ".";
            filename += PROJECT_FILE_EXTENSION;
        }
    } else {
        filename = currentProjectFile;
    }

    if(ProjectFile::SaveToProjectFile(this,filename)) {
        SetSetting("lastProjectDir",QFileInfo(filename).absolutePath());
        ConfigDialog::AddRecentProjectFile(filename,this);
        currentProjectFile = filename;
        emit currentFileChanged();
    }
}
