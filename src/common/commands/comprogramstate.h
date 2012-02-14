#ifndef COMPROGRAMSTATE_H
#define COMPROGRAMSTATE_H

class MainHost;
class Group;
class ComProgramState : public QUndoCommand
{
public:
    ComProgramState(MainHost *myHost, const QList<Group> &lstGroup, QUndoCommand *parent=0);

    void undo();
    void redo();
private:
    MainHost *myHost;
    QList<Group> lstGroup;
};

#endif // COMPROGRAMSTATE_H
