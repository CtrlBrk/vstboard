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

#ifndef CONNECTABLEPIN_H
#define CONNECTABLEPIN_H

//#include "../precomp.h"
#include "../globals.h"
#include "connectioninfo.h"
#include "msghandler.h"
#include "pinslist.h"


namespace Connectables {
	class Object;
	class Cable;

	class pinConstructArgs : public ConnectionInfo {
	public:
        Object *parent = 0;
        float value = .0f;
        float stepSize = .0f;
        bool visible = true;
        PinsList *pinList = 0;
        QVariantList *listValues = 0;
        QVariant defaultVariantValue = 0;
//		float defaultValue = .0f;
        bool nameCanChange = false;
        QString name = "";
        bool doublePrecision = false;
        bool externalAllocation = false;
        unsigned long bufferSize = 0;

        pinConstructArgs() {};

        pinConstructArgs(const ConnectionInfo &i) {
			container = i.container;
			objId = i.objId;
			type = i.type;
			direction = i.direction;
			pinNumber = i.pinNumber;
			bridge = i.bridge;
			isRemoveable = i.isRemoveable;
			myHost = i.myHost;
		};

	};

    
    class Pin : public QObject, public MsgHandler
    {
    Q_OBJECT
        Q_PROPERTY(float value READ GetValNoReset)
    public:
		Pin(const pinConstructArgs &conf);
        virtual ~Pin();

        virtual void GetInfos(MsgObject &msg);

        virtual void SendMsg(const PinMessage::Enum msgType,void *data=0);

        /*!
         Message received from another pin
         \param msgType PinMessage
         \param data received data
         */
        virtual void ReceivePinMsg(const PinMessage::Enum /*msgType*/,void * /*data*/) {}

        /*!
          Get the current value (vu meter or parameter value)
          \return current value
          */
        virtual float GetValue() {return value;}

        /*!
          Get the current value, don't reset vumeter counter
          */
        float GetValNoReset() {return value;}
        /*!
          get the pin infos
          \return ConnectionInfo
          */
        const ConnectionInfo &GetConnectionInfo() const {return connectInfo;}

        void SetPinList(PinsList *lst);
//        virtual void SetParentModelIndex(PinsList *list, const QModelIndex &newParent);
        void SetContainerId(qint32 id);

        /// \return true if visible
        inline bool GetVisible() {return visible;}
        virtual void SetVisible(bool vis);
        void EnableVuUpdates(bool enab);
        void SetBridge(bool bridge);
        void Close();

        /*!
          Prepare for a new rendering
          Called one time at the beginning of the loop
           */
        virtual void NewRenderLoop() {}

        void AddCable(const QWeakPointer<Cable>&c) {cablesMutex.lock(); listCables << c; cablesMutex.unlock();}
        void RemoveCable(const QWeakPointer<Cable>&c) {cablesMutex.lock(); listCables.removeAll(c); cablesMutex.unlock();}

    protected:

        QMutex cablesMutex;
        QList<QWeakPointer<Cable> >listCables;

        /// ConnectionInfo og the pin
        ConnectionInfo connectInfo;

        /// current value
        float value;

        /// size of each steps (=1/nbvalues)
        float stepSize;

        /// pointer to the parent Object
        Object *parent;

        /// is displayed or not
        bool visible;

        /// index of the parent in the model
//        QPersistentModelIndex parentIndex;

        /// index of this pin in the model
//        QPersistentModelIndex modelIndex;

        /// global pin mutex
        QMutex objMutex;

        /// true if closed or closing (no send, no receive)
        bool closed;

        /// text to displaye
        QString displayedText;

        /// true if the value changes since the laste view update
        bool valueChanged;

        PinsList *pinList;

    public slots:
        virtual void updateView();

    };
}

#endif // CONNECTABLEPIN_H
