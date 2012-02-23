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
        Value,
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
        Min,
        Max
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
