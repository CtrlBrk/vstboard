/**************************************************************************
#    Copyright 2010-2020 Raphaël François
#    Contact : ctrlbrk76@gmail.com
#
#    This file is part of VstBoard.
#
#    VstBoard is free software: you can redistribute it and/or modify
#    it under the terms of the under the terms of the GNU Lesser General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    VstBoard is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    under the terms of the GNU Lesser General Public License for more details.
#
#    You should have received a copy of the under the terms of the GNU Lesser General Public License
#    along with VstBoard.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#include "msghandler.h"
#include "msgobject.h"

MsgHandler::MsgHandler(MsgController *msgCtrl, qint32 objId) :
    msgCtrl(msgCtrl),
    msgEnabled(false),
    objId(objId)
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
    if(msgCtrl && objId!=-1) {
        int n = msgCtrl->listObj.remove(objId);
//        if(objId < msgCtrl->lastFreeId)
//            msgCtrl->lastFreeId=objId;

        if(n!=1) {
            LOG("nb removed handlers " << n)
        }
    }
}

bool MsgHandler::SetIndex(qint32 id)
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
