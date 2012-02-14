#include "comaddprogram.h"
#include "mainhost.h"

ComAddProgram::ComAddProgram(MainHost *myHost,
                             int prgId,
                             const QByteArray &data,
                             QUndoCommand *parent) :
    QUndoCommand(parent),
    myHost(myHost),
    prgId(prgId),
    data(data)

{
    setText(QObject::tr("Add program"));
}

void ComAddProgram::undo()
{
    QDataStream stream(&data, QIODevice::WriteOnly);
    myHost->programContainer->ProgramToStream(prgId, stream);
    myHost->programContainer->RemoveProgram(prgId);
}

void ComAddProgram::redo()
{
    QDataStream stream(&data, QIODevice::ReadOnly);
    myHost->programContainer->ProgramFromStream(prgId, stream);
}
