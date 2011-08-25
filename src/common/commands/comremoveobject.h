#ifndef COMREMOVEOBJECT_H
#define COMREMOVEOBJECT_H

#include "precomp.h"
#include "connectables/objects/container.h"
#include "threadsafecom.h"

class MainHost;
class ComRemoveObject : public ThreadSafeCom
{
public:
    ComRemoveObject( MainHost *myHost,
                     const MetaInfo &objectInfo,
                     RemoveType::Enum removeType,
                     QUndoCommand *parent=0);

private:
    Q_INVOKABLE void tUndo();
    Q_INVOKABLE void tRedo();

    MetaInfo objectInfo;
    RemoveType::Enum removeType;
    ObjectContainerAttribs attr;

    QList< QPair<MetaInfo,MetaInfo> >listAddedCables;
    QList< QPair<MetaInfo,MetaInfo> >listRemovedCables;

    QByteArray objState;

    int currentGroup;
    int currentProg;

    int progId;
};

#endif // COMREMOVEOBJECT_H
