/**************************************************************************
#    Copyright 2010-2012 Raphaël François
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

#ifndef MSGHANDLER_H
#define MSGHANDLER_H

#include "msgcontroller.h"

class MsgObject;
class MsgHandler
{
public:
    MsgHandler(MsgController *msgCtrl, int objId);
    virtual ~MsgHandler();
    virtual void GetInfos(MsgObject &) {}
    int GetIndex() const {return objId;}
    bool SetIndex(int id);
    virtual void ReceiveMsg(const MsgObject &) {}
    MsgController *msgCtrl;

    bool MsgEnabled() const { return msgEnabled;}
    virtual void SetMsgEnabled(bool enab) {msgEnabled=enab;}

private:
    bool msgEnabled;
    int objId;
};

#endif // MSGHANDLER_H
