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

#ifndef GROUPLISTVIEW_H
#define GROUPLISTVIEW_H

#include <QListView>

class GroupsProgramsModel;
class GroupListView : public QListView
{
    Q_OBJECT
public:
    explicit GroupListView(QWidget *parent = 0);
    virtual void setModel(GroupsProgramsModel *model);

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragMoveEvent ( QDragMoveEvent * event );
    virtual void dropEvent(QDropEvent *event);
//    virtual void currentChanged (const QModelIndex &current, const QModelIndex &previous);

    virtual QStringList MimeTypes();

    QAction *actRename;
    QAction *actDel;
    QAction *actAddNew;
    QAction *actCopy;
    QAction *actPaste;
    QAction *actCut;

private:
    void CreateActions();

signals:
    void ShowProgList(const QModelIndex &index);

public slots:
    virtual void SetCurrentNoSelect(const QModelIndex &index);
    virtual void OnContextMenu(const QPoint & pos);
    virtual void EditItem();
    virtual void DeleteItem();
    virtual void InsertItem();
    virtual void Copy();
    virtual void Paste();
    virtual void Cut();
};

#endif // GROUPLISTVIEW_H
