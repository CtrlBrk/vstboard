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

#include "comdisconnectpin.h"
#include "connectables/objectfactory.h"
#include "connectables/container.h"
#include "mainhost.h"
#include "programmanager.h"

ComDisconnectPin::ComDisconnectPin(MainHost *myHost,
                                   const ConnectionInfo &pinInfo,
                                   QUndoCommand  *parent) :
    QUndoCommand(parent),
    myHost(myHost),
    pinInfo(pinInfo),
    currentGroup(0),
    currentProg(0)
{
    setText(QObject::tr("Disconnect pin"));

    currentGroup = myHost->programManager->GetCurrentMidiGroup();
    currentProg =  myHost->programManager->GetCurrentMidiProg();
}

void ComDisconnectPin::undo ()
{
    myHost->programManager->ChangeProgNow(currentGroup,currentProg);

    QSharedPointer<Connectables::Container>cntPtr = myHost->objFactory->GetObjectFromId( pinInfo.container ).staticCast<Connectables::Container>();
    if(!cntPtr)
        return;

    foreach(ConnectionInfo info, listConnectedPins) {
        if(pinInfo.direction==PinDirection::Output) {
            cntPtr->UserAddCable(pinInfo, info);
        } else {
            cntPtr->UserAddCable(info, pinInfo);
        }
    }
}

void ComDisconnectPin::redo ()
{
    myHost->programManager->ChangeProgNow(currentGroup,currentProg);

    QSharedPointer<Connectables::Container>cntPtr = myHost->objFactory->GetObjectFromId( pinInfo.container ).staticCast<Connectables::Container>();
    if(!cntPtr)
        return;

    listConnectedPins.clear();
    cntPtr->GetListOfConnectedPinsTo(pinInfo, listConnectedPins);
    cntPtr->UserRemoveCableFromPin(pinInfo);
}
