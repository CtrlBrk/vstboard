#include "comremovegroup.h"
#include "mainhost.h"

ComRemoveGroup::ComRemoveGroup(MainHost *myHost,
                               int prgId,
                               QUndoCommand *parent) :
    QUndoCommand(parent),
    myHost(myHost),
    prgId(prgId)
{
    setText(QObject::tr("Remove group"));
}

void ComRemoveGroup::undo()
{
    QDataStream stream(&data, QIODevice::ReadOnly);
    myHost->groupContainer->ProgramFromStream(prgId, stream);
}

void ComRemoveGroup::redo()
{
    QDataStream stream(&data, QIODevice::WriteOnly);
    myHost->groupContainer->ProgramToStream(prgId, stream);
    myHost->groupContainer->RemoveProgram(prgId);
}
