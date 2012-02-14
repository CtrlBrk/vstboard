#include "comprogramstate.h"
#include "mainhost.h"

ComProgramState::ComProgramState(MainHost *myHost,
                                 const QList<Group> &lstGroup,
                                 QUndoCommand *parent) :
    QUndoCommand(parent),
    lstGroup(lstGroup),
    myHost(myHost)
{
    setText(QObject::tr("ProgramList"));
}

void ComProgramState::undo()
{
    redo();
}

void ComProgramState::redo()
{
    QList<Group> tmpLst;
    tmpLst = myHost->programManager->GetListGroups();
    myHost->programManager->SetListGroups(lstGroup);
    lstGroup=tmpLst;
}
