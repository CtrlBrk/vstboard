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


#include "connectioninfo.h"
#include "object.h"
#include "objectfactory.h"
#include "mainhost.h"

/*!
  \class ConnectionInfo
  \brief describe a pin
  */

/*!
  default constructor
  */
ConnectionInfo::ConnectionInfo()
    : container(0), objId(0), type(PinType::Audio), direction(PinDirection::Output), pinNumber(0), bridge(false), isRemoveable(false), myHost(0)
{
}

/*!
  \param myHost pointer to the MainHost
  \param objId id of the parent object
  \param type the pin type
  \param direction the pin direction
  \param pinNumber the pin number in a list of pins
  \param bridge true if it's a bridge pin (can connect with the pins of the parent container)
  */
ConnectionInfo::ConnectionInfo(MainHost *myHost,quint16 objId, PinType::Enum type, PinDirection::Enum direction, quint16 pinNumber, bool bridge, bool removeable) :
    container(-1),
    objId(objId),
    type(type),
    direction(direction),
    pinNumber(pinNumber),
    bridge(bridge),
    isRemoveable(removeable),
    myHost(myHost)
{
}

/*!
  Check if the provided pin can connect to this one
  \param c the other pin
  \return true if it can connect
  */
bool ConnectionInfo::CanConnectTo(const ConnectionInfo &c) const {

    //don't connect object to itself
//    if(objId == c.objId)
//        return false;

    //must be the same type (audio/midi/automation) or a bridge pin
    if(type!=PinType::Bridge && c.type!=PinType::Bridge && type != c.type)
        return false;


    //must be opposite directions
    if(direction == c.direction)
        return false;

    int cntA = container;
    int cntB = c.container;

    //if it's a bridge : get the container's container id
    if(bridge)
        cntA = myHost->objFactory->GetObjectFromId( container )->GetContainerId();

    if(c.bridge)
        cntB = myHost->objFactory->GetObjectFromId( c.container )->GetContainerId();

    //must be in the same container
    if(cntA == cntB)
        return true;

    return false;
}

/*!
  Out the object in a data stream
  \param out the stream
  \return the stream
  */
QDataStream & ConnectionInfo::toStream(QDataStream& out) const
{
    out << bridge;
    out << container;
    out << objId;
    out << (quint8)type;
    out << (quint8)direction;
    out << pinNumber;
    return out;
}

/*!
  Read from a data stream
  \param in the stream
  \return the stream
  */
QDataStream & ConnectionInfo::fromStream(QDataStream& in)
{
    in >> bridge;
    quint16 savedContainerId;
    in >> savedContainerId;
    int id = myHost->objFactory->IdFromSavedId(savedContainerId);
    if(id==-1) {
        LOG("container not found");
        return in;
    }
    container = id;

    quint16 savedId;
    in >> savedId;
    id = myHost->objFactory->IdFromSavedId(savedId);
    if(id==-1) {
        LOG("obj not found");
        return in;
    }
    objId = id;

    in >> (quint8&)type;
    in >> (quint8&)direction;
    in >> pinNumber;

    return in;
}

/*!
  overload datastream for ConnectionInfo
  */
QDataStream & operator<< (QDataStream& out, const ConnectionInfo& connInfo)
{
   return connInfo.toStream(out);
}

/*!
  overload datastream for ConnectionInfo
  */
QDataStream & operator>> (QDataStream& in, ConnectionInfo& connInfo)
{
    return connInfo.fromStream(in);
}

ConnectionInfo::ConnectionInfo(const MsgObject &msg) :
    myHost(0)
{
    container = msg.prop[MsgObject::Container].toInt();
    objId = msg.prop[MsgObject::Object].toInt();
    type = static_cast<PinType::Enum>(msg.prop[MsgObject::Type].toInt());
    direction = static_cast<PinDirection::Enum>(msg.prop[MsgObject::Direction].toInt());
    pinNumber = msg.prop[MsgObject::PinNumber].toInt();
    bridge = msg.prop[MsgObject::Bridge].toBool();
    isRemoveable = msg.prop[MsgObject::IsRemoveable].toBool();
}

void ConnectionInfo::GetInfos(MsgObject &msg)
{
    msg.prop[MsgObject::Container]=container;
    msg.prop[MsgObject::Object]=objId;
    msg.prop[MsgObject::Type]=type;
    msg.prop[MsgObject::Direction]=direction;
    msg.prop[MsgObject::PinNumber]=pinNumber;
    msg.prop[MsgObject::Bridge]=bridge;
    msg.prop[MsgObject::IsRemoveable]=isRemoveable;
}
