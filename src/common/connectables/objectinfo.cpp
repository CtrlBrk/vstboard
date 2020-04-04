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
	id(0),
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

ObjectInfo::ObjectInfo(QJsonObject &json)
{
    nodeType = static_cast<NodeType::Enum>(json["nodeType"].toInt());
    objType = static_cast<ObjType::Enum>(json["objType"].toInt());
    id = json["id"].toDouble();
    name = json["name"].toString();
    filename = json["filename"].toString();
    inputs = json["inputs"].toInt();
    outputs = json["outputs"].toInt();
    duplicateNamesCounter = json["duplicateNamesCounter"].toInt();
    apiName = json["apiName"].toString();
    api = json["api"].toInt();
    forcedObjId = json["forcedObjId"].toInt();
    initDelay = json["initDelay"].toDouble();
}

void ObjectInfo::toJson(QJsonObject &json) const
{
    json["nodeType"] = nodeType;
    json["objType"] = objType;
    json["id"] = static_cast<double>(id);
    json["name"] = name;
    json["filename"] = filename;
    json["inputs"] = inputs;
    json["outputs"] = outputs;
    json["duplicateNamesCounter"] = duplicateNamesCounter;
    json["apiName"] = apiName;
    json["api"] = api;
    json["forcedObjId"] = forcedObjId;
    json["initDelay"] = static_cast<double>(initDelay);
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

