#ifndef COMADDGROUP_H
#define COMADDGROUP_H

class MainHost;
class ComAddGroup : public QUndoCommand
{
public:
    ComAddGroup(MainHost *myHost,
                int prgId,
                const QByteArray &data,
                QUndoCommand *parent=0);
    void undo();
    void redo();

private:
    MainHost *myHost;
    int prgId;
    QByteArray data;
};

#endif // COMADDGROUP_H
