#ifndef COMADDOBJECT_H
#define COMADDOBJECT_H

#include "connectables/objects/container.h"

class MainHost;
class ComAddObject : public QUndoCommand
{
public:
    ComAddObject(MainHost *myHost,
                 const MetaInfo &objInfo,
                 const MetaInfo &targetInfo,
                 InsertionType::Enum insertType = InsertionType::NoInsertion,
                 QUndoCommand  *parent=0);
    void undo ();
    void redo ();

    void ReloadObject(const MetaInfo &info);

private:
    MainHost *myHost;

    MetaInfo objectInfo;
    MetaInfo targetInfo;

    InsertionType::Enum insertType;

    QList< QPair<MetaInfo,MetaInfo> >listAddedCables;
    QList< QPair<MetaInfo,MetaInfo> >listRemovedCables;

    ObjectContainerAttribs attr;
    QByteArray objState;

    ObjectContainerAttribs targetAttr;
    QByteArray targetState;

    int currentGroup;
    int currentProg;
};

#endif // COMADDOBJECT_H
