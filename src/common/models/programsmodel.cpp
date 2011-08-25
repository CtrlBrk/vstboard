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

#include "programsmodel.h"
#include "globals.h"
#include "mainhost.h"
#include "commands/comchangeprogramitem.h"
#include "commands/comremoveprogram.h"
#include "commands/comaddprogram.h"
#include "commands/comremovegroup.h"
#include "commands/comaddgroup.h"
#include "commands/comchangeprogram.h"
#include "commands/comdiscardchanges.h"
#include "commands/comchangeautosave.h"

#include "mainwindow.h"

ProgramsModel::ProgramsModel(MainHost *myHost, QObject *parent) :
    QStandardItemModel(parent),
    myHost(myHost),
    currentGrp(QModelIndex()),
    currentPrg(QModelIndex()),
    nextGroupId(1),
    nextProgId(1),
    progAutosaveState(Qt::Checked),
    groupAutosaveState(Qt::Checked),
    dirtyFlag(false),
    currentProgColor(Qt::green),
    fromCom(false),
    currentCommandGroup(0),
    nbOfCommandsToGroup(0),
    openedPrompt(false),
    currentCommandHasBeenProcessed(false),
    lastDialogAnswer(false)
{
    setObjectName("ProgramsModel");

    connect(this, SIGNAL(UndoStackPush(QUndoCommand*)),
            myHost, SLOT(UndoStackPush(QUndoCommand*)));

}

void ProgramsModel::UserAddGroup(int row)
{
    if(row<0 || row>rowCount())
        row=rowCount();

    QByteArray tmpBa;
    QDataStream stream(&tmpBa,QIODevice::WriteOnly);
    stream << QString(tr("New"));

    QUndoCommand *com = new QUndoCommand( tr("New group") );
    new ComAddGroup(this,row,&tmpBa,com);
    new ComAddProgram(this,row,0,&tmpBa,com);
    emit UndoStackPush( com );
}

void ProgramsModel::UserAddProgram(const QModelIndex &grpIndex, int row)
{
    if(!grpIndex.isValid())
        return;

    int m = rowCount(grpIndex);
    if( row < 0 || row > m )
        row=m;

    QByteArray tmpBa;
    QDataStream stream(&tmpBa,QIODevice::WriteOnly);
    stream << QString(tr("New"));

    emit UndoStackPush( new ComAddProgram(this,grpIndex.row(),row,&tmpBa) );
}

bool ProgramsModel::AddGroup(QModelIndex &index, int row)
{
    int groupId = myHost->programsModel->GetNextGroupId();

    QString name("New grp");

    //create the group
    QStandardItem *groupItem = new QStandardItem( name );
    groupItem->setData(GroupNode,NodeType);
    groupItem->setData(groupId,ProgramId);
#ifndef QT_NO_DEBUG
    groupItem->setData(groupId,Qt::ToolTipRole);
#endif
    groupItem->setDragEnabled(true);
    groupItem->setDropEnabled(false);
    groupItem->setEditable(true);

    if(row==-1)
        row=rowCount();
    insertRow( row, groupItem );

    index = groupItem->index();

    return true;
}

bool ProgramsModel::AddProgram(int groupNum, QModelIndex &index, int row)
{
    QStandardItem *groupItem = item(groupNum);

    //if the group was disabled re-enable it
    if(groupItem->rowCount()==0) {
        groupItem->setBackground(Qt::transparent);
        groupItem->setToolTip("");
    }

    int progId = myHost->programsModel->GetNextProgId();

    QString name("New prog");

    //create the program item
    QStandardItem *prgItem = new QStandardItem( name );
    prgItem->setData(ProgramNode,NodeType);
    prgItem->setData(progId,ProgramId);
#ifndef QT_NO_DEBUG
    prgItem->setData(progId,Qt::ToolTipRole);
#endif
    prgItem->setDragEnabled(true);
    prgItem->setDropEnabled(false);
    prgItem->setEditable(true);

    if(row==-1)
        row=groupItem->rowCount();
    groupItem->insertRow(row, prgItem);

    index = prgItem->index();

//    ValidateProgChange(index);

    return true;
}

