/**************************************************************************
#    Copyright 2010-2012 Raphaël François
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


#include "objectinfo.h"
#include "mainhost.h"

ObjectInfo::ObjectInfo() :
    nodeType(NodeType::ND),
    objType(ObjType::ND),
    id(0),
    name(""),
    filename(""),
    inputs(0),
    outputs(0),
    duplicateNamesCounter(0),
    api(0),
    forcedObjId(0),
    initDelay(0)
{

}

ObjectInfo::ObjectInfo(NodeType::Enum nodeType, ObjType::Enum objType, QString name) :
    nodeType(nodeType),
    objType(objType),
    name(name),
    filename(""),
    inputs(0),
    outputs(0),
    duplicateNamesCounter(0),
    api(0),
    forcedObjId(0),
    initDelay(0)
{

}

ObjectInfo::ObjectInfo(const ObjectInfo &c)
{
    *this = c;
}

QDataStream & ObjectInfo::toStream(QDataStream& stream) const
{
    stream << (quint8)nodeType;
    stream << (quint8)objType;
    stream << id;
    stream << name;
    stream << filename;
    stream << inputs;
    stream << outputs;
    stream << duplicateNamesCounter;
    stream << apiName;
    stream << api;
    stream << forcedObjId;
    return stream;
}

QDataStream & ObjectInfo::fromStream(QDataStream& stream)
{
    stream >> (quint8&)nodeType;
    stream >> (quint8&)objType;
    stream >> id;
    stream >> name;
    stream >> filename;
    stream >> inputs;
    stream >> outputs;
    stream >> duplicateNamesCounter;
    stream >> apiName;
    stream >> api;
    stream >> forcedObjId;
    return stream;
}

QDataStream & operator<< (QDataStream& stream, const ObjectInfo& objInfo)
{
   return objInfo.toStream(stream);
}

QDataStream & operator>> (QDataStream& stream, ObjectInfo& objInfo)
{
    return objInfo.fromStream(stream);
}

QDataStream & ObjectContainerAttribs::toStream (QDataStream& out) const
{
    out << position;
//    out << size;
    out << editorVisible;
    out << editorPosition;
    out << editorSize;
    out << editorHScroll;
    out << editorVScroll;

    return out;
}

QDataStream & ObjectContainerAttribs::fromStream (QDataStream& in)
{
    in >> position;
//    in >> size;
    in >> editorVisible;
    in >> editorPosition;
    in >> editorSize;
    in >> editorHScroll;
    in >> editorVScroll;
    return in;
}

QDataStream & operator<< (QDataStream & out, const ObjectContainerAttribs& value)
{
    return value.toStream(out);
}

QDataStream & operator>> (QDataStream & in, ObjectContainerAttribs& value)
{
    return value.fromStream(in);
}
