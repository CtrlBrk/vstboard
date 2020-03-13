/**************************************************************************
#    Copyright 2010-2012 RaphaÃ«l FranÃ§ois
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

#ifndef PINSLIST_H
#define PINSLIST_H

#include "precomp.h"
#include "globals.h"
#include "connectioninfo.h"
#include "objectinfo.h"
#include "audiobuffer.h"
#include "msghandler.h"

namespace Connectables {

    class Pin;
    class Object;
    class Container;

    class PinsList : public QObject, public MsgHandler
    {
        Q_OBJECT
        Q_PROPERTY(int nbPins READ nbPins WRITE SetNbPins)

    public:
        PinsList(MainHost *myHost, Object *parent, MsgController *msgCtrl, int objId);
        virtual ~PinsList();
        void SetContainerId(const quint16 id);
        void Hide();
//        void SetVisible(bool visible);
        void EnableVuUpdates(bool enab);
        void ChangeNumberOfPins(int newNbIn);
        void SetBridge(bool bridge);
        void SetInfo(Object *parent,const ConnectionInfo &connInfo, const ObjectInfo &objInfo);
        Pin * GetPin(int pinNumber, bool autoCreate=false);
        void ConnectAllTo(Container* container,PinsList *other, bool hidden=false);
        void AsyncAddPin(int nb);
        void AsyncRemovePin(int nb);

        int nbPins() const { return listPins.count(); }

        AudioBuffer *GetBuffer(int pinNumber);

        QMap<quint16,Pin*> listPins;
        ConnectionInfo connInfo;
        ObjectInfo objInfo;

        virtual QDataStream & toStream (QDataStream &) const;
        virtual QDataStream & fromStream (QDataStream &);

        void GetInfos(MsgObject &msg);

//        inline const int GetIndex() const {return index;}

        inline bool IsVisible() { return visible; }

        void ReceiveMsg(const MsgObject &msg);
        void SetMsgEnabled(bool enab);

    protected:
//        int index;
//        QPersistentModelIndex modelList;
        Object *parent;
        MainHost *myHost;
        bool visible;

    signals :
        void PinAdded(int nb);
        void PinRemoved(int nb);
        void NbPinChanged(int nb);

    public slots:
        void SetNbPins(int nb, QList<quint16> *listAdded=0,QList<quint16> *listRemoved=0);
        Pin * AddPin(int nb);
        void RemovePin(int nb);
    };
}

QDataStream & operator<< (QDataStream & out, const Connectables::PinsList& value);
QDataStream & operator>> (QDataStream & in, Connectables::PinsList& value);

#endif // PINSLIST_H
