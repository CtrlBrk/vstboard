/**************************************************************************
#    Copyright 2010-2025 Raphaël François
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

#ifndef OBJECTFACTORY_H
#define OBJECTFACTORY_H

//#include "../precomp.h"
#include "../globals.h"
#include "object.h"
#include "objectinfo.h"
class MainHost;

namespace Connectables {

    class ObjectFactory : public QObject
    {
    Q_OBJECT
    public:
        ObjectFactory(MainHost *myHost);
        ~ObjectFactory();

        void Close();

        QSharedPointer<Object> NewObject(const ObjectInfo &info, int containerId=0);
        QSharedPointer<Object> GetObj(const QModelIndex & index);
        inline void RemoveObject(int id) {
            listObjects.remove(id);
        }

        QSharedPointer<Object> GetObjectFromId(int id);
        int IdFromSavedId(int savedId);
        void ResetAllSavedId();
        Pin *GetPin(const ConnectionInfo &pinInfo);
        inline Pin *GetPin(const QModelIndex & index) {
            return GetPin( index.data(UserRoles::connectionInfo).value<ConnectionInfo>() );
        }

        const hashObjects &GetListObjects() {return listObjects;}

        QList<int>listDelayObj;

    protected:
        virtual Object *CreateOtherObjects(const ObjectInfo &info, int objId)=0;
        WORD Check32bits(const ObjectInfo &info);
        hashObjects listObjects;
        MainHost *myHost;

    };

}

#endif // OBJECTFACTORY_H
