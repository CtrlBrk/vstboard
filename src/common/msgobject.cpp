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
