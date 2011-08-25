/**************************************************************************
#    Copyright 2010-2011 Raphaël François
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

#include "scenemodel.h"
#include "globals.h"
#include "connectables/objectfactory.h"
#include "mainhost.h"
#include "models/programsmodel.h"
#include "connectables/objects/vstplugin.h"
#include "commands/comaddobject.h"
#include "objectinfo.h"
#include "events.h"
#include "mainwindow.h"
#include "sceneview/sceneview.h"

SceneModel::SceneModel(MainHost *myHost, View::SceneView *view, QObject *parent) :
    QObject(parent),
    MetaTransporter(),
    myHost(myHost),
    mySceneView(view),
    delayedAction(0),
    LoadFileMapper(0)
{
    setObjectName("HostModel");

    MetaTransporter::autoUpdate=true;

    LoadFileMapper = new QSignalMapper(this);
    delayedAction = new QTimer(this);
    delayedAction->setSingleShot(true);
    if(myHost) {
        connect(LoadFileMapper, SIGNAL(mapped(QString)), myHost, SLOT(LoadFile(QString)));
        connect(delayedAction, SIGNAL(timeout()), LoadFileMapper, SLOT(map()));
    }
}

void SceneModel::removeFromParking(quint32 objId, QStandardItemModel *model)
{
    int i=model->rowCount()-1;
    while(i>=0) {
        QStandardItem *item = model->invisibleRootItem()->child(i);
        if(!item) {
            LOG("item not found"<<i);
            --i;
            continue;
        }

        if(item->data(UserRoles::id).toInt()==objId)
            model->removeRow(i);

        --i;
    }
}

bool SceneModel::event(QEvent *event)
{
    switch(event->type()) {
        case Events::typeNewObj : {
            Events::sendObj *e = static_cast<Events::sendObj*>(event);
            e->objInfo.SetTransporter(this);
//            if(e->objInfo.ObjId()==1022) {
//                LOG("add" << e->objInfo.toStringFull());
//            }
            mySceneView->AddObj(e->objInfo);

            if(e->objInfo.Type()==MetaTypes::object) {
                if(e->objInfo.ContainerId()==FixedObjIds::programContainer) {
                    removeFromParking(e->objInfo.ObjId(), &myHost->mainWindow->programParkModel);
                }
                if(e->objInfo.ContainerId()==FixedObjIds::groupContainer) {
                    removeFromParking(e->objInfo.ObjId(), &myHost->mainWindow->groupParkModel);
                }
            }
            return true;
        }
        case Events::typeParkObj : {
            Events::sendObj *e = static_cast<Events::sendObj*>(event);
//            e->objInfo.SetTransporter(this);
//            if(e->objInfo.ObjId()==1022) {
//                LOG("park" << e->objInfo.toStringFull());
//            }
            if(e->objInfo.Type()==MetaTypes::object) {
                mySceneView->DelObj(e->objInfo.ObjId());

                if(e->objInfo.ContainerId()==FixedObjIds::programContainer) {
                    myHost->mainWindow->programParkModel.appendRow( e->objInfo.toModelItem() );
                }
                if(e->objInfo.ContainerId()==FixedObjIds::groupContainer) {
                    myHost->mainWindow->groupParkModel.appendRow( e->objInfo.toModelItem() );
                }
            } else {
                mySceneView->DelObj(e->objInfo.ObjId());
            }
            return true;
        }
        case Events::typeDelObj : {
            Events::delObj *e = static_cast<Events::delObj*>(event);
//            if(e->objId==1022) {
//                LOG("del" << e->objId);
//            }
            mySceneView->DelObj(e->objId);
            removeFromParking(e->objId, &myHost->mainWindow->programParkModel);
            removeFromParking(e->objId, &myHost->mainWindow->groupParkModel);
            return true;
        }
        case Events::typeUpdateObj : {
            Events::sendObj *e = static_cast<Events::sendObj*>(event);
            e->objInfo.SetTransporter(this);
//            if(e->objInfo.ObjId()==1022) {
//                LOG("update" << e->objInfo.toStringFull());
//            }
            mySceneView->UpdateObj(e->objInfo);
            return true;
        }
    }

    return QObject::event(event);
}

bool SceneModel::dropFile( const QString &fName, MetaInfo &info, MetaInfo &target)
{
    QFileInfo fileInfo;
    fileInfo.setFile( fName );

    if ( !fileInfo.isFile() || !fileInfo.isReadable() )
        return false;

    QString fileType(fileInfo.suffix().toLower());


#ifdef VSTSDK
    //vst plugin ?
    if ( fileType=="dll" ) {
        info.SetType(MetaTypes::object);
        info.SetMeta(MetaInfos::ObjType, ObjTypes::VstPlugin);
        info.SetMeta(MetaInfos::Filename, fName);
        return true;
    }
#endif

    //fxb file
    if( fileType == VST_BANK_FILE_EXTENSION ) {
        if(target.Meta(MetaInfos::ObjType).toInt() != ObjTypes::VstPlugin) {
            LOG("fxb bad target");
            return false;
        }

//        if( senderObj->LoadBank(fName) ) {
            target.SetMeta(MetaInfos::bankFile,fName);
            return true;
//        }
    }

    if( fileType == VST_PROGRAM_FILE_EXTENSION ) {
        if(target.Meta(MetaInfos::ObjType).toInt() != ObjTypes::VstPlugin) {
            LOG("fxp bad target");
            return false;
        }

//        if( senderObj->LoadProgram(fName) ) {
            target.SetMeta(MetaInfos::programFile,fName);
            return true;
//        }
    }

    //setup file
    if ( fileType==SETUP_FILE_EXTENSION ) {
        if(myHost->programsModel->userWantsToUnloadSetup()) {
            //load on the next loop : we have to get out of the container before loading files
            LoadFileMapper->setMapping(delayedAction, fName);
            delayedAction->start(0);
            return true;
        }
    }

    //project file
    if ( fileType==PROJECT_FILE_EXTENSION ) {
        if(myHost->programsModel->userWantsToUnloadProject()) {
            //load on the next loop : we have to get out of the container before loading files
            LoadFileMapper->setMapping(delayedAction, fName);
            delayedAction->start(0);
            return true;
        }
    }

    return false;
}

bool SceneModel::dropMime ( const QMimeData * data, MetaInfo & senderInfo, QPointF &pos, InsertionType::Enum insertType )
{
    QList<MetaInfo>listObjInfoToAdd;


    if(data->hasFormat(MIMETYPE_METAINFO)) {
        QByteArray b = data->data(MIMETYPE_METAINFO);
        QDataStream stream(&b,QIODevice::ReadOnly);

        while(!stream.atEnd()) {
            MetaInfo info;
            stream >> info;
            listObjInfoToAdd << info;
        }
    }

    //drop a file
    if (data->hasUrls()) {
        foreach(QUrl url,data->urls()) {
            QString fName = url.toLocalFile();
            MetaInfo info;
            if(!dropFile(fName, info, senderInfo)) {
                myHost->mainWindow->DisplayMessage(
                            QMessageBox::Information,
                            tr("File not accepted"),
                            tr("The object %1 cannot load %2").arg(senderInfo.Name().arg(fName))
                            );
                continue;
            }

            if(info.Type()!=MetaTypes::ND)
                listObjInfoToAdd << info;
        }
    }


//    //audio interface
//    if(data->hasFormat(MIMETYPE_AUDIODEVICE)) {
//        QByteArray b = data->data(MIMETYPE_AUDIODEVICE);
//        QDataStream stream(&b,QIODevice::ReadOnly);

//        while(!stream.atEnd()) {
//            MetaInfo info;
//            stream >> info;
//            if(info.Meta(MetaInfos::ObjType).toInt() == ObjTypes::AudioInterface) {
//                if(info.Meta(MetaInfos::nbInputs).toInt()!=0) {
//                    info.SetMeta(MetaInfos::Direction,Directions::Input);
//                    listObjInfoToAdd << info;
//                 }

//                if(info.Meta(MetaInfos::nbOutputs).toInt()!=0) {

//                    //move the second device
//                    if(info.Meta(MetaInfos::nbInputs).toInt()!=0) {
//                        QPointF pt = info.Meta(MetaInfos::Position).toPointF();
//                        pt.rx()+=110;
//                        info.SetMeta(MetaInfos::Position,pt);
//                    }

//                    info.SetMeta(MetaInfos::Direction,Directions::Output);
//                    listObjInfoToAdd << info;
//                }
//            }
//        }
//    }



    foreach(MetaInfo info, listObjInfoToAdd) {

        if(senderInfo.Type()==MetaTypes::container)
            info.SetContainerId(senderInfo.ObjId());
        else
            info.SetContainerId(senderInfo.ContainerId());

        info.SetMeta(MetaInfos::Position,pos);
        pos+=QPointF(15,15);

        MetaInfo targetInfo;
        if(senderInfo.Type()==MetaTypes::object)
            targetInfo=senderInfo;

        ComAddObject *com = new ComAddObject(myHost, info, targetInfo, insertType);
        Connectables::VstPlugin::shellSelectView->command=com;
        //emit UndoStackPush(com);
        Events::command *e = new Events::command(com);
        PostEvent(e);
    }

    return true;
}

