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

#ifndef MSGOBJECT_H
#define MSGOBJECT_H

#include "precomp.h"
#include "connectables/objectinfo.h"

class MsgObject
{
public:
    enum Props {
        Name,
        Id,
        Add,
        Remove,
        Update,
        GetUpdate,
        Clear,
        Delay,
        Type,
        PinOut,
        PinIn,
        Load,
        Save,
        SaveAs,
        Project,
        Setup,
        Undo,
        Redo,
        Message,
        Answer,
        Container,
        Object,
        Direction,
        PinNumber,
        Bridge,
        IsRemoveable,
        ParentNodeType,
        ParentObjType,
        ObjInfo,
        ConnectPin,
        UnplugPin,
        RemovePin,
        ObjectsToLoad,
        FilesToLoad,
        Rescan,
        GroupAutosave,
        ProgAutosave,
        Program,
        Group,
        State,
        Increment,
        SolverMap,
        Row,
        Col,
        Value,
        Value1,
        Value2,
        Value3,
        Value4,
        Value5
    };

    MsgObject(int objIndex=FixedObjId::ND);
//    void SetProp(Props propId, const QVariant &value) { prop.insert(propId,value); }
//    const QVariant & GetProp(Props propId) { return prop.value(propId,0); }
//    bool PropExists(Props propId) { return prop.contains(propId); }

    int objIndex;
//    int parentIndex;
    QList<MsgObject>children;
    QMap<Props,QVariant>prop;


private:

    QDataStream & toStream (QDataStream &out) const;
    QDataStream & fromStream (QDataStream &in);
    friend QDataStream & operator<< (QDataStream&, const MsgObject&);
    friend QDataStream & operator>> (QDataStream&, MsgObject&);
};

QDataStream & operator<< (QDataStream& out, const MsgObject& value);
QDataStream & operator>> (QDataStream& in, MsgObject& value);

Q_DECLARE_METATYPE(MsgObject);
typedef QList<MsgObject> ListMsgObj;
Q_DECLARE_METATYPE(ListMsgObj);

#endif // MSGOBJECT_H
