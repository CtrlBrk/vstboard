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

#include "msgcontroller.h"
#include "msghandler.h"

MsgController::MsgController()
{
    lastFreeId = FixedObjId::dynamicIdStart;
}

MsgController::~MsgController()
{
    foreach(MsgHandler* h, listObj) {
        h->msgCtrl=0;
    }
}

qint32 MsgController::GetNewObjId() {
//    if(lastFreeId >= listObj.lastKey() ){
//        return ++lastFreeId;
//    }
//    hashMsgHandlers::const_iterator i = listObj.constBegin();
//    while (i != listObj.constEnd()) {
//        if(i.key() >= FixedObjId::dynamicIdStart) {
//            if(i.key() > lastFreeId)
//                return lastFreeId;
//            else
//                ++lastFreeId;
//        }
//        ++i;
//    }
//    return lastFreeId;
    lastFreeId = listObj.lastKey()+1;
    lastFreeId = std::max(lastFreeId,FixedObjId::dynamicIdStart);
    return lastFreeId;
}
