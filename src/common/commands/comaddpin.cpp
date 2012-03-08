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

#include "comaddpin.h"
#include "mainhost.h"
#include "programmanager.h"

ComAddPin::ComAddPin(MainHost *myHost,
                     const ConnectionInfo &pinInfo,
                     QUndoCommand  *parent) :
    QUndoCommand(parent),
    myHost(myHost),
    pinInfo(pinInfo),
    currentGroup(0),
    currentProg(0)
{
    setText(QObject::tr("Add pin"));

    currentGroup = myHost->programManager->GetCurrentMidiGroup();
    currentProg =  myHost->programManager->GetCurrentMidiProg();
}

void ComAddPin::undo ()
{
    myHost->programManager->ChangeProgNow(currentGroup,currentProg);

    QSharedPointer<Connectables::Container>cntPtr = myHost->objFactory->GetObjectFromId( pinInfo.container ).staticCast<Connectables::Container>();
    if(!cntPtr)
        return;

    QSharedPointer<Connectables::Object>objPtr = myHost->objFactory->GetObjectFromId( pinInfo.objId );
    if(!objPtr)
        return;

    Connectables::ParameterPin *pin = static_cast<Connectables::ParameterPin*>(myHost->objFactory->GetPin(pinInfo));
    if(pin) {
        value=pin->GetVariantValue();
    }

    cntPtr->GetListOfConnectedPinsTo(pinInfo,listConnectedPins);
    objPtr->UserRemovePin(pinInfo);
}

void ComAddPin::redo ()
{
    myHost->programManager->ChangeProgNow(currentGroup,currentProg);

    QSharedPointer<Connectables::Container>cntPtr = myHost->objFactory->GetObjectFromId( pinInfo.container ).staticCast<Connectables::Container>();
    if(!cntPtr)
        return;

    QSharedPointer<Connectables::Object>objPtr = myHost->objFactory->GetObjectFromId( pinInfo.objId );
    if(!objPtr)
        return;

    objPtr->UserAddPin(pinInfo);

    foreach( ConnectionInfo info, listConnectedPins) {
        if(pinInfo.direction==PinDirection::Output)
            cntPtr->UserAddCable(pinInfo,info);
        else
            cntPtr->UserAddCable(info,pinInfo);
    }

    if(value.isValid()) {
        Connectables::ParameterPin *pin = static_cast<Connectables::ParameterPin*>(myHost->objFactory->GetPin(pinInfo));
        if(pin) {
            pin->SetVariantValue(value);
        }
    }

    listConnectedPins.clear();
}
