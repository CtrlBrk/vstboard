#include "comremoveobject.h"
#include "mainhost.h"
#include "connectables/objects/container.h"
#include "models/programsmodel.h"

ComRemoveObject::ComRemoveObject( MainHost *myHost,
                                  const MetaInfo &objectInfo,
                                  RemoveType::Enum removeType,
                                  QUndoCommand  *parent) :
    ThreadSafeCom(myHost,parent),
    objectInfo(objectInfo),
    removeType(removeType),
    currentGroup(0),
    currentProg(0)
{
    setText(QObject::tr("Remove %1").arg(objectInfo.Name()));

    currentGroup = myHost->programsModel->GetCurrentMidiGroup();
    currentProg =  myHost->programsModel->GetCurrentMidiProg();
}

void ComRemoveObject::tUndo ()
{
    QMutexLocker l(&myHost->programsModel->mutexProgChange);
    myHost->programsModel->ChangeProgNow(currentGroup,currentProg);

    //get the container
    QSharedPointer<Connectables::Container> container = myHost->objFactory->GetObjectFromId( objectInfo.ContainerId() ).staticCast<Connectables::Container>();
    if(!container)
        return;

    //get the object
    QSharedPointer<Connectables::Object> obj = myHost->objFactory->GetObjectFromId( objectInfo.ObjId() );
    if(!obj) {
          //object was deleted, create a new one
        obj = myHost->objFactory->NewObject( objectInfo );
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
    QPair<MetaInfo,MetaInfo>pair;
    foreach( pair, listAddedCables) {
        myHost->objFactory->UpdatePinInfo( pair.first );
        myHost->objFactory->UpdatePinInfo( pair.second );
        container->UserRemoveCable(pair);
    }
    //add cables removed at creation
    foreach( pair, listRemovedCables) {
        myHost->objFactory->UpdatePinInfo( pair.first );
        myHost->objFactory->UpdatePinInfo( pair.second );
        container->UserAddCable(pair);

    }

    listAddedCables.clear();
    listRemovedCables.clear();
}

void ComRemoveObject::tRedo()
{
    QMutexLocker l(&myHost->programsModel->mutexProgChange);
    myHost->programsModel->ChangeProgNow(currentGroup,currentProg);

    //get the container
    QSharedPointer<Connectables::Container> container = myHost->objFactory->GetObjectFromId( objectInfo.ContainerId() ).staticCast<Connectables::Container>();
    if(!container)
        return;

    myHost->programsModel->ChangeProgNow(currentGroup,currentProg);

    //get the object
    QSharedPointer<Connectables::Object> obj = myHost->objFactory->GetObjectFromId( objectInfo.ObjId() );
    if(!obj)
        return;

    QDataStream stream(&objState, QIODevice::ReadWrite);

    obj->SaveProgram();
    obj->toStream( stream );
    obj->GetContainerAttribs(attr);

    //remove the object
    container->UserParkObject(obj,removeType,&listAddedCables,&listRemovedCables);
}
