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

#ifndef PROGRAMMANAGER_H
#define PROGRAMMANAGER_H

//#include "precomp.h"
#include "msghandler.h"

class Program
{
public:
	Program() : id(0) {}

    quint32 id;
    QString name;

    void GetInfos(MsgObject &msg) const {
        msg.prop[MsgObject::Name]=name;
        msg.prop[MsgObject::Id]=id;
    }
};

class Group
{
public:
	Group() : id(0) {}
    ~Group() {
        listPrograms.clear();
    }

    void GetInfos(MsgObject &msg) const {
        msg.prop[MsgObject::Name]=name;
        msg.prop[MsgObject::Id]=id;

        int cpt=0;
        foreach(const Program &prg, listPrograms) {
//            MsgObject msgProg(cpt);
            _MSGOBJ(msgProg,cpt);
            prg.GetInfos(msgProg);
            msg.children << msgProg;
            ++cpt;
        }
    }

    quint32 id;
    QString name;
    QList<Program> listPrograms;
};

class MainHost;
class ProgramManager : public QObject, public MsgHandler
{
Q_OBJECT
public:

    ProgramManager(MainHost *myHost);
    virtual ~ProgramManager();

    friend QDataStream & operator<< (QDataStream&, ProgramManager&);
    friend QDataStream & operator>> (QDataStream&, ProgramManager&);

    inline bool IsDirty() { return dirtyFlag; }
    inline void SetDirty(bool dirty=true) const { dirtyFlag=dirty; }

    inline quint16 GetNextProgId() { return nextProgId++; }
    inline quint16 GetNextGroupId() { return nextGroupId++; }

    inline quint16 GetCurrentMidiGroup() const { return currentMidiGroup; }
    inline quint16 GetCurrentMidiProg() const { return currentMidiProg; }

    bool ChangeProgNow(int midiGroupNum, int midiProgNum);
    bool ValidateProgChange(int midiGroupNum, int midiProgNum);

    bool userWantsToUnloadGroup();
    bool userWantsToUnloadProgram();
    bool userWantsToUnloadProject();
    bool userWantsToUnloadSetup();

    void BuildDefaultPrograms();

    void ReceiveMsg(const MsgObject &msg);

    const QList<Group> &GetListGroups() {return listGroups;}
    void SetListGroups(const QList<Group> &grps);

    void fromJson(QJsonObject &json);
    void toJson(QJsonObject &json) const;

private:
    void Clear();
    void UserChangeProg(quint16 prog, quint16 grp);
    void ValidateMidiChange(quint16 prog, quint16 grp);
    bool FindCurrentProg();

    QTimer updateTimer;
    bool orderChanged;

    QDataStream & toStream (QDataStream &out) const;
    QDataStream & fromStream (QDataStream &in);

    MainHost *myHost;

    QList<Group> listGroups;

    quint16 nextGroupId;
    quint16 nextProgId;
    mutable bool dirtyFlag;
    Qt::CheckState groupAutosaveState;
    Qt::CheckState progAutosaveState;

    quint16 currentMidiGroup;
    quint16 currentMidiProg;
    quint32 currentGroupId;
    quint32 currentProgId;

    int promptAnswer;

    int WaitPromptAnswer(const QString &type);

signals:
    void ProgChanged(quint32 progId);
    void GroupChanged(quint32 grpId);
    void ProgDelete(quint32 progId);
    void GroupDelete(quint32 grpId);
    void MidiProgChanged(quint16 prog);
    void MidiGroupChanged(quint16 grp);

public slots:
    void UserChangeGroup(quint16 grp);
    void UserChangeProg(quint16 prog);

private slots:
    void UpdateView();

friend class ComChangeAutosave;
};

QDataStream & operator<< (QDataStream& out, ProgramManager& value);
QDataStream & operator>> (QDataStream& in, ProgramManager& value);

#endif // PROGRAMMANAGER_H
