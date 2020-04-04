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

#ifndef OBJECTINFO_H
#define OBJECTINFO_H

#include "globals.h"

class ObjectInfo
{
public:
    ObjectInfo();
    ObjectInfo(NodeType::Enum nodeType, ObjType::Enum objType=ObjType::ND, QString name="");
    ObjectInfo(QJsonObject &json);
    void toJson(QJsonObject &json) const;
    QDataStream & toStream(QDataStream& stream) const;
    QDataStream & fromStream(QDataStream& stream);

    NodeType::Enum nodeType;
    ObjType::Enum objType;
    quint32 id;
    QString name;
    QString filename;
    quint16 inputs;
    quint16 outputs;
    quint16 duplicateNamesCounter;
    quint8 api;
    QString apiName;
    quint16 forcedObjId;
    long initDelay;
};

Q_DECLARE_METATYPE(ObjectInfo);

QDataStream & operator<< (QDataStream& stream, const ObjectInfo& objInfo);
QDataStream & operator>> (QDataStream& stream, ObjectInfo& objInfo);


#endif // OBJECTINFO_H
