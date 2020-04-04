#include "objectcontainerattribs.h"

ObjectContainerAttribs::ObjectContainerAttribs(QJsonObject &json, int &id)
{
    id = json["id"].toInt();
    position.setX( json["x"].toInt() );
    position.setY( json["y"].toInt() );

    QJsonObject jEdtr = json["editor"].toObject();
    editorVisible = jEdtr["visible"].toBool();
    editorPosition.setX( jEdtr["x"].toInt() );
    editorPosition.setY( jEdtr["y"].toInt() );
    editorSize.setWidth( jEdtr["width"].toInt() );
    editorSize.setHeight( jEdtr["height"].toInt() );
    editorHScroll = jEdtr["hscroll"].toInt();
    editorVScroll = jEdtr["vscroll"].toInt();
}

void ObjectContainerAttribs::toJson(QJsonObject &json, int id) const
{
    json["id"] = id;
    json["x"] = position.x();
    json["y"] = position.y();

    QJsonObject jEdtr;
    jEdtr["visible"] = editorVisible;
    jEdtr["x"] = editorPosition.x();
    jEdtr["y"] = editorPosition.y();
    jEdtr["width"] = editorSize.width();
    jEdtr["height"] = editorSize.height();
    jEdtr["hscroll"] = editorHScroll;
    jEdtr["vscroll"] = editorVScroll;

    json["editor"] = jEdtr;
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
