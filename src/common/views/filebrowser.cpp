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
//#include "precomp.h"
#include "filebrowser.h"
#include "ui_filebrowser.h"

FileBrowser::FileBrowser(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileBrowser),
    historyPosition(0),
    model(0)
{
    ui->setupUi(this);

    actRename = new QAction(tr("Rename"),ui->treeFiles);
    actRename->setShortcut(Qt::Key_F2);
    actRename->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    ui->treeFiles->addAction(actRename);
    connect(actRename,SIGNAL(triggered()),
            this,SLOT(Rename()));

    actDel = new QAction( QIcon::fromTheme("process-stop") ,tr("Delete"),ui->treeFiles);
    actDel->setShortcut(Qt::Key_Delete);
    actDel->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(actDel,SIGNAL(triggered()),
            this,SLOT(DeleteSelectedFile()));
    ui->treeFiles->addAction(actDel);

    actNewFolder = new QAction( QIcon::fromTheme("folder-new") ,tr("New Folder"),ui->treeFiles);
    connect(actNewFolder,SIGNAL(triggered()),
            this,SLOT(NewFolder()));
    ui->treeFiles->addAction(actNewFolder);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeFiles, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(OnContextMenu(QPoint)));

    connect(ui->treeFiles, SIGNAL(historyBack()),
            this, SLOT(on_previousDir_clicked() ));
    connect(ui->treeFiles, SIGNAL(historyForward()),
            this, SLOT(on_nextDir_clicked() ));
}

FileBrowser::~FileBrowser()
{
    delete ui;
}

void FileBrowser::setModel(QFileSystemModel *model)
{
    this->model = model;
    ui->treeFiles->setModel(model);

    if(!model)
        return;

    ui->treeFiles->setColumnHidden(1,true);
    ui->treeFiles->setColumnHidden(2,true);
    ui->treeFiles->setColumnHidden(3,true);
    setPath(model->rootPath());

    if(model->isReadOnly())
        ui->treeFiles->setContextMenuPolicy(Qt::NoContextMenu);
    else
        ui->treeFiles->setContextMenuPolicy(Qt::CustomContextMenu);

    actRename->setDisabled( model->isReadOnly() );
    actDel->setDisabled( model->isReadOnly() );
}

void FileBrowser::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void FileBrowser::on_path_textEdited(QString txt)
{
    QFileInfo info(txt);
    if(info.isDir())
        setPath(txt);
}

void FileBrowser::setPath(QString dir)
{
    for(int i=historyPosition+1; i<dirHistory.size();i++)
        dirHistory.removeAt(i);

    dirHistory << dir;

    if(dirHistory.size()>20) dirHistory.removeFirst();
    historyPosition = dirHistory.size()-1;

    if(dirHistory.size()>1)
        ui->previousDir->setEnabled(true);
    ui->nextDir->setEnabled(false);

    ui->path->setText(dir);
    ui->treeFiles->setRootIndex(model->index(dir));
}

QString FileBrowser::path()
{
    return dirHistory.value(historyPosition,"");
}

void FileBrowser::on_treeFiles_doubleClicked(QModelIndex index)
{
    QFileInfo info = model->fileInfo(index);
    if(!info.isReadable() || !info.isDir())
        return;
    setPath(info.canonicalFilePath());
}

void FileBrowser::on_parentDir_clicked()
{
    QFileInfo info = model->fileInfo( ui->treeFiles->rootIndex() );
    setPath(info.canonicalPath());
}

void FileBrowser::on_rootDir_clicked()
{
    setPath(":");
}

void FileBrowser::on_previousDir_clicked()
{
    if(historyPosition<=0) {
        return;
    }

    int cpt = historyPosition-1;

    while(cpt>0 && !QFileInfo(dirHistory[cpt]).exists() )
        cpt--;

    if(cpt<0)
        return;


    historyPosition=cpt;

    ui->path->setText(dirHistory[historyPosition]);
    ui->treeFiles->setRootIndex(model->index(dirHistory[historyPosition]));

    if(historyPosition==0)
        ui->previousDir->setEnabled(false);
    if(historyPosition<dirHistory.size())
        ui->nextDir->setEnabled(true);
}

