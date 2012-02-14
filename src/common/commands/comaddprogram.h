#ifndef COMADDPROGRAM_H
#define COMADDPROGRAM_H

class MainHost;
class ComAddProgram : public QUndoCommand
{
public:
    ComAddProgram(MainHost *myHost,
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

#endif // COMADDPROGRAM_H
