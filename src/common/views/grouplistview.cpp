/**************************************************************************
#    Copyright 2010-2011 Rapha�l Fran�ois
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
//#include "precomp.h"
#include "grouplistview.h"
#include "globals.h"
#include "models/programsmodel.h"

GroupListView::GroupListView(QWidget *parent) :
    QListView(parent)
{
    CreateActions();

    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(OnContextMenu(QPoint)));
}

QStringList GroupListView::MimeTypes()
{
    return QStringList() << MIMETYPE_GROUP << MIMETYPE_PROGRAM;
}

void GroupListView::CreateActions()
{
    actRename = new QAction(QIcon(":/img16x16/cell_edit.png"),tr("Rename"),this);
    actRename->setShortcut( Qt::Key_F2 );
    actRename->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(actRename,SIGNAL(triggered()),
            this,SLOT(EditItem()));
    addAction(actRename);

    actAddNew = new QAction(QIcon(":/img16x16/edit_add.png"),tr("Insert new"),this);
    actAddNew->setShortcut( Qt::Key_Insert );
    actAddNew->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(actAddNew,SIGNAL(triggered()),
            this,SLOT(InsertItem()));
    addAction(actAddNew);

    actDel = new QAction(QIcon(":/img16x16/edit_remove.png"),tr("Delete"),this);
    actDel->setShortcut(QKeySequence::Delete);
    actDel->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(actDel,SIGNAL(triggered()),
            this,SLOT(DeleteItem()));
    addAction(actDel);

    actCopy = new QAction( QIcon(":/img16x16/editcopy.png"), "Copy", this);
    actCopy->setShortcuts(QKeySequence::Copy);
    actCopy->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(actCopy, SIGNAL(triggered()),
            this, SLOT(Copy()));
    addAction(actCopy);

    actPaste = new QAction( QIcon(":/img16x16/editpaste.png"), "Paste", this);
    actPaste->setShortcuts(QKeySequence::Paste);
    actPaste->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(actPaste, SIGNAL(triggered()),
            this, SLOT(Paste()));
    addAction(actPaste);

    actCut = new QAction( QIcon(":/img16x16/editcut.png"), "Cut", this);
    actCut->setShortcuts(QKeySequence::Cut);
    actCut->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(actCut, SIGNAL(triggered()),
            this, SLOT(Cut()));
    addAction(actCut);
}

void GroupListView::setModel(ProgramsModel *model)
{
    QListView::setModel(model);

    connect(model, SIGNAL(GroupChanged(QModelIndex)),
            this,SLOT(SetCurrentNoSelect(QModelIndex)));

    connect(this, SIGNAL(clicked(QModelIndex)),
            model, SLOT(UserChangeGroup(QModelIndex)));
}

void GroupListView::SetCurrentNoSelect(const QModelIndex &index)
{
    selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
}

void GroupListView::dragEnterEvent(QDragEnterEvent *event)
{
    //accept programs, will add the at the end of the list
    if(event->mimeData()->formats().contains(MIMETYPE_PROGRAM))
        event->accept();

    //default behavior
    QListView::dragEnterEvent(event);

}

void GroupListView::dragMoveEvent ( QDragMoveEvent * event )
{
    //default behavior for ourself (groups reordering)
    if(event->source() == this) {
        setDropIndicatorShown(true);
        QListView::dragMoveEvent(event);
        return;
    }

    //accept if we drag a program over a group +switch to the group hovered
    QModelIndex i = indexAt(event->pos());
    if(i.isValid() && event->mimeData()->formats().contains(MIMETYPE_PROGRAM)) {

        //show the group content
        ProgramsModel *progModel = qobject_cast<ProgramsModel*>(model());
        if(!progModel)
            return;
        progModel->UserChangeGroup( i );

        setDropIndicatorShown(false);
        selectionModel()->setCurrentIndex(i, QItemSelectionModel::NoUpdate);
        return;
    }

    //ignore if we're not over a program
    event->ignore();
}

void GroupListView::dropEvent(QDropEvent *event)
{
    //allow program drop on a group
    if(event->mimeData()->formats().contains(MIMETYPE_PROGRAM)) {
        QModelIndex i = indexAt(event->pos());
        if(i.isValid()) {
            event->accept();
            model()->dropMimeData( event->mimeData(), event->dropAction(), i.row(), 0, QModelIndex() );
            return;
        }
    }

    //default behavior (reorder groups)
    QListView::dropEvent(event);
}

//void GroupListView::currentChanged (const QModelIndex &current, const QModelIndex &previous)
//{
//    Q_UNUSED(previous)

//    ProgramsModel *progModel = qobject_cast<ProgramsModel*>(model());
//    if(!progModel)
//        return;

//    progModel->UserChangeGroup( current );
//}

void GroupListView::OnContextMenu(const QPoint & pos)
{
    QMenu menu;
    menu.exec(actions(), mapToGlobal(pos), actions().at(0), this);
}

void GroupListView::EditItem()
{
    if(currentIndex().isValid())
        edit(currentIndex());
}

void GroupListView::DeleteItem()
{
    ProgramsModel *progModel = qobject_cast<ProgramsModel*>(model());
    if(!progModel)
        return;

    if(!selectedIndexes().isEmpty())
        progModel->UserRemoveRows(selectedIndexes(),rootIndex());
    else if(currentIndex().isValid())
        progModel->UserRemoveRows(QModelIndexList()<<currentIndex(),rootIndex());
}

void GroupListView::InsertItem()
{
    ProgramsModel *progModel = qobject_cast<ProgramsModel*>(model());
    if(!progModel)
        return;

    int row=-1;
    if(currentIndex().isValid())
        row=currentIndex().row();

    progModel->UserAddGroup( row );
}

void GroupListView::Copy()
{
    QMimeData *mime=0;

    if(!selectedIndexes().isEmpty())
        mime = model()->mimeData( selectedIndexes() );
    else if(currentIndex().isValid())
        mime = model()->mimeData( QModelIndexList() << currentIndex() );

    if(!mime)
        return;

    QApplication::clipboard()->setMimeData( mime );
}

void GroupListView::Paste()
{
    int row=-1;
    if(currentIndex().isValid())
        row=currentIndex().row();

    const QMimeData *mime = QApplication::clipboard()->mimeData();
    foreach( const QString &t, MimeTypes() ) {
        if(mime->hasFormat(t)) {
            model()->dropMimeData( mime, Qt::CopyAction, row, 0, rootIndex() );
            return;
        }
    }
}

void GroupListView::Cut()
{
    Copy();
    DeleteItem();
}
