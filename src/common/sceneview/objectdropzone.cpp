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

#include "objectdropzone.h"
#include "globals.h"

using namespace View;

ObjectDropZone::ObjectDropZone(MsgController *msgCtrl, int objId, QGraphicsItem *parent, QWidget *parking) :
    QGraphicsWidget(parent),
    MsgHandler(msgCtrl,objId),
    myParking(parking)
{
    // qApp->installEventFilter(this);
    setAcceptDrops(true);
}


// bool ObjectDropZone::eventFilter(QObject *obj, QEvent *event)
// {
//     if(event->type() == QEvent::DragEnter) {
//         // LOG("Enter");
//         QDragMoveEvent *mouseEvent = static_cast<QDragMoveEvent*>(event);
//         LOG(QString("Mouse enter (%1,%2)").arg(mouseEvent->position().x()).arg(mouseEvent->position().y()));
//     } else if(event->type() == QEvent::DragMove) {
//         // LOG("DropMove");
//         QDragMoveEvent *mouseEvent = static_cast<QDragMoveEvent*>(event);
//         LOG(QString("Mouse move (%1,%2)")  .arg(mouseEvent->position().x()) .arg(mouseEvent->position().y())  );
//         QObject * src = mouseEvent->source();
//         LOG(src->objectName())
//     } else if(event->type() == QEvent::Drop) {
//         // LOG("Drop");
//     } else if(event->type() == QEvent::GraphicsSceneDrop)   {
//         // LOG("Scene drop");
//     }
//     return QObject::eventFilter(obj, event);
// }

void ObjectDropZone::dragEnterEvent( QGraphicsSceneDragDropEvent *event)
{
    //accepts objects from parking
    if(event->source() == myParking && myParking!=0) {
        QGraphicsWidget::dragEnterEvent(event);
        HighlightStart();
        return;
    }

    if (event->mimeData()->hasUrls()) {
        QString fName;
        QFileInfo info;

        foreach(QUrl url,event->mimeData()->urls()) {
            fName = url.toLocalFile();
            info.setFile( fName );

            if ( info.isReadable() ) {
                QString fileType(info.suffix().toLower());

                QStringList acceptedFiles;
                acceptedFiles << SETUP_FILE_EXTENSION << PROJECT_FILE_EXTENSION;
                acceptedFiles << SETUP_JSON_FILE_EXTENSION << SETUP_JSON_BINARY_FILE_EXTENSION;
                acceptedFiles << PROJECT_JSON_FILE_EXTENSION << PROJECT_JSON_BINARY_FILE_EXTENSION;
 #ifdef VSTSDK
                acceptedFiles << "dll" << "vst" << "vst3" << "fxb" << "fxp" << "clap";
#endif
                if( acceptedFiles.contains( info.suffix(), Qt::CaseInsensitive) ) {
                    event->setDropAction(Qt::CopyAction);
                    event->accept();
                    HighlightStart();
                    return;
                }
            }
        }
    }

    //accept Audio interface
    //accept Midi interface
    //accept Tools
    if(event->mimeData()->hasFormat("application/x-audiointerface") ||
       event->mimeData()->hasFormat("application/x-midiinterface") ||
       event->mimeData()->hasFormat("application/x-tools")) {
        event->setDropAction(Qt::CopyAction);
        event->accept();
        HighlightStart();
        return;
    }
    event->ignore();
}

void ObjectDropZone::dragMoveEvent( QGraphicsSceneDragDropEvent *event)
{
    if(event->modifiers() & Qt::ControlModifier)
        event->setDropAction(Qt::CopyAction);
    else
        event->setDropAction(Qt::MoveAction);

    QGraphicsWidget::dragMoveEvent(event);
}

void ObjectDropZone::dragLeaveEvent( QGraphicsSceneDragDropEvent *event)
{
    HighlightStop();
    QGraphicsWidget::dragLeaveEvent(event);
}

void ObjectDropZone::dropEvent( QGraphicsSceneDragDropEvent *event)
{
    HighlightStop();

    MsgObject msg;
    TranslateMimeData(event->mimeData(), msg);

    emit(ObjectDropped(event, msg));
    //reset to copy : we don't really want a moveaction
    event->setDropAction(Qt::CopyAction);
//    QGraphicsWidget::dropEvent(event);
}

