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

#ifndef CONNECTABLEOBJECT_H
#define CONNECTABLEOBJECT_H

#include "precomp.h"

#include "audiopin.h"
#include "midipinin.h"
#include "midipinout.h"
#include "parameterpinin.h"
#include "parameterpinout.h"
#include "bridgepinin.h"
#include "bridgepinout.h"
#include "objectprogram.h"
#include "objectinfo.h"
#include "pinslist.h"
#include "msghandler.h"

class SolverNode;
class MainHost;

namespace Connectables {

    typedef QHash<int,ObjectProgram*> hashPrograms;

    class Object : public QObject, public MsgHandler
    {
    Q_OBJECT
    public:

        Object(MainHost *host, int index, const ObjectInfo &info);
        virtual ~Object();

        void setObjectName(const QString &name);

        /*!
          When saving a project the index of the object is saved,
            used when loading a project file to map the connections with the current index
          \return saved index
          */
        inline int GetSavedIndex() {return savedIndex;}

        /// Reset the savedIndex to the current index, when the file is loaded or before saving
        inline void ResetSavedIndex(int id=-2) {savedIndex=id;}

        /// \return the current ObjectInfo
        const ObjectInfo & info() const {return objInfo;}

        /*!
          A SolverNode is a temporarry object used by the solver to create a rendering order
          used by PathSolver
          \return the current SolverNode
          */
        SolverNode *GetSolverNode() const {return solverNode;}

        /*!
          Set the solver node
          used by PathSolver
          */
        void SetSolverNode(SolverNode *node) {solverNode=node;}

        /*!
          Get a list of bridge pins input, only user by Bridge and Container
          \return a PinsList
          */
        PinsList* GetListBridgePinIn() {return listBridgePinIn;}
        PinsList* GetListAudioPinIn() {return listAudioPinIn;}
        PinsList* GetListMidiPinIn() {return listMidiPinIn;}

        /*!
          Get a list of bridge pins ouput, only user by Bridge and Container
          \return a PinsList
          */
        PinsList* GetListBridgePinOut() {return listBridgePinOut;}
        PinsList* GetListAudioPinOut() {return listAudioPinOut;}
        PinsList* GetListMidiPinOut() {return listMidiPinOut;}

        inline bool GetSleep() const {
            QMutexLocker l(&mutexSleep);
            return sleep;
        }

        virtual void NewRenderLoop();

        /// Lock the object mutex
        inline void Lock() { objMutex.lock();}

        /// Unlock the object mutex
        inline void Unlock() { objMutex.unlock();}

        LearningMode::Enum GetLearningMode();

        /*!
          Get the current container id
          \return container id
          */
        int GetContainerId() const {return containerId;}

        /*!
          Get the current program id
          \return program id
          */
        int GetCurrentProgId() {return currentProgId;}

        /// \return a pointer to the MainHost
        inline MainHost *getHost() {return myHost;}


        virtual bool Open();
        virtual bool Close();
//        virtual void Hide();
        virtual Pin * GetPin(const ConnectionInfo &pinInfo);
        virtual void SetSleep(bool sleeping);
        virtual QDataStream & toStream (QDataStream &) const;
        virtual bool fromStream (QDataStream &);
        virtual void SetContainerId(quint16 id);
//        virtual void SetBridgePinsInVisible(bool visible);
//        virtual void SetBridgePinsOutVisible(bool visible);
        virtual void RemoveProgram(int prg);
        virtual void SetContainerAttribs(const ObjectContainerAttribs &attr);
        virtual void GetContainerAttribs(ObjectContainerAttribs &attr);
        virtual void CopyStatusTo(QSharedPointer<Object>objPtr);
        virtual Pin* CreatePin(const ConnectionInfo &info);
        virtual bool IsDirty();

        /// Render the object, can be called multiple times if the rendering needs multiple passes
        virtual void Render() {}

        /// Called by the midi pins when a midi message is received
        virtual void MidiMsgFromInput(long /*msg*/) {}

