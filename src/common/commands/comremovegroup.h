#ifndef COMREMOVEGROUP_H
#define COMREMOVEGROUP_H

class MainHost;
class ComRemoveGroup : public QUndoCommand
{
public:
    ComRemoveGroup(MainHost *myHost,
                   int prgId,
                   QUndoCommand *parent=0);
    void undo();
    void redo();

private:
    MainHost *myHost;
    int prgId;
    QByteArray data;
};

#endif // COMREMOVEGROUP_H
