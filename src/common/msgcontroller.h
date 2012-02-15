#ifndef MSGCONTROLLER_H
#define MSGCONTROLLER_H

#include "precomp.h"
#include "msgobject.h"

class MsgHandler;
class MsgController
{
public:
    MsgController();
    ~MsgController();
    virtual void SendMsg(const MsgObject &msg)=0;
    virtual void ReceiveMsg(const MsgObject &msg)=0;

    QHash<int,MsgHandler*>listObj;
};

#endif // MSGCONTROLLER_H