bool ProgramsModel::GroupFromStream( QDataStream &stream, const QModelIndex &grpIndex)
{
    QString name;
    stream >> name;
    itemFromIndex(grpIndex)->setText(name);

    //send the group data to the host
    int groupId = grpIndex.data(ProgramId).toInt();
    myHost->groupContainer->ProgramFromStream( groupId, stream );
    myHost->mainWindow->mySceneView->viewGroup->ProgramFromStream( groupId, stream );

    //add the programs
    QByteArray programs;
    stream >> programs;
    QDataStream streamProgs( &programs, QIODevice::ReadOnly);
    int i=0;
    while(!streamProgs.atEnd()) {

        QModelIndex prgIndex;
        if(!AddProgram(grpIndex.row(),prgIndex,i))
            return false;

        if( !ProgramFromStream(streamProgs, prgIndex) )
            return false;
        ++i;
    }
    return true;
}

bool ProgramsModel::GroupFromStreamWithPrograms( QDataStream &stream, const QModelIndex &grpIndex)
{
    if(!GroupFromStream(stream,grpIndex))
        return false;

    //add the programs
    QByteArray programs;
    stream >> programs;
    QDataStream streamProgs( &programs, QIODevice::ReadOnly);
    int i=0;
    while(!streamProgs.atEnd()) {

        QModelIndex prgIndex;
        if(!AddProgram(grpIndex.row(),prgIndex,i))
            return false;

        if( !ProgramFromStream(streamProgs, prgIndex) )
            return false;
        ++i;
    }
    return true;
}

bool ProgramsModel::ProgramFromStream( QDataStream &stream, const QModelIndex &prgIndex)
{
    QString name;
    stream >> name;
    itemFromIndex(prgIndex)->setText(name);

    //send the data to the host
    int progId = prgIndex.data(ProgramId).toInt();
    myHost->programContainer->ProgramFromStream( progId, stream );
    myHost->mainWindow->mySceneView->viewProgram->ProgramFromStream( progId, stream );
    return true;
}

bool ProgramsModel::GroupToStream( QDataStream &stream, const QModelIndex &groupIndex) const
{
    //add the group name
    stream << groupIndex.data().toString();

    //add group datas
    int groupId = groupIndex.data(ProgramId).toInt();
    myHost->groupContainer->ProgramToStream( groupId, stream );
    myHost->mainWindow->mySceneView->viewGroup->ProgramToStream( groupId, stream );

    return true;
}

bool ProgramsModel::GroupToStreamWithPrograms( QDataStream &stream, const QModelIndex &groupIndex) const
{
    if(!GroupToStream(stream,groupIndex))
        return false;

    //add programs datas
    QByteArray programs;
    QDataStream streamProgs( &programs, QIODevice::WriteOnly);
    for(int i=0; i<rowCount(groupIndex); i++) {
        if(!ProgramToStream( streamProgs, groupIndex.child(i,0) ))
            return false;
    }
    stream << programs;
    return true;
}


bool ProgramsModel::ProgramToStream( QDataStream &stream, const QModelIndex &progIndex) const
{
    if(!progIndex.isValid()) {
        LOG("invalid index");
        return false;
    }

    //add the program name
    stream << progIndex.data().toString();

    //add the datas
    int progId = progIndex.data(ProgramId).toInt();
    myHost->programContainer->ProgramToStream( progId, stream );
    myHost->mainWindow->mySceneView->viewProgram->ProgramToStream( progId, stream );

    return true;
}

