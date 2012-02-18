#include "msghandler.h"
#include "msgobject.h"

MsgHandler::MsgHandler(MsgController *msgCtrl, int objId) :
    msgCtrl(msgCtrl),
    objId(objId),
    msgEnabled(false)
{
    if(objId==-1 || !msgCtrl)
        return;
    if(msgCtrl->listObj.contains(objId)) {
        LOG("objId already exists"<<objId)
        return;
    }
    msgCtrl->listObj[objId]=this;
}

MsgHandler::~MsgHandler()
{
    if(msgCtrl)
        msgCtrl->listObj.remove(objId);
}

bool MsgHandler::SetIndex(int id)
{
    if(!msgCtrl)
        return true;

    if(objId==id)
        return true;

    if(objId!=-1) {
        LOG("reset id not allowed")
        return false;
    }

    if(msgCtrl->listObj.contains(id)) {
        LOG("objId already exists"<<objId)
        return false;
    }
    objId=id;
    msgCtrl->listObj[objId]=this;
    return true;
}