void ObjectDropZone::HighlightStart()
{
    setAutoFillBackground(true);
}

void ObjectDropZone::HighlightStop()
{
    setAutoFillBackground(false);
}

void ObjectDropZone::UpdateHeight()
{
    resize( size().width(), parentWidget()->size().height() );
}

bool ObjectDropZone::TranslateMimeData( const QMimeData * data, MsgObject &msg )
{
    QByteArray listObjInfoToAdd;
    QDataStream streamObj(&listObjInfoToAdd, QIODevice::WriteOnly);

    //drop a file
    if (data->hasUrls()) {
        QStringList lstFiles;
        foreach(QUrl url, data->urls()) {

            QFileInfo fInfo;
            fInfo.setFile( url.toLocalFile() );
            QString fileType(fInfo.suffix().toLower());

            ObjectInfo i;

#ifdef VSTSDK
            //vst plugin
            if ( fileType=="dll" || fileType=="vst") {
                i.nodeType = NodeType::object;
                i.objType = ObjType::VstPlugin;
                i.filename = url.toLocalFile();
                i.name = fInfo.baseName();
                streamObj << i;
                continue;
            }
            //vst3 plugin
            if ( fileType=="vst3" ) {
                i.id = FixedObjId::noContainer;
                i.nodeType = NodeType::object;
                i.objType = ObjType::Vst3Plugin;
                i.filename = url.toLocalFile();
                i.name = fInfo.baseName();
                streamObj << i;
                continue;
            }
#endif
            if ( fileType=="clap") {
                i.nodeType = NodeType::object;
                i.objType = ObjType::ClapPlugin;
                i.filename = url.toLocalFile();
                i.name = fInfo.baseName();
                streamObj << i;
                continue;
            }
            lstFiles << url.toLocalFile();
        }
        if(!lstFiles.isEmpty())
            msg.prop[MsgObject::FilesToLoad]=lstFiles;
    }

    //objects from parking
//    QByteArray unparkObj;
//    QDataStream stream(&unparkObj, QIODevice::WriteOnly);

    if(data->hasFormat("application/x-qstandarditemmodeldatalist")) {
        QStandardItemModel mod;
        mod.dropMimeData(data,Qt::CopyAction,0,0,QModelIndex());
        int a=mod.rowCount();
        for(int i=0;i<a;i++) {
            QStandardItem *it = mod.invisibleRootItem()->child(i);

            if(it->data(UserRoles::objInfo).isValid()) {
                ObjectInfo inf = it->data(UserRoles::objInfo).value<ObjectInfo>();
                streamObj << inf;
//                stream << it->data().toInt();
            }
        }
    }
//    if(!unparkObj.isEmpty())
//        msg.prop[MsgObject::objToUnpark"]=unparkObj;



    //audio interface
    if(data->hasFormat("application/x-audiointerface")) {
        QByteArray b = data->data("application/x-audiointerface");
        QDataStream stream(&b,QIODevice::ReadOnly);

        while(!stream.atEnd()) {
            ObjectInfo info;
            stream >> info;

            if(info.inputs!=0) {
                info.objType=ObjType::AudioInterfaceIn;
                streamObj << info;
            }

            if(info.outputs!=0) {
                info.objType=ObjType::AudioInterfaceOut;
                streamObj << info;
            }
        }
    }

    //midi interface
    if(data->hasFormat("application/x-midiinterface")) {
        QByteArray b = data->data("application/x-midiinterface");
        QDataStream stream(&b,QIODevice::ReadOnly);

        while(!stream.atEnd()) {
            ObjectInfo info;
            stream >> info;
            streamObj << info;
        }
    }

    //tools
    if(data->hasFormat("application/x-tools")) {
        QByteArray b = data->data("application/x-tools");
        QDataStream stream(&b,QIODevice::ReadOnly);

        while(!stream.atEnd()) {
            ObjectInfo info;
            stream >> info;
            streamObj << info;
        }
    }

    if(!listObjInfoToAdd.isEmpty())
        msg.prop[MsgObject::ObjectsToLoad]=listObjInfoToAdd;

    return true;
}
