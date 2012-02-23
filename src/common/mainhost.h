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

#ifndef MAINHOST_H
#define MAINHOST_H

//#include "precomp.h"
#include <QUndoStack>
#include "connectables/objectfactory.h"
#include "connectables/object.h"
#include "connectables/container.h"
//#include "renderer/pathsolver.h"
//#include "renderer/renderer.h"
#include "solver/solver.h"
#include "renderer/renderer2.h"
#include "globals.h"
//#include "models/hostmodel.h"
#include "settings.h"
#include "msgcontroller.h"
#include "programmanager.h"

#ifdef VSTSDK
    #include "vst/cvsthost.h"
    #include "vst/vst3host.h"
#endif

#define MAX_NB_THREADS 500

class MainWindow;
//class ProgramsModel;
class MainHost : public QObject, public MsgController
{
Q_OBJECT
public:
    MainHost( Settings *settings, QObject *parent = 0);
    virtual ~MainHost();


//    void SendMsg(const ConnectionInfo &senderPin,const PinMessage::Enum msgType,void *data);

    void SetBufferSizeMs(unsigned int ms);
    void SetBufferSize(unsigned long size);
    void SetSampleRate(float rate=44100.0);
    unsigned long GetBufferSize() {return bufferSize;}
    float GetSampleRate() {return sampleRate;}

    void EnableSolverUpdate(bool enable);
//    bool IsSolverUpdateEnabled();

    void GetTempo(int &tempo, int &sign1, int &sign2);

#ifdef VSTSDK
    void SetTimeInfo(const VstTimeInfo *info);
#endif

//    void SetCurrentBuffers(float ** inputBuffer, float ** outputBuffer,unsigned long size) {
//        QMutexLocker l(&currentBuffersMutex);
//        currentInputBuffer = inputBuffer;
//        currentOutputBuffer = outputBuffer;
//        currentFramesPerBuffer = size;
//        inBufferReady = true;
//        outBufferReady = true;

//        if(currentFramesPerBuffer > bufferSize) {
//           SetBufferSize(currentFramesPerBuffer);
//        }
//    }

//    float ** TakeInputBuffer(unsigned long &size) {
//        QMutexLocker l(&currentBuffersMutex);
//        if(!inBufferReady)
//            return 0;
//        size = currentFramesPerBuffer;
//        inBufferReady = false;
//        return currentInputBuffer;
//    }
//    float ** TakeOutputBuffer(unsigned long &size) {
//        QMutexLocker l(&currentBuffersMutex);
//        if(!outBufferReady)
//            return 0;
//        size = currentFramesPerBuffer;
//        outBufferReady = false;
//        return currentOutputBuffer;
//    }

//    QMutex currentBuffersMutex;
//    float **currentInputBuffer;
//    float **currentOutputBuffer;
//    unsigned long currentFramesPerBuffer;
//    bool inBufferReady;
//    bool outBufferReady;

//    QStandardItemModel *GetRendererModel() { return renderer->GetModel(); }

    Renderer2 * GetRenderer() { return renderer; }

    void SetSetupDirtyFlag() { if(hostContainer) hostContainer->SetDirty(); }

    void ReceiveMsg(const MsgObject &msg);

    inline void UpdateSolverNow() {
        //if we need to update everything now, we have to ask for an update and force a render loop
        SetSolverUpdateNeeded();
        Render();
    }

    inline void SetSolverUpdateNeeded() {
        solverMutex.lock();
        solverNeedAnUpdate = true;
        solverMutex.unlock();
    }

//    void UpdateGlobalDelay(long samples)
//    {
//        emit DelayChanged(samples);
//    }

    inline bool undoProgramChanges() {return undoProgramChangesEnabled;}

    QSharedPointer<Connectables::Container> mainContainer;
    QSharedPointer<Connectables::Container> hostContainer;
    QSharedPointer<Connectables::Container> projectContainer;
    QSharedPointer<Connectables::Container> programContainer;
    QSharedPointer<Connectables::Container> groupContainer;

    QTimer *updateViewTimer;

//    HostModel * GetModel() {return model;}
    ProgramManager *programManager;
    Connectables::ObjectFactory *objFactory;
    MainWindow *mainWindow;

#ifdef VSTSDK
    vst::CVSTHost *vstHost;
    static int vstUsersCounter;
    Vst3Host *vst3Host;
#endif

    static quint32 currentFileVersion;
    bool doublePrecision;

#ifdef SCRIPTENGINE
    QScriptEngine *scriptEngine;
#endif

    QString currentProjectFile;
    QString currentSetupFile;

    QUndoStack undoStack;
    QMutex mutexRender;

    Settings *settings;

protected:
    void Close();
    QTime timeFromStart;
    float sampleRate;
    unsigned long bufferSize;

private:
    void SetupMainContainer();
    void SetupHostContainer();
    void SetupProjectContainer();
    void SetupProgramContainer();
    void SetupGroupContainer();

    void CheckTempo();

    void currentFileChanged();

    bool solverNeedAnUpdate;
    bool solverUpdateEnabled;

    QMap<int,Connectables::Object*>listContainers;
    QMap<ConnectionInfo,Connectables::Pin*>listPins;

//    hashCables workingListOfCables;
//    QMutex *mutexListCables;
    Renderer2 *renderer;

    QMutex solverMutex;

//    HostModel *model;

    int currentTempo;
    int currentTimeSig1;
    int currentTimeSig2;

    bool undoProgramChangesEnabled;

//    PathSolver *solver;
    Solver *solver;
    long globalDelay;

    int nbThreads;

    QTimer updateRendererViewTimer;

signals:
    void SampleRateChanged(float rate);
    void BufferSizeChanged(unsigned long size);
    void ObjectRemoved(int contrainerId, int obj);
    void SolverToUpdate();
    void Rendered();
//    void programParkingModelChanged(QStandardItemModel *model);
//    void groupParkingModelChanged(QStandardItemModel *model);
    void TempoChanged(int tempo=120, int sign1=4, int sign2=4);
    void DelayChanged(long samples);

public slots:
    virtual void Init();
    void Open();
    virtual void Kill();
    void SetTempo(int tempo=120, int sign1=0, int sign2=0);
    virtual void Render();
    void LoadFile(const QString &filename);
    void LoadSetupFile(const QString &filename = QString());
    void LoadProjectFile(const QString &filename = QString());
    void ReloadProject();
    void ReloadSetup();
    void ClearSetup();
    void ClearProject();
    bool SaveSetupFile(bool saveAs=false);
    bool SaveProjectFile(bool saveAs=false);
    void ChangeNbThreads(int nbTh=-1);
    void ResetDelays();
    void UpdateRendererMap();
    void UpdateRendererView();

private slots:
    void UpdateSolver(bool forceUpdate=false);

    friend class SetupFile;
    friend class ProjectFile;
};

#endif // MAINHOST_H

