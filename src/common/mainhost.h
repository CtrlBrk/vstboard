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

#ifndef MAINHOST_H
#define MAINHOST_H

#include "connectables/objectfactory.h"
#include "connectables/object.h"
#include "connectables/container.h"
#include "renderer/solver.h"
#include "renderer/renderer2.h"
#include "globals.h"
#include "settings.h"
#include "msgcontroller.h"
#include "programmanager.h"

#ifdef VST24SDK
    #include "vst/cvsthost.h"
#endif
#ifdef VSTSDK
    #include "vst/vst3host.h"
#endif

#define MAX_NB_THREADS 500

class MainWindow;
class MainHost : public QObject, public MsgController
{
Q_OBJECT
public:
    MainHost( Settings *settings, QObject *parent = 0);
    virtual ~MainHost();

    void SetBufferSizeMs(qint32 ms);
    void SetBufferSize(qint32 size);
    void SetSampleRate(float rate=44100.0);
    qint32 GetBufferSize() {return bufferSize;}
    float GetSampleRate() {return sampleRate;}

    void EnableSolverUpdate(bool enable);
    void GetTempo(int &tempo, int &sign1, int &sign2);

    void Open();

#ifdef VSTSDK
    #ifdef VST24SDK
        void SetTimeInfo(const VstTimeInfo *info);
    #endif
#endif

    Renderer2 * GetRenderer() { return renderer; }

    void SetSetupDirtyFlag() { if(hostContainer) hostContainer->SetDirty(); }

    void ReceiveMsg(const MsgObject &msg);
/*
    inline void UpdateSolverNow() {
        //if we need to update everything now, we have to ask for an update and force a render loop
        SetSolverUpdateNeeded();
        Render();
    }
*/
    inline void SetSolverUpdateNeeded() {
        solverMutex.lock();
        solverNeedAnUpdate = true;
        solverMutex.unlock();
    }


    inline bool undoProgramChanges() {return undoProgramChangesEnabled;}

    QSharedPointer<Connectables::Container> mainContainer;
    QSharedPointer<Connectables::Container> hostContainer;
    QSharedPointer<Connectables::Container> projectContainer;
    QSharedPointer<Connectables::Container> programContainer;
    QSharedPointer<Connectables::Container> groupContainer;

    QTimer *updateViewTimer;
    ProgramManager *programManager;
    Connectables::ObjectFactory *objFactory;
    MainWindow *mainWindow;

#ifdef VSTSDK
    #ifdef VST24SDK
        vst::CVSTHost *vstHost;
    #endif
    static int vstUsersCounter;
    Vst3Host *vst3Host;
#endif

    static quint32 currentFileVersion;
    bool doublePrecision;

#ifdef SCRIPTENGINE
#ifdef OLDSCRIPTENGINE
    QScriptEngine scriptEngine;
#endif
#endif

    QString currentProjectFile;
    QString currentSetupFile;

    QUndoStack undoStack;

    Settings *settings;

    void GetRenderMap(RenderMap &map);
    void SetRenderMap(const RenderMap &map);

protected:
    void Close();
    QElapsedTimer timeFromStart;
    Renderer2 *renderer;

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
    QMutex solverMutex;

    float sampleRate;
    qint32 bufferSize;
    int currentTempo;
    int currentTimeSig1;
    int currentTimeSig2;

    bool undoProgramChangesEnabled;

    Solver *solver;
    long globalDelay;

    int nbThreads;

    QTimer updateRendererViewTimer;

signals:
    void SampleRateChanged(float rate);
    void BufferSizeChanged(qint32 size);
    void ObjectRemoved(int contrainerId, int obj);
    void SolverToUpdate();
    void Rendered();
    void TempoChanged(int tempo=120, int sign1=4, int sign2=4);
    void DelayChanged(long samples);

public slots:
    virtual void Init();
    virtual void Kill();
    void SetTempo(int tempo=120, int sign1=0, int sign2=0);
    virtual void Render();
//    bool LoadFile(const QString &filename);
    void LoadDefaultFiles();
    bool LoadSetupFile(const QString &filename = QString());
    bool LoadProjectFile(const QString &filename = QString());
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
    void UpdateView();
    void OnRenderTimeout();

private slots:
    void UpdateSolver(bool forceUpdate=false);

    friend class SetupFile;
    friend class ProjectFile;
    friend class JsonReader;
};

#endif // MAINHOST_H

