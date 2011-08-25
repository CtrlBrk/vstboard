#ifndef COMCONNECTPIN_H
#define COMCONNECTPIN_H

#include "objectinfo.h"

class MainHost;
class ComAddCable : public QUndoCommand
{
public:
    ComAddCable(MainHost *myHost,
                const MetaInfo &outInfo,
                const MetaInfo &inInfo,
                QUndoCommand  *parent=0);
    void undo ();
    void redo ();

private:
    MainHost *myHost;
    MetaInfo outInfo;
    MetaInfo inInfo;
    int currentGroup;
    int currentProg;
};

#endif // COMCONNECTPIN_H
