#ifndef OBJECTCONTAINERATTRIBS_H
#define OBJECTCONTAINERATTRIBS_H


class ObjectContainerAttribs
{
public:
    ObjectContainerAttribs() :
        position(QPointF(0.0f,0.0f)),
        editorVisible(false),
        editorPosition(QPoint(0.0f,0.0f)),
        editorSize(QSize(0,0)),
        editorHScroll(0),
        editorVScroll(0)
        {}

    ObjectContainerAttribs(QJsonObject &json, int &id);

    void toJson(QJsonObject &json, int id) const;

    QPointF position;
    bool editorVisible;
    QPoint editorPosition;
    QSize editorSize;
    quint16 editorHScroll;
    quint16 editorVScroll;

    QDataStream & toStream (QDataStream &) const;
    QDataStream & fromStream (QDataStream &);
};

Q_DECLARE_METATYPE(ObjectContainerAttribs);

QDataStream & operator<< (QDataStream & out, const ObjectContainerAttribs& value);
QDataStream & operator>> (QDataStream & in, ObjectContainerAttribs& value);

inline bool operator==(const ObjectContainerAttribs &c1, const ObjectContainerAttribs &c2)
{
    if(c1.position != c2.position)
        return false;
    if(c1.editorVisible != c2.editorVisible)
        return false;
    if(c1.editorPosition != c2.editorPosition)
        return false;
    if(c1.editorSize != c2.editorSize)
        return false;
    if(c1.editorHScroll != c2.editorHScroll)
        return false;
    if(c1.editorVScroll != c2.editorVScroll)
        return false;

    return true;
}

inline bool operator!=(const ObjectContainerAttribs &c1, const ObjectContainerAttribs &c2)
{
    return !(c1==c2);
}

#endif // OBJECTCONTAINERATTRIBS_H