bool ProgramsModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    switch(action) {
    case Qt::CopyAction:
        currentCommandGroup = new QUndoCommand(tr("Copy programs"));
        break;
    case Qt::MoveAction:
        currentCommandGroup = new QUndoCommand(tr("Move programs"));
        break;
    default:
        LOG("unsupported action"<<action);
        return false;
    }

    int countRows=0;

    if(row==-1)
        row=rowCount(parent);

    if(data->hasFormat(MIMETYPE_GROUP)) {
        QDataStream stream( &data->data(MIMETYPE_GROUP), QIODevice::ReadOnly);
        while(!stream.atEnd()) {
            QByteArray tmpBa;
            stream >> tmpBa;
            new ComAddGroup( this, row+countRows, &tmpBa, currentCommandGroup );
            ++countRows;
        }
    } else {
        if(data->hasFormat(MIMETYPE_PROGRAM)) {
            int groupNum;
            if(parent.isValid()) {
                groupNum = parent.row();
            } else {
                //drop programs on a group
                groupNum = row;
                if( !index(groupNum,0).isValid() ) {
                    delete currentCommandGroup;
                    currentCommandGroup=0;
                    return false;
                }
                row = rowCount( index(groupNum,0) );
            }

            QDataStream stream( &data->data(MIMETYPE_PROGRAM), QIODevice::ReadOnly);
            while(!stream.atEnd()) {
                QByteArray tmpBa;
                stream >> tmpBa;
                new ComAddProgram( this, groupNum, row+countRows, &tmpBa, currentCommandGroup );
                ++countRows;
            }
        }
    }

    switch(action) {
    case Qt::CopyAction :
        emit UndoStackPush( currentCommandGroup );
        currentCommandGroup = 0;
        nbOfCommandsToGroup = 0;
        break;
    case Qt::MoveAction :
        nbOfCommandsToGroup = countRows;
        break;
    }

    return true;
}

bool ProgramsModel::removeRows( int row, int count, const QModelIndex & parent )
{
    int newCount = 0;

    //don't change prog or delete rows if a dialog is opened
    if(!openedPrompt) {

        //get the number of rows we're allowed to remove
        //and switch to a valid program if needed
        QModelIndexList list;
        for(int i=0; i<count; i++) {
            list << index(row+i,0,parent);
        }
        newCount = HowManyItemsCanWeRemoveInThisList(list);

        //issued by a command, do it
        if(fromCom) {
            if(newCount==0)
                return false;
            return removeRowsFromCommand(row,newCount,parent);
        }
    }

    //command creation in progress, add to the command group
    if(currentCommandGroup) {

        if(!openedPrompt)
            removeRowsAddToCommandStack(row,newCount,parent);

        //are we at the end of a command group ?
        nbOfCommandsToGroup-=count;
        if(nbOfCommandsToGroup==0)
            CloseCurrentCommandGroup();

        if(newCount==0)
            return false;

        return true;
    }

    //this should never happen
    LOG("remove row with no undoCommand");
    return false;
}

/*!
    issued by a command, tell the containers and views to delete the programs
    */
bool ProgramsModel::removeRowsFromCommand ( int row, int count, const QModelIndex & parent ) {

    if(parent.isValid()) {
        QStandardItem *parentItem = item(parent.row());

        //removing all the programs from a group
        if( parentItem->rowCount() == count ) {
            parentItem->setBackground(Qt::gray);
            parentItem->setToolTip( tr("This group has no program.\nDrag & drop some programs here to enable it.") );
        }

        for(int i=0; i<count; i++)
            emit ProgDelete(parent.child(row+i,0).data(ProgramsModel::ProgramId).toInt());

    } else {
        for(int i=0; i<count; i++) {
            QModelIndex groupIndex = index(row+i,0);
            int nbProg = rowCount(groupIndex);
            for(int j=0; j<nbProg; j++) {
                emit ProgDelete(groupIndex.child(j,0).data(ProgramsModel::ProgramId).toInt());
            }
            emit GroupDelete(groupIndex.data(ProgramsModel::ProgramId).toInt());
        }
    }

    SetDirty();
    currentCommandHasBeenProcessed = true;
    return QStandardItemModel::removeRows(row,count,parent);
}

/*!
  a command creation is in progress, add to the command group
  */
