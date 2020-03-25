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

#include "comremoveobject.h"
#include "mainhost.h"
#include "connectables/container.h"
#include "programmanager.h"

ComRemoveObject::ComRemoveObject( MainHost *myHost,
                                  const QSharedPointer<Connectables::Object> &objPtr,
                                  RemoveType::Enum removeType,
                                  QUndoCommand  *parent) :
    QUndoCommand(parent),
    myHost(myHost),
    removeType(removeType),
    currentGroup(0),
    currentProg(0)
{
    ContainerPtr = myHost->objFactory->GetObjectFromId( objPtr->GetContainerId() ).staticCast<Connectables::Container>();
    if(!ContainerPtr)
        return;

    setText(QObject::tr("Remove %1").arg(objPtr->objectName()));

    objectInfo = objPtr->info();

    currentGroup = myHost->programManager->GetCurrentMidiGroup();
    currentProg =  myHost->programManager->GetCurrentMidiProg();
}

void ComRemoveObject::undo ()
{
    myHost->programManager->ChangeProgNow(currentGroup,currentProg);

    //get the container
    QSharedPointer<Connectables::Container> container = ContainerPtr.toStrongRef();
    if(!container)
        return;

    //get the object
    QSharedPointer<Connectables::Object> obj = myHost->objFactory->GetObjectFromId( objectInfo.forcedObjId );
    if(!obj) {
        //object was deleted, create a new one
        obj = myHost->objFactory->NewObject( objectInfo, container->GetIndex() );
    }
    if(!obj)
        return;

    objectInfo = obj->info();



    QDataStream stream(&objState, QIODevice::ReadWrite);
    obj->fromStream( stream );
    objState.resize(0);

    container->UserAddObject( obj );

    obj->SetContainerAttribs(attr);

    //remove cables added at creation
    QPair<ConnectionInfo,ConnectionInfo>pair;
    foreach( pair, listAddedCables) {
        container->UserRemoveCable(pair);
    }
    //add cables removed at creation
    foreach( pair, listRemovedCables) {
        container->UserAddCable(pair);
    }

    listAddedCables.clear();
    listRemovedCables.clear();
}

void ComRemoveObject::redo()
{
    myHost->programManager->ChangeProgNow(currentGroup,currentProg);

    //get the object
    QSharedPointer<Connectables::Object> obj = myHost->objFactory->GetObjectFromId( objectInfo.forcedObjId );
    if(!obj)
        return;

    //get the container
    QSharedPointer<Connectables::Container> container = ContainerPtr.toStrongRef();
    if(!container)
        return;

    QDataStream stream(&objState, QIODevice::ReadWrite);
    obj->SaveProgram();
    obj->toStream( stream );
    obj->GetContainerAttribs(attr);

    //remove the object
    container->UserParkObject(obj,removeType,&listAddedCables,&listRemovedCables);

}
