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

#include "msgobject.h"
#include "connectables/connectioninfo.h"

MsgObject::MsgObject(int objIndex) :
    objIndex(objIndex)
{

}

QDataStream & MsgObject::toStream (QDataStream &out) const
{
    out << (qint32)objIndex;
//    out << (qint32)parentIndex;

    out << (qint32)prop.count();
    QMap<Props,QVariant>::const_iterator i = prop.constBegin();
    while(i!=prop.constEnd()) {
        QString s(i.key());
        out << static_cast<int>(i.key());
        out << i.value();
        ++i;
    }

    out << (qint32)children.count();
    foreach(const MsgObject &o, children) {
        out << o;
    }

    return out;
}

QDataStream & MsgObject::fromStream (QDataStream &in)
{
    in >> (qint32)objIndex;
//    in >> (qint32)parentIndex;
    qint32 c;
    in >> c;
    for(qint32 i=0; i<c; i++) {
        int k;
        QVariant v;
        in >> k;
        in >> v;
        prop.insert(static_cast<Props>(k),v);
    }

    in >> c;
    for(qint32 i=0; i<c; i++) {
        MsgObject msg;
        in >> msg;
        children << msg;
    }
    return in;
}

QDataStream & operator<< (QDataStream& out, const MsgObject& value)
{
    return value.toStream(out);
}

QDataStream & operator>> (QDataStream& in, MsgObject& value)
{
    return value.fromStream(in);
}
