#ifndef COMCHANGEPROGRAM_H
#define COMCHANGEPROGRAM_H

#include "precomp.h"
#include <QUndoCommand>

class ProgramsModel;
class ComChangeProgram : public QUndoCommand
{
public:
    ComChangeProgram(ProgramsModel *model,
                     int oldGroup,
                     int oldProg,
                     int newGroup,
                     int newProg,
                     QUndoCommand  *parent=0);
    void undo();
    void redo();

private:
    ProgramsModel *model;
    int oldGroup;
    int oldProg;
    int newGroup;
    int newProg;
};

#endif // COMCHANGEPROGRAM_H
