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

#ifndef FILEBROWSER_H
#define FILEBROWSER_H

namespace Ui {
    class FileBrowser;
}

class FileBrowser : public QWidget {
    Q_OBJECT
public:
    FileBrowser(QWidget *parent = 0);
    ~FileBrowser();

    void setModel(QFileSystemModel *model=0);
    void setPath(QString dir);
    QString path();

protected:
    void changeEvent(QEvent *e);
    QAction *actRename;
    QAction *actDel;
    QAction *actNewFolder;

private:
    Ui::FileBrowser *ui;
    QStringList dirHistory;
    int historyPosition;
    QFileSystemModel *model;


signals:
    void DeleteFile(const QModelIndexList &listIndex);

private slots:
    void on_nextDir_clicked();
    void on_previousDir_clicked();
    void on_rootDir_clicked();
    void on_parentDir_clicked();
    void on_treeFiles_doubleClicked(QModelIndex index);
    void on_path_textEdited(QString );
    void OnContextMenu(const QPoint & pos);
    void DeleteItem();
    void Rename();
    void NewFolder();
};

#endif // FILEBROWSER_H
