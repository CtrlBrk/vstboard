#include "comaddgroup.h"
#include "mainhost.h"

ComAddGroup::ComAddGroup(MainHost *myHost,
                         int prgId,
                         const QByteArray &data,
                         QUndoCommand *parent) :
    QUndoCommand(parent),
    myHost(myHost),
    prgId(prgId),
    data(data)
{
    setText(QObject::tr("Add group"));
}

void ComAddGroup::undo()
{
    QDataStream stream(&data, QIODevice::WriteOnly);
    myHost->groupContainer->ProgramToStream(prgId, stream);
    myHost->groupContainer->RemoveProgram(prgId);
}

void ComAddGroup::redo()
{
    QDataStream stream(&data, QIODevice::ReadOnly);
    myHost->groupContainer->ProgramFromStream(prgId, stream);
}