void ProgramsModel::removeRowsAddToCommandStack ( int row, int count, const QModelIndex & parent ) {
    for(int i=row+count-1; i>=row; i--) {
        if(parent.isValid()) {
            new ComRemoveProgram( this, i, parent.row(), currentCommandGroup);
        } else {
            new ComRemoveGroup( this, i, currentCommandGroup);
        }
    }
}

//all the pending items have been moved or the command had been canceled
void ProgramsModel::CloseCurrentCommandGroup()
{
    //don't issue the command if nothing has been deleted
    if(currentCommandHasBeenProcessed) {
        emit UndoStackPush( currentCommandGroup );
    } else {
        delete currentCommandGroup;
    }

    //reset state
    currentCommandGroup=0;
    currentCommandHasBeenProcessed=false;
}

void ProgramsModel::UserRemoveRows ( const QModelIndexList &listToRemove, const QModelIndex & parent )
{
    QList<int>rows;
    foreach(QModelIndex index, listToRemove) {
        rows << index.row();
    }

    currentCommandGroup = new QUndoCommand(tr("Remove programs"));
    nbOfCommandsToGroup = rows.size();

    qSort(rows.begin(),rows.end(),qGreater<int>());
    foreach(int r, rows)
        removeRow( r, parent);
}

bool ProgramsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    //redirect renaming to a command
    if(!fromCom && role == Qt::EditRole) {
        int groupNum=-1;
        if( index.parent().isValid() )
            groupNum=index.parent().row();

        if(index.data(role)==value)
            return true;

        emit UndoStackPush( new ComChangeProgramItem( this, index.row(), groupNum, index.data(role), value, role ) );
        return true;
    }

    return QStandardItemModel::setData(index,value,role);
}

QStringList ProgramsModel::mimeTypes () const
{
    return QStringList() << MIMETYPE_PROGRAM << MIMETYPE_GROUP;
}

QMimeData * ProgramsModel::mimeData ( const QModelIndexList & indexes ) const
{
    QByteArray groups;
    QDataStream streamGroup( &groups, QIODevice::WriteOnly);
    QByteArray programs;
    QDataStream streamProg( &programs, QIODevice::WriteOnly);

    foreach(QModelIndex index, indexes) {
        if(index.data(NodeType).toInt() == GroupNode) {
            QByteArray tmpBa;
            QDataStream tmpStream(&tmpBa, QIODevice::WriteOnly);
            GroupToStreamWithPrograms(tmpStream,index);
            streamGroup << tmpBa;
        }
        if(index.data(NodeType).toInt() == ProgramNode) {
            QByteArray tmpBa;
            QDataStream tmpStream(&tmpBa, QIODevice::WriteOnly);
            ProgramToStream(tmpStream,index);
            streamProg << tmpBa;
        }
    }
    QMimeData *mime = new QMimeData();
    if(!groups.isEmpty())
        mime->setData(MIMETYPE_GROUP, groups);
    if(!programs.isEmpty())
        mime->setData(MIMETYPE_PROGRAM, programs);
    return mime;
}

void ProgramsModel::UserChangeProg(const QModelIndex &newPrg)
{
    if(newPrg==currentPrg)
        return;

    if(!newPrg.isValid() || !newPrg.parent().isValid()) {
        LOG("invalid prog"<<newPrg.row()<<newPrg.parent().row());
        return;
    }


    if(progAutosaveState == Qt::Unchecked && myHost->programContainer->IsDirty()) {
        QUndoCommand *discardCom = new QUndoCommand( tr("Discard changes") );
        if(newPrg.parent() != currentGrp && groupAutosaveState == Qt::Unchecked && myHost->groupContainer->IsDirty())
            new ComDiscardChanges(this,currentGrp.row(),-1,discardCom);
        new ComDiscardChanges(this,currentPrg.row(),currentGrp.row(),discardCom);
        new ComChangeProgram(this,currentGrp.row(),currentPrg.row(),newPrg.parent().row(), newPrg.row(), discardCom);
        emit UndoStackPush( discardCom );
    } else {
        if(myHost->undoProgramChanges())
            emit UndoStackPush( new ComChangeProgram(this, currentGrp.row(), currentPrg.row(), newPrg.parent().row(), newPrg.row()) );
    }

    ValidateProgChange(newPrg);
}

