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
    virtual void SendMsg(const MsgObject & /*msg*/) {}
    virtual void ReceiveMsg(const MsgObject & /*msg*/) {}

    QHash<int,MsgHandler*>listObj;
};

#endif // MSGCONTROLLER_H