void FileBrowser::on_nextDir_clicked()
{
    if(historyPosition >= dirHistory.size()-1) {
        return;
    }

    int cpt = historyPosition+1;

    while(cpt<dirHistory.size() && !QFileInfo(dirHistory[cpt]).exists() )
        cpt++;
    if(cpt==dirHistory.size())
        return;

    historyPosition=cpt;

    ui->path->setText(dirHistory[historyPosition]);
    ui->treeFiles->setRootIndex(model->index(dirHistory[historyPosition]));

    if(historyPosition>=0)
        ui->previousDir->setEnabled(true);
    if(historyPosition==dirHistory.size()-1)
        ui->nextDir->setEnabled(false);
}

void FileBrowser::OnContextMenu(const QPoint & pos)
{
    if(ui->treeFiles->indexAt(pos).isValid()) {
        QMenu::exec(ui->treeFiles->actions(), mapToGlobal(pos),nullptr,this);
        return;
    }

    QMenu menu(this);
    menu.addAction(actNewFolder);
    menu.exec(mapToGlobal(pos));
}

bool FileBrowser::DeleteFile(const QModelIndex &index, int &deleteConfirmed,bool &skipErrors)
{
    if(!index.isValid())
        return false;

    bool allChildrenDeleted=true;

    //delete children first
    if(model->isDir(index)) {
        int nbChildren = model->rowCount(index);
        QList<QPersistentModelIndex>listIdx;
        for(int i=0;i<nbChildren; i++) {
            const QModelIndex &child = model->index(i, 0, index);
            listIdx << child;
        }
        foreach(QPersistentModelIndex idx, listIdx) {
            if(!DeleteFile(idx,deleteConfirmed,skipErrors))
                allChildrenDeleted=false;
        }
    }

    //didn't delete all children : return false
    if(deleteConfirmed==QMessageBox::NoToAll || !allChildrenDeleted)
        return false;

    //yestoall not clicked yet : ask for confirmation
    if(deleteConfirmed!=QMessageBox::YesToAll) {
        ui->treeFiles->scrollTo(index);
        QMessageBox msgBox(QMessageBox::Warning,tr("Delete file/folder"),tr("Permanently delete %1 ?").arg(model->fileName(index)),QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::NoToAll, this);
        msgBox.exec();
        deleteConfirmed=msgBox.result();

        //not confirmed : return false
        if(deleteConfirmed==QMessageBox::No || deleteConfirmed==QMessageBox::NoToAll)
            return false;
    }

    if(!model->remove(index)) {
        //can't delete : show error if skipError was not clicked
        if(!skipErrors) {
            QMessageBox msgBox(QMessageBox::Information,tr("Can't delete file/folder"),tr("Unable to delete %1").arg(model->fileName(index)),QMessageBox::Ok | QMessageBox::YesToAll , this);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.exec();
            if(msgBox.result()==QMessageBox::YesToAll)
                skipErrors=true;
        }

        return false;
    }

    //all files and subfolders delete : return true
    return true;
}

void FileBrowser::DeleteSelectedFile()
{
    int confirmed=0;
    bool skipErr=false;

    //delete all selected items
    QList<QPersistentModelIndex>listIdx;
    foreach(QPersistentModelIndex index, ui->treeFiles->selectionModel()->selectedRows(0) ) {
        listIdx << index;
    }
    foreach(QPersistentModelIndex index, listIdx ) {
        DeleteFile(index,confirmed,skipErr);
        if(confirmed==QMessageBox::NoToAll)
            return;
    }
}

void FileBrowser::Rename()
{
    ui->treeFiles->scrollTo(ui->treeFiles->currentIndex());
    ui->treeFiles->edit(ui->treeFiles->currentIndex());
}

void FileBrowser::NewFolder()
{
    QString name(tr("New Folder"));
    QString newFolderName(name);
    int cpt=1;

    QModelIndex baseFolder = ui->treeFiles->currentIndex();

    if( !baseFolder.isValid() || !model->isDir(baseFolder) ) {
        baseFolder=ui->treeFiles->rootIndex();
    }

    while(model->index( model->filePath(baseFolder)+"/"+newFolderName).isValid()) {
        cpt++;
        newFolderName=name+" ("+QString::number(cpt)+")";
    }

    QModelIndex newDir = model->mkdir(baseFolder,newFolderName);
    if(!newDir.isValid()) {
        QMessageBox msgBox(QMessageBox::Information,tr("Can't create folder"),tr("Unable to create folder %1").arg(newFolderName),QMessageBox::Ok , this);
        msgBox.exec();
        return;
    } else {
        ui->treeFiles->scrollTo(newDir);
        ui->treeFiles->edit(newDir);
    }
}

void FileBrowser::setPalette(const QPalette &pal) {
    QWidget::setPalette(pal);
    ui->path->setPalette(pal);
}
