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

#include "comaddcable.h"
#include "connectables/objectfactory.h"
#include "connectables/container.h"
#include "mainhost.h"
#include "programmanager.h"

ComAddCable::ComAddCable(MainHost *myHost,
                         const ConnectionInfo &outInfo,
                         const ConnectionInfo &inInfo,
                         QUndoCommand  *parent) :
    QUndoCommand(parent),
    myHost(myHost),
    outInfo(outInfo),
    inInfo(inInfo),
    currentGroup(0),
    currentProg(0)
{
    setText(QObject::tr("Add cable"));

    currentGroup = myHost->programManager->GetCurrentMidiGroup();
    currentProg =  myHost->programManager->GetCurrentMidiProg();

    if(outInfo.direction==PinDirection::Input) {
        ConnectionInfo tmp(outInfo);
        this->outInfo=ConnectionInfo(inInfo);
        this->inInfo=ConnectionInfo(tmp);
    }
}

void ComAddCable::undo ()
{
    myHost->programManager->ChangeProgNow(currentGroup,currentProg);

    QSharedPointer<Connectables::Container>cntPtr = myHost->objFactory->GetObjectFromId( inInfo.container ).staticCast<Connectables::Container>();
    if(!cntPtr)
        return;
    static_cast<Connectables::Container*>(cntPtr.data())->UserRemoveCable(outInfo,inInfo);
}

void ComAddCable::redo ()
{
    myHost->programManager->ChangeProgNow(currentGroup,currentProg);

    QSharedPointer<Connectables::Container>cntPtr = myHost->objFactory->GetObjectFromId( inInfo.container ).staticCast<Connectables::Container>();
    if(!cntPtr)
        return;
    static_cast<Connectables::Container*>(cntPtr.data())->UserAddCable(outInfo,inInfo);
}