void ProgramsModel::UserChangeGroup(const QModelIndex &newGrp)
{
    QModelIndex newPrg = newGrp.child(0,0);
    if(currentPrg.isValid() && newGrp.child( currentPrg.row(),0 ).isValid() ) {
        newPrg = newGrp.child( currentPrg.row(),0 );
    }
    UserChangeProg(newPrg);
}

void ProgramsModel::UserChangeProg(int prg)
{
    int maxPrg=rowCount(currentPrg.parent())-1;
    if(prg>maxPrg)
        prg=maxPrg;
    UserChangeProg( currentPrg.sibling(prg,0) );
}

void ProgramsModel::UserChangeGroup(int grp)
{
    int maxGrp=rowCount()-1;
    if(grp>maxGrp)
        grp=maxGrp;
    UserChangeGroup( index(grp,0) );
}

bool ProgramsModel::ValidateProgChange(const QModelIndex &newPrg, bool fromCommand)
{

//    LOG(newPrg.parent().row()<<newPrg.row()<<newPrg.data(ProgramId));

    if(!newPrg.isValid()) {
        LOG("invalid program");
        return false;
    }

    if(newPrg==currentPrg)
        return false;

    if(!fromCommand) {
        if(!mutexProgChange.tryLock()) {
            LOG("PROG CHANGE REJECTED")
            return false;
        }
        mutexProgChange.unlock();
    }

    if(!userWantsToUnloadProgram()) {
        if(!fromCommand)
            emit ProgChanged( currentPrg );
        return false;
    }

    QModelIndex newgrp = newPrg.parent();
    if(newgrp!=currentGrp) {


        if(!userWantsToUnloadGroup()) {
            if(!fromCommand)
                emit ProgChanged( currentPrg);
            return false;
        }

        if(currentGrp.isValid()) {
            itemFromIndex( currentGrp )->setBackground(Qt::transparent);
        }

        currentGrp=newgrp;

        QStandardItem *newGrpItem = itemFromIndex(currentGrp);
        if(newGrpItem) {
            newGrpItem->setBackground(currentProgColor);
        }

        if(!fromCommand)
            emit GroupChanged( currentGrp);
    }


    if(currentPrg.isValid()) {
        itemFromIndex(currentPrg)->setBackground(Qt::transparent);
    }

    itemFromIndex(newPrg)->setBackground(currentProgColor);

    currentPrg = newPrg;
    if(!fromCommand)
        emit ProgChanged( currentPrg);

    SetDirty();
    return true;
}

/*!
  change program, returns when the program is fully loaded
  */
bool ProgramsModel::ChangeProgNow(int midiGroupNum, int midiProgNum)
{
    if( !index(midiGroupNum,0).child(midiProgNum,0).isValid() )
        return false;

    //if the program has not been changed, just return
    if( !ValidateProgChange( index(midiGroupNum,0).child(midiProgNum,0), true) )
        return false;

    //if program changed, force a prog change now
    myHost->programContainer->SetProgram(currentPrg,true);
    myHost->groupContainer->SetProgram(currentGrp,true);

    return true;
}

void ProgramsModel::UserChangeProgAutosave(const Qt::CheckState state)
{
    if(fromCom) {
        progAutosaveState=state;
        SetDirty();
        emit ProgAutosaveChanged(progAutosaveState);
    } else {
        emit UndoStackPush( new ComChangeAutosave(this,1,state) );
    }
}

void ProgramsModel::UserChangeGroupAutosave(const Qt::CheckState state)
{
    if(fromCom) {
        groupAutosaveState=state;
        SetDirty();
        emit GroupAutosaveChanged(groupAutosaveState);
    } else {
        emit UndoStackPush( new ComChangeAutosave(this,0,state) );
    }
}

