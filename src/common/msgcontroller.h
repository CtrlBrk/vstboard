/**************************************************************************
#    Copyright 2010-2025 Raphaël François
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

#ifndef MSGCONTROLLER_H
#define MSGCONTROLLER_H

//#include "precomp.h"
#include "msgobject.h"

class MsgHandler;

//typedef QHash<int,MsgHandler*> hashMsgHandlers;
typedef QMap<qint32,MsgHandler*> hashMsgHandlers;

class MsgController
{
public:
    MsgController();
    ~MsgController();
    virtual void SendMsg(const MsgObject & /*msg*/) {}
    virtual void ReceiveMsg(const MsgObject & /*msg*/) {}
    qint32 GetNewObjId();
    hashMsgHandlers listObj;
    qint32 lastFreeId;
};

#endif // MSGCONTROLLER_H
