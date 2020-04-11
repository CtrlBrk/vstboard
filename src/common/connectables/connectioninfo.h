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

#ifndef CONNECTIONINFO_H
#define CONNECTIONINFO_H

#include "globals.h"
#include "msgobject.h"

class MainHost;
class ConnectionInfo
{
public:
    ConnectionInfo();
    ConnectionInfo(MainHost *myHost,quint16 objId, PinType::Enum type, PinDirection::Enum direction, quint16 pinNumber, bool bridge, bool removeable=false);
    ConnectionInfo(MainHost *host,const QJsonObject &json );
    ConnectionInfo(const MsgObject &msg);

    bool CanConnectTo(const ConnectionInfo &c) const;

    void toJson(QJsonObject &json) const;
    QDataStream & toStream(QDataStream& out) const;
    QDataStream & fromStream(QDataStream& in);

    void GetInfos(MsgObject &msg);

    /// the Connectables::Container id (can connect to other pins in the same container)
    quint16 container;

    /// the parent Connectables::Object id
    quint16 objId;

    /// pin type
    PinType::Enum type;

    /// pin direction
    PinDirection::Enum direction;

    /// pin number in a list
    quint16 pinNumber;

    /// true if it's a bridge
    bool bridge;

    /// true if the user can remove the pin
    bool isRemoveable;

    /// pointer to the MainHost
    MainHost *myHost;
};

Q_DECLARE_METATYPE(ConnectionInfo);

/*!
  overload == for ConnectionInfo
  */
inline bool operator==(const ConnectionInfo &c1, const ConnectionInfo &c2)
{
    return c1.objId == c2.objId
        && c1.type == c2.type
        && c1.direction == c2.direction
        && c1.pinNumber == c2.pinNumber;
}

/*!
  overload < for ConnectionInfo
  */
inline bool operator<(const ConnectionInfo &c1, const ConnectionInfo &c2)
{
    if(c1.container < c2.container)
        return true;
    if(c1.container > c2.container)
        return false;

    if(c1.bridge < c2.bridge)
        return true;
    if(c1.bridge > c2.bridge)
        return false;

    if(c1.objId < c2.objId)
        return true;
    if(c1.objId > c2.objId)
        return false;

    if(c1.pinNumber < c2.pinNumber)
        return true;
    if(c1.pinNumber > c2.pinNumber)
        return false;

    if(c1.type < c2.type)
        return true;
    if(c1.type > c2.type)
        return false;

    if(c1.direction < c2.direction)
        return true;

    return false;
}


QDataStream & operator<< (QDataStream& out, const ConnectionInfo& connInfo);
QDataStream & operator>> (QDataStream& in, ConnectionInfo& connInfo);

#endif // CONNECTIONINFO_H