        /*!
          Get the name of a parameter pin
          \param pinInfo
          \return the name
          */
        virtual QString GetParameterName(ConnectionInfo /*pinInfo*/) {return "";}

        /// the current model index
//        QPersistentModelIndex modelIndex;

        /// true if the object is parked (no rendering)
        quint16 parkingId;

        /// true if the object is rendered at double precision
        bool doublePrecision;

        /// true if the object is programmable
        /// \todo cleaup container and maincontainer and remove this
        bool listenProgramChanges;

        virtual void ProgramToStream (int progId, QDataStream &out);
        virtual void ProgramFromStream (int progId, QDataStream &in);

        long GetInitDelay() {return initialDelay;}

        virtual void GetInfos(MsgObject &msg);

        virtual void ReceiveMsg(const MsgObject &msg);
        virtual void SetMsgEnabled(bool enab);
    protected:

        void SetInitDelay(long d) {initialDelay=d;}

        /// pointer to the MainHost
        MainHost *myHost;

        /// list of all the pins
        QMap<QString, PinsList*>pinLists;

        /// list og audio pin in
        PinsList *listAudioPinIn;

        /// list of audio pin out
        PinsList *listAudioPinOut;

        /// list of midi pin in
        PinsList *listMidiPinIn;

        /// list of midi pin out
        PinsList *listMidiPinOut;

        /// list of bridge pin in
        /// \todo only used by bridges, can be removed ?
        PinsList *listBridgePinIn;

        /// list of bridge pin out
        /// \todo only used by bridges, can be removed ?
        PinsList *listBridgePinOut;

        /// list of parameters input
        PinsList *listParameterPinIn;

        /// list of parameters output
        PinsList *listParameterPinOut;

        /// list of values used by the editor pin (0 and 1)
        QList<QVariant>listEditorVisible;

        /// global object mutex
        QMutex objMutex;

        /// temporary SolverNode, used by PathSolver
        SolverNode *solverNode;

        /// list of programs
        hashPrograms listPrograms;

        /// the index the object had when the project was saved
        int savedIndex;

        /// pointer to the currently loaded program
        ObjectProgram *currentProgram;

        /// the current program is
        int currentProgId;

        /// true if the object is closed or is closing
        bool closed;

        /// ObjectInfo defining the object
        ObjectInfo objInfo;

        /// a string describing the error if the object can't be created
        QString errorMessage;

        ObjectContainerAttribs currentViewAttr;

    private:
        /// the current container id if not parked
        quint16 containerId;
        long initialDelay;
        QTimer updateViewDelay;

        /// true if sleeping
        mutable QMutex mutexSleep;
        bool sleep;

    signals:
        /// Sent to the editor window when we want to close it
        void HideEditorWindow();

    public slots:
        void SuspendIfParked() {if(parkingId!=FixedObjId::ND) SetSleep(true);}
        void Suspend() {SetSleep(true);}
        void Resume() {SetSleep(false);}
        virtual void SaveProgram();
        virtual void UnloadProgram();
        virtual void LoadProgram(int prog);
        void OnProgramDirty();

        /// set the buffer size
        virtual void SetBufferSize(unsigned long size);

        /// set the sampling rate
        virtual void SetSampleRate(float /*rate*/) {}

        virtual void OnParameterChanged(ConnectionInfo pinInfo, float value);

        bool ToggleEditor(bool visible);

        /// to show the editor window from another thread
        virtual void OnShowEditor() {}

        /// to hide the editor window from another thread
        virtual void OnHideEditor() {}

        virtual void UserRemovePin(const ConnectionInfo &info);
        virtual void UserAddPin(const ConnectionInfo &info);

        void SetErrorMessage(const QString &msg);
        bool IsInError() { return (objInfo.objType == ObjType::dummy || !errorMessage.isEmpty()); }
        void UpdateView();
        void UpdateViewNow();
    };
}

#endif // CONNECTABLEOBJECT_H
