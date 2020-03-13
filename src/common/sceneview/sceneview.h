/**************************************************************************
#    Copyright 2010-2012 RaphaÃ«l FranÃ§ois
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

#ifndef SCENEVIEW_H
#define SCENEVIEW_H

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4100 )
#endif

//#include "precomp.h"
#include "connectables/connectioninfo.h"
#include "views/maingraphicsview.h"
#include "msghandler.h"

namespace Connectables {
    class ObjectFactory;
}
class MainHost;
class MainWindow;
class MsgObject;
namespace View {
    class MainContainerView;
    class ViewConfig;
    class SceneView : public QObject, public MsgHandler
    {
        Q_OBJECT
    public:
        explicit SceneView(MainWindow *mainWindow, MsgController *msgCtrl, int objId, MainGraphicsView *viewHost, MainGraphicsView *viewProject, MainGraphicsView *viewProgram, MainGraphicsView *viewGroup,QWidget *parent = 0);

//        QRect visualRect(const QModelIndex &index) const {return QRect();}
//        void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible) {}
//        QModelIndex indexAt(const QPoint &point) const {return QModelIndex();}

        void SetParkings(QWidget *progPark, QWidget *groupPark);
        void ReceiveMsg(const MsgObject &msg);

        MainGraphicsView *viewHost;
        MainGraphicsView *viewProject;
        MainGraphicsView *viewProgram;
        MainGraphicsView *viewGroup;

    private:

        void AddContainer(const MsgObject &msg);

        //we need parent objects to avoid a bug in qgraphicssene
        QGraphicsRectItem *rootObjHost;
        QGraphicsRectItem *rootObjProject;
        QGraphicsRectItem *rootObjProgram;
        QGraphicsRectItem *rootObjGroup;

//        QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) {return QModelIndex();}
//        int horizontalOffset() const {return 0;}
//        int verticalOffset() const {return 0;}
//        bool isIndexHidden(const QModelIndex &index) const {return false;}
//        void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command) {}
//        QRegion visualRegionForSelection(const QItemSelection &selection) const {return QRegion();}

        QHash<QPersistentModelIndex,QObject*>hashItems;
        QMap<ConnectionInfo,QPersistentModelIndex>mapConnectionInfo;

        QGraphicsScene *sceneHost;
        QGraphicsScene *sceneProject;
        QGraphicsScene *sceneProgram;
        QGraphicsScene *sceneGroup;

        MainContainerView *hostContainerView;
        MainContainerView *projectContainerView;
        MainContainerView *programContainerView;
        MainContainerView *groupContainerView;

        QWidget *progParking;
        QWidget *groupParking;

        MainWindow *mainWindow;

    signals:
//        void hostShown(bool shown);
//        void projectShown(bool shown);
//        void programShown(bool shown);
//        void insertShown(bool shown);

    public slots:
        void ToggleHostView(bool show);
        void ToggleProjectView(bool show);
        void ToggleProgramView(bool show);
        void ToggleInsertView(bool show);

    };
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif // SCENEVIEW_H