bool ProgramsModel::userWantsToUnloadGroup()
{
    if(openedPrompt)
        return false;

    if(groupAutosaveState == Qt::Unchecked)
        return true;

    if(!myHost->groupContainer->IsDirty())
        return true;

    if(groupAutosaveState == Qt::Checked) {
        myHost->groupContainer->SaveProgram();
        SetDirty();
        return true;
    }

    //prompt
    myHost->mainWindow->DisplayMessage(
            QMessageBox::Critical,
            tr("The group has been modified."),
            tr("Do you want to save your changes ?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Save
        );
    int res = myHost->mainWindow->GetLastMessageResult();

    openedPrompt=false;

    switch(res) {
        case QMessageBox::Cancel:
            return false;
        case QMessageBox::Save:
            myHost->groupContainer->SaveProgram();
            SetDirty();
            return true;
    }

    return true;
}

bool ProgramsModel::userWantsToUnloadProgram()
{
    if(openedPrompt)
        return false;

    if(progAutosaveState == Qt::Unchecked)
        return true;

    if(!myHost->programContainer->IsDirty())
        return true;

    if(progAutosaveState == Qt::Checked) {
        myHost->programContainer->SaveProgram();
        SetDirty();
        return true;
    }

    //prompt
    myHost->mainWindow->DisplayMessage(
            QMessageBox::Critical,
            tr("The progarm has been modified."),
            tr("Do you want to save your changes ?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Save
        );
    int res = myHost->mainWindow->GetLastMessageResult();

    openedPrompt=false;

    switch(res) {
        case QMessageBox::Cancel:
            return false;
        case QMessageBox::Save:
            myHost->programContainer->SaveProgram();
            SetDirty();
            return true;
    }

    return true;
}

bool ProgramsModel::userWantsToUnloadProject()
{
    if(openedPrompt)
        return false;

    Qt::CheckState onUnsaved = (Qt::CheckState)myHost->GetSetting("onUnsavedProject",Qt::PartiallyChecked).toInt();

    if(onUnsaved == Qt::Unchecked)
        return true;

    if(!userWantsToUnloadProgram())
        return false;

    if(!userWantsToUnloadGroup())
        return false;

    if(!IsDirty())
        return true;

    if(onUnsaved == Qt::Checked) {
        myHost->SaveProjectFile("");
        return true;
    }

    //prompt
    myHost->mainWindow->DisplayMessage(
            QMessageBox::Critical,
            tr("The project has been modified."),
            tr("Do you want to save your changes ?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Save
        );
    int res = myHost->mainWindow->GetLastMessageResult();

    openedPrompt=false;

    switch(res) {
        case QMessageBox::Cancel :
            return false;
        case QMessageBox::Save :
            myHost->SaveProjectFile("");
            return true;
    }

    return true;
}

bool ProgramsModel::userWantsToUnloadSetup()
{
    if(openedPrompt)
        return false;

    Qt::CheckState onUnsaved = (Qt::CheckState)myHost->GetSetting("onUnsavedSetup",Qt::PartiallyChecked).toInt();

    if(onUnsaved == Qt::Unchecked)
        return true;

    if(!myHost->hostContainer->IsDirty())
        return true;

    if(onUnsaved == Qt::Checked) {
        myHost->SaveSetupFile("");
        return true;
    }

    //prompt
    myHost->mainWindow->DisplayMessage(
            QMessageBox::Critical,
            tr("The setup has been modified."),
            tr("Do you want to save your changes ?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Save
        );
    int res = myHost->mainWindow->GetLastMessageResult();

    openedPrompt=false;

    switch(res) {
        case QMessageBox::Cancel :
            return false;
        case QMessageBox::Save :
            myHost->SaveSetupFile("");
            break;
        default :
            break;
    }

    return true;
}

int ProgramsModel::HowManyItemsCanWeRemoveInThisList(const QModelIndexList &list)
{
    //only one program left, just say no
    if(rowCount()==1 && rowCount( index(0,0) )==1)
        return 0;

    //current program is not in the list, it's ok
    if( !list.contains(currentPrg) && !list.contains(currentGrp) )
        return list.size();

    QModelIndex target = QModelIndex();

    if(!FindAValidProgram(currentPrg, list, target)) {
        //no place to go, remove the last program from the list and go there
        if(!list.last().parent().isValid()) {
            //the last item in the list is a group, cancel
            return 0;
        } else {
            ValidateProgChange( list.last() );
            return list.size()-1;
        }
    }

    //go to that program
    if(!ValidateProgChange(target))
        return 0;

    return list.size();
}

bool ProgramsModel::FindAValidProgramInGroup( const QModelIndex &group, int progRow, const QModelIndexList &listToAvoid , QModelIndex &target )
{
    if(!group.isValid() || rowCount(group)==0)
        return false;

    if(listToAvoid.contains(group))
        return false;

    //test the program itself
    target = index( progRow, 0, group );
    if(!listToAvoid.contains(target))
        return true;

    //test the next program
    target = group.child(progRow+1,0);// index( progRow+1, 0, group );
    if(target.isValid() && !listToAvoid.contains(target))
        return true;

    //test the previous program
    target = index( progRow-1, 0, group );
    if(target.isValid() && !listToAvoid.contains(target))
        return true;

    //test the other programs in the group
    for(int i=0; i<rowCount(group); i++) {
        target = index( i, 0, group );
        if(target.isValid() && !listToAvoid.contains(target))
            return true;
    }

    //no valid in this group
    return false;
}

/*!
  move to another program (before deletion)

  */
bool ProgramsModel::FindAValidProgram( const QModelIndex &prog, const QModelIndexList &listToAvoid, QModelIndex &target )
{
    int groupRow = prog.parent().row();
    int progRow = prog.row();

    //test the current group
    if( FindAValidProgramInGroup( index( groupRow, 0), progRow, listToAvoid, target ) )
        return true;

    //test the next group
    if( FindAValidProgramInGroup( index( groupRow+1, 0), progRow, listToAvoid, target ) )
        return true;

    //test the previous group
    if( FindAValidProgramInGroup( index( groupRow-1, 0), progRow, listToAvoid, target ) )
        return true;

    //test the groups after
    for(int i=groupRow+2; i<rowCount(); i++) {
        if( FindAValidProgramInGroup( index(i,0) , progRow, listToAvoid, target ) )
            return true;
    }

    //test the groups before
    for(int i=0; i<groupRow-1; i++) {
        if( FindAValidProgramInGroup( index(i,0), progRow, listToAvoid, target ) )
            return true;
    }

    //no valid place to go
    return false;
}

void ProgramsModel::BuildDefaultModel()
{
    clear();

    nextGroupId=1;
    nextProgId=1;

    invisibleRootItem()->setDropEnabled(true);

    for(unsigned int grp=0; grp<3; grp++) {
        int groupId = GetNextGroupId();
        QStandardItem *grpItem = new QStandardItem(QString("Grp%1").arg(grp));
        grpItem->setData(GroupNode,NodeType);
        grpItem->setData(groupId,ProgramId);
#ifndef QT_NO_DEBUG
        grpItem->setData(groupId,Qt::ToolTipRole);
#endif
        grpItem->setDragEnabled(true);
        grpItem->setDropEnabled(false);
        grpItem->setEditable(true);

        for(unsigned int prg=0; prg<5; prg++) {
            int progId = GetNextProgId();
            QStandardItem *prgItem = new QStandardItem(QString("Prg%1").arg(prg));
            prgItem->setData(ProgramNode,NodeType);
            prgItem->setData(progId,ProgramId);
#ifndef QT_NO_DEBUG
            prgItem->setData(progId,Qt::ToolTipRole);
#endif
            prgItem->setDragEnabled(true);
            prgItem->setDropEnabled(false);
            prgItem->setEditable(true);
            grpItem->appendRow(prgItem);
        }
        appendRow(grpItem);
    }
    currentGrp = item(0)->index();
    currentPrg = item(0)->child(0)->index();
    item(0)->setBackground(currentProgColor);
    item(0)->child(0)->setBackground(currentProgColor);

    emit GroupChanged( currentGrp );
    emit ProgChanged( currentPrg );

    groupAutosaveState=Qt::Checked;
    progAutosaveState=Qt::Checked;

    fromCom=true;
    emit GroupAutosaveChanged(groupAutosaveState);
    emit ProgAutosaveChanged(progAutosaveState);
    fromCom=false;

    SetDirty(false);
}


QDataStream & ProgramsModel::toStream (QDataStream &out)
{
    out << (quint16)rowCount();
    for(int i=0; i<rowCount(); i++) {
        QModelIndex grpIndex = index(i,0);
        out << grpIndex.data().toString();
        out << (quint32)grpIndex.data(ProgramId).toInt();

        out << (quint16)rowCount(grpIndex);
        for(int j=0; j<rowCount(grpIndex); j++) {
            QModelIndex prgIndex = grpIndex.child(j,0);
            out << prgIndex.data().toString();
            out << (quint32)prgIndex.data(ProgramId).toInt();
        }
    }

    out << (quint16)currentGrp.row();
    out << (quint16)currentPrg.row();
    out << (quint8)groupAutosaveState;
    out << (quint8)progAutosaveState;

    SetDirty(false);
    return out;
}

QDataStream & ProgramsModel::fromStream (QDataStream &in)
{
    clear();

    quint16 nbgrp;
    in >> nbgrp;
    for(unsigned int i=0; i<nbgrp; i++) {
        QStandardItem *grpItem = new QStandardItem();
        grpItem->setData(GroupNode,NodeType);
        grpItem->setDragEnabled(true);
        grpItem->setDropEnabled(false);
        grpItem->setEditable(true);

        QString str;
        in >> str;
        grpItem->setText(str);

        quint32 groupId;
        in >> groupId;
        if(groupId>=nextGroupId)
            nextGroupId=groupId+1;
        grpItem->setData(groupId,ProgramId);
#ifndef QT_NO_DEBUG
        grpItem->setData(groupId,Qt::ToolTipRole);
#endif

        quint16 nbprog;
        in >> nbprog;
        for(unsigned int j=0; j<nbprog; j++) {
            QStandardItem *prgItem = new QStandardItem();
            prgItem->setData(ProgramNode,NodeType);
            prgItem->setDragEnabled(true);
            prgItem->setDropEnabled(false);
            prgItem->setEditable(true);

            QString prgStr;
            in >> prgStr;
            prgItem->setText(prgStr);

            quint32 prgId;
            in >> prgId;
            if(prgId>=nextProgId)
                nextProgId=prgId+1;
            prgItem->setData(prgId,ProgramId);
#ifndef QT_NO_DEBUG
            prgItem->setData(prgId,Qt::ToolTipRole);
#endif

            grpItem->appendRow(prgItem);
        }
        appendRow(grpItem);
    }

    quint16 grp;
    in >> grp;
    quint16 prg;
    in >> prg;

    currentGrp=QModelIndex();
    currentPrg=QModelIndex();

    ValidateProgChange( index(grp,0).child(prg,0) );

    fromCom=true;
    in >> (quint8&)groupAutosaveState;
    emit GroupAutosaveChanged(groupAutosaveState);

    in >> (quint8&)progAutosaveState;
    emit ProgAutosaveChanged(progAutosaveState);
    fromCom=false;

    SetDirty(false);
    return in;
}


QDataStream & operator<< (QDataStream& out, ProgramsModel& value)
{
    return value.toStream(out);
}

QDataStream & operator>> (QDataStream& in, ProgramsModel& value)
{
    return value.fromStream(in);
}

void ProgramsModel::UpdateColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color)
{
    if(groupId==ColorGroups::Programs && colorId==Colors::HighlightBackground) {
        currentProgColor = color;
        if(currentGrp.isValid())
            itemFromIndex( currentGrp )->setBackground(currentProgColor);
        if(currentPrg.isValid())
            itemFromIndex( currentPrg )->setBackground(currentProgColor);
    }
}
