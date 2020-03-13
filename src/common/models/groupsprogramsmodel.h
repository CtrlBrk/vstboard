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

#ifndef GROUPSPROGRAMSMODEL_H
#define GROUPSPROGRAMSMODEL_H

#include "precomp.h"
#include "msghandler.h"
#include "views/viewconfig.h"

#define MIMETYPE_GROUP QLatin1String("application/x-groupsdata")
#define MIMETYPE_PROGRAM QLatin1String("application/x-programsdata")

class GroupsProgramsModel : public QStandardItemModel, public MsgHandler
{
    Q_OBJECT
public:
    GroupsProgramsModel( MsgController *msgCtrl, QObject *parent=0 );
    void ReceiveMsg(const MsgObject &msg);

    QStringList mimeTypes () const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    bool removeRows(int row, int count, const QModelIndex &parent);
    bool insertRows(int row, int count, const QModelIndex &parent);
    QMimeData * mimeData ( const QModelIndexList & indexes ) const;

private:
    QPersistentModelIndex currentGroup;
    QPersistentModelIndex currentProg;
    QBrush currentProgColor;
    QTimer updateTimer;
    bool orderChanged;
    quint16 currentMidiGroup;
    quint16 currentMidiProgram;

signals:
    void GroupChanged(const QModelIndex &idx);
    void ProgChanged(const QModelIndex &idx);
    void ProgAutosaveChanged(const Qt::CheckState state);
    void GroupAutosaveChanged(const Qt::CheckState state);

public slots:
    void Update();
    void UserChangeProg(const QModelIndex &idx);
//    void UserRemoveGroup(const QModelIndexList &grp);
//    void UserRemoveProg(const QModelIndexList &prg);
//    void UserAddGroup(const int row);
//    void UserAddProgram(const int row);
    void UpdateColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color);
    void UserChangeProgAutosave(const Qt::CheckState state);
    void UserChangeGroupAutosave(const Qt::CheckState state);

private slots:
    void SendUpdateToHost();
};

#endif // GROUPSPROGRAMSMODEL_H
