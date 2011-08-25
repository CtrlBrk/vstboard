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

#ifndef PROGRAMSMODEL_H
#define PROGRAMSMODEL_H
#include "precomp.h"
#include "viewconfig.h"
#include "debugmutex.h"

class MainHost;
class ProgramsModel : public QStandardItemModel
{
    Q_OBJECT
public:

    enum ItemTypesEnum {
        ProgramNode,
        GroupNode
    };
    enum RolesEnum{
        NodeType = Qt::UserRole+1,
        ProgramId
    };

    explicit ProgramsModel(MainHost *myHost, QObject *parent = 0);
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    bool removeRows ( int row, int count, const QModelIndex & parent = QModelIndex() );
    bool setData(const QModelIndex &index, const QVariant &value, int role);

    QStringList mimeTypes () const;
    QMimeData * mimeData ( const QModelIndexList & indexes ) const;

    bool userWantsToUnloadGroup();
    bool userWantsToUnloadProgram();
    bool userWantsToUnloadProject();
    bool userWantsToUnloadSetup();

    inline int GetCurrentMidiGroup() const {
        if(!currentGrp.isValid())
            return 0;
        return currentGrp.row();
    }

    inline int GetCurrentMidiProg() const {
        if(!currentPrg.isValid())
            return 0;
        return currentPrg.row();
    }

    void BuildDefaultModel();

    bool GetLastDialogAnswer() {return lastDialogAnswer;}

    Qt::CheckState GetProgAutosaveState() const {return progAutosaveState;}
    Qt::CheckState GetGroupAutosaveState() const {return groupAutosaveState;}
    const QModelIndex &GetCurrentProg() const {return currentPrg;}

    DMutex mutexProgChange;

private:
    bool AddGroup(QModelIndex &index=QModelIndex(), int row=-1);
    bool AddProgram(int groupNum, QModelIndex &index=QModelIndex(), int row=-1);

    bool GroupFromStream( QDataStream &stream, const QModelIndex &grpIndex);
    bool GroupFromStreamWithPrograms( QDataStream &stream, const QModelIndex &grpIndex);
    bool ProgramFromStream( QDataStream &stream, const QModelIndex &prgIndex);
    bool GroupToStream( QDataStream &stream, const QModelIndex &groupIndex) const;
    bool GroupToStreamWithPrograms( QDataStream &stream, const QModelIndex &groupIndex) const;
    bool ProgramToStream( QDataStream &stream, const QModelIndex &progIndex) const;

    void removeRowsAddToCommandStack ( int row, int count, const QModelIndex & parent = QModelIndex() );
    void CloseCurrentCommandGroup();
    bool removeRowsFromCommand ( int row, int count, const QModelIndex & parent = QModelIndex() );


    int HowManyItemsCanWeRemoveInThisList(const QModelIndexList &progList);
    bool FindAValidProgramInGroup( const QModelIndex &group, int progRow, const QModelIndexList &listToAvoid, QModelIndex &target );
    bool FindAValidProgram( const QModelIndex &prog, const QModelIndexList &listToAvoid, QModelIndex &target = QModelIndex() );

    bool ValidateProgChange(const QModelIndex &newPrg, bool fromCommand=false);
    bool ChangeProgNow(int midiGroupNum, int midiProgNum);

    QDataStream & toStream (QDataStream &);
    QDataStream & fromStream (QDataStream &);

    inline bool IsDirty() {
        return dirtyFlag;
    }
    inline void SetDirty(bool dirty=true) {
        dirtyFlag=dirty;
    }

    inline uint GetNextProgId() {
        return nextProgId++;
    }

    inline uint GetNextGroupId() {
        return nextGroupId++;
    }


    MainHost *myHost;
    QPersistentModelIndex currentGrp;
    QPersistentModelIndex currentPrg;
    unsigned int nextGroupId;
    unsigned int nextProgId;
    Qt::CheckState progAutosaveState;
    Qt::CheckState groupAutosaveState;
    bool dirtyFlag;
    QBrush currentProgColor;
    bool fromCom;
    QUndoCommand *currentCommandGroup;
    int nbOfCommandsToGroup;
    bool openedPrompt;
    bool currentCommandHasBeenProcessed;

    bool lastDialogAnswer;

    friend class ComAddProgram;
    friend class ComAddGroup;
    friend class ComRemoveProgram;
    friend class ComRemoveGroup;
    friend class ComChangeProgram;
    friend class ComChangeProgramItem;
    friend class ComAddCable;
    friend class ComDisconnectPin;
    friend class ComAddObject;
    friend class ComRemoveObject;
    friend class ComDiscardChanges;
    friend class ComChangeAutosave;
    friend class ComAddPin;
    friend class ComRemovePin;

    friend QDataStream & operator<< (QDataStream&, ProgramsModel&);
    friend QDataStream & operator>> (QDataStream&, ProgramsModel&);

signals:
    void UndoStackPush(QUndoCommand *cmd);

    void ProgChanged(const QModelIndex &prgIndex, bool fromCom=false);
    void GroupChanged(const QModelIndex &grpIndex, bool fromCom=false);
    void ProgDelete(int progId);
    void GroupDelete(int groupId);

    void ProgAutosaveChanged(const Qt::CheckState state);
    void GroupAutosaveChanged(const Qt::CheckState state);

    void NewProgramAdded(const QModelIndex &prgIndex);

public slots:
    void UserChangeProg(const QModelIndex &newPrg);
    void UserChangeGroup(const QModelIndex &newGrp);
    void UserChangeProg(int prg);
    void UserChangeGroup(int grp);
    void UserChangeProgAutosave(const Qt::CheckState state);
    void UserChangeGroupAutosave(const Qt::CheckState state);
    void UserRemoveRows ( const QModelIndexList &listToRemove, const QModelIndex & parent = QModelIndex() );
    void UserAddGroup(int row=-1);
    void UserAddProgram(const QModelIndex &grpIndex, int row=-1);

    void UpdateColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color);


    void asyncUserWantsToUnloadProject() {lastDialogAnswer = userWantsToUnloadProject();}
    void asyncUserWantsToUnloadSetup() {lastDialogAnswer = userWantsToUnloadSetup();}
};

QDataStream & operator<< (QDataStream& out, ProgramsModel& value);
QDataStream & operator>> (QDataStream& in, ProgramsModel& value);

#endif // PROGRAMSMODEL_H
