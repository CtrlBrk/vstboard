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

#include "sceneview.h"
#include "globals.h"
#include "connectables/objectfactory.h"
#include "connectableobjectview.h"
#include "maincontainerview.h"
#include "bridgeview.h"
#include "connectables/container.h"
#include "connectables/objectfactory.h"
#include "mainhost.h"
#include <QSplitter>
#include "commands/comaddcable.h"
#include "commands/comdisconnectpin.h"
#include "commands/comremovepin.h"

using namespace View;

SceneView::SceneView(MainWindow *mainWindow, MsgController *msgCtrl, int objId, MainGraphicsView *viewHost, MainGraphicsView *viewProject, MainGraphicsView *viewProgram, MainGraphicsView *viewGroup,QWidget *parent) :
    QObject(parent),
    MsgHandler(msgCtrl, objId),
    viewHost(viewHost),
    viewProject(viewProject),
    viewProgram(viewProgram),
    viewGroup(viewGroup),
    rootObjHost(0),
    rootObjProject(0),
    rootObjProgram(0),
    rootObjGroup(0),
    sceneHost(0),
    sceneProgram(0),
    sceneGroup(0),
    hostContainerView(0),
    projectContainerView(0),
    programContainerView(0),
    groupContainerView(0),
    progParking(0),
    groupParking(0),
    mainWindow(mainWindow)
{
    sceneHost = new QGraphicsScene(this);
    sceneProject = new QGraphicsScene(this);
    sceneProgram = new QGraphicsScene(this);
    sceneGroup = new QGraphicsScene(this);

    viewHost->SetViewConfig(mainWindow->viewConfig);
    viewProject->SetViewConfig(mainWindow->viewConfig);
    viewProgram->SetViewConfig(mainWindow->viewConfig);
    viewGroup->SetViewConfig(mainWindow->viewConfig);

    //we need a root object to avoid a bug when the scene is empty
    rootObjHost = new QGraphicsRectItem(0);
    sceneHost->addItem(rootObjHost);
    rootObjProject = new QGraphicsRectItem(0);
    sceneProject->addItem(rootObjProject);
    rootObjProgram = new QGraphicsRectItem(0);
    sceneProgram->addItem(rootObjProgram);
    rootObjGroup = new QGraphicsRectItem(0);
    sceneGroup->addItem(rootObjGroup);

    viewHost->setScene(sceneHost);
    viewProject->setScene(sceneProject);
    viewProgram->setScene(sceneProgram);
    viewGroup->setScene(sceneGroup);

//    connect(myHost->programManager,SIGNAL(ProgChanged(quint32)),
//            viewProgram, SLOT(SetViewProgram(quint32)));
//    connect(myHost->programManager,SIGNAL(ProgDelete(quint32)),
//            viewProgram, SLOT(RemoveViewProgram(quint32)));

//    connect(myHost->programManager,SIGNAL(GroupChanged(quint32)),
//            viewGroup, SLOT(SetViewProgram(quint32)));
//    connect(myHost->programManager,SIGNAL(GroupDelete(quint32)),
//            viewGroup, SLOT(RemoveViewProgram(quint32)));
}

void SceneView::SetParkings(QWidget *progPark, QWidget *groupPark)
{
    progParking = progPark;
    groupParking = groupPark;

    if(programContainerView)
        programContainerView->SetParking(progParking);
    if(groupContainerView)
        groupContainerView->SetParking(groupParking);
}

void SceneView::ReceiveMsg(const MsgObject &msg)
{
    //update existing object
//    if(msg.prop.contains(MsgObject::Update)) {
//        if(!msgCtrl->listObj.contains(msg.objIndex)) {
////            LOG("obj not found"<<msg.objIndex)
//            return;
//        }
//        msgCtrl->listObj.value(msg.objIndex)->ReceiveMsg(msg);
//        return;
//    }

    //create new object
    if(msg.prop.contains(MsgObject::Add)) {

        switch(msg.prop[MsgObject::Add].toInt()) {
            case NodeType::container :
                AddContainer(msg);
                break;
        }
        return;
    }

    LOG("unhandled action"<<msg.prop);
}

void SceneView::AddContainer(const MsgObject &msg)
{
    if(msgCtrl->listObj.contains( msg.prop[MsgObject::Id].toInt() )) {
        delete msgCtrl->listObj[msg.prop[MsgObject::Id].toInt()];
    }

    MainContainerView *objView = new MainContainerView( mainWindow->viewConfig, msgCtrl, msg.prop[MsgObject::Id].toInt() );

    switch(msg.prop[MsgObject::Id].toInt()) {
        case FixedObjId::hostContainer :
            objView->setParentItem(rootObjHost);
            connect(viewHost,SIGNAL(viewResized(QRectF)),
                    objView,SLOT(OnViewChanged(QRectF)));
            QTimer::singleShot(0, viewHost, SLOT(ForceResize()));
            break;

        case FixedObjId::projectContainer :
            objView->setParentItem(rootObjProject);
            connect(viewProject,SIGNAL(viewResized(QRectF)),
                    objView,SLOT(OnViewChanged(QRectF)));
            QTimer::singleShot(0, viewProject, SLOT(ForceResize()));
            break;

        case FixedObjId::programContainer :
            objView->setParentItem(rootObjProgram);
            objView->SetParking(progParking);
            connect(viewProgram,SIGNAL(viewResized(QRectF)),
                    objView,SLOT(OnViewChanged(QRectF)));
            QTimer::singleShot(0, viewProgram, SLOT(ForceResize()));
            break;

        case FixedObjId::groupContainer :
            objView->setParentItem(rootObjGroup);
            objView->SetParking(groupParking);
            connect(viewGroup,SIGNAL(viewResized(QRectF)),
                    objView,SLOT(OnViewChanged(QRectF)));
            QTimer::singleShot(0, viewGroup, SLOT(ForceResize()));
            break;

        default :
            LOG("unknown container")
            return;
    }

    objView->ReceiveMsg(msg);
}

void SceneView::ToggleHostView(bool show)
{
    viewHost->setVisible(show);
}

void SceneView::ToggleProjectView(bool show)
{
    viewProject->setVisible(show);
}

void SceneView::ToggleProgramView(bool show)
{
    viewProgram->setVisible(show);
}

void SceneView::ToggleInsertView(bool show)
{
    viewGroup->setVisible(show);
}
