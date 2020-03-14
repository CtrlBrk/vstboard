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
#include "precomp.h"
#include "proglistview.h"
#include "globals.h"
#include "models/groupsprogramsmodel.h"

ProgListView::ProgListView(QWidget *parent) :
    GroupListView(parent)
{
}

QStringList ProgListView::MimeTypes()
{
    return QStringList() << MIMETYPE_PROGRAM;
}

void ProgListView::setModel(GroupsProgramsModel *model)
{
    QListView::setModel(model);

//    connect(model, SIGNAL(GroupChanged(QModelIndex)),
//            this, SLOT(setRootIndex(QModelIndex)));
    connect(model, SIGNAL(ProgChanged(QModelIndex)),
            this, SLOT(SetCurrentNoSelect(QModelIndex)));

    connect(this, SIGNAL(clicked(QModelIndex)),
            model, SLOT(UserChangeProg(QModelIndex)));
}

void ProgListView::dragEnterEvent(QDragEnterEvent *event)
{
    //the root index (a group) is set to 'no-drop'
    //(otherwise dragging a group on top of another one will move it as a child of the group instead of reordering )
    //we have to force accept to allow reordering
    if(event->source() == this)
        event->accept();
    else
        event->ignore();
}

void ProgListView::dragMoveEvent ( QDragMoveEvent * event )
{
    //don't allow drop on itself
    QModelIndex idx = indexAt( event->pos() );

    if(idx.isValid()) {
        if( selectionModel()->selectedIndexes().contains( idx )) {
            event->ignore();
            return;
        }
    }

    //default hover highlight - but no dropindicator :(
    QListView::dragMoveEvent(event);

    //force accept and we'll get the drop
    if(event->source() == this)
        event->accept();
}

void ProgListView::dropEvent(QDropEvent *event)
{
    QListView::dropEvent(event);
}

//void ProgListView::currentChanged (const QModelIndex &current, const QModelIndex &previous)
//{
//    Q_UNUSED(previous)

//    ProgramsModel *progModel = qobject_cast<ProgramsModel*>(model());
//    if(!progModel)
//        return;

//    progModel->UserChangeProg( current );
//}

//void ProgListView::InsertItem()
//{
//    GroupsProgramsModel *progModel = qobject_cast<GroupsProgramsModel*>(model());
//    if(!progModel)
//        return;

//    int row=-1;
//    if(currentIndex().isValid())
//        row=currentIndex().row();

//    progModel->UserAddProgram( row );
//}
