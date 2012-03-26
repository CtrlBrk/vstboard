/**************************************************************************
#    Copyright 2010-2012 Raphaël François
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

#include "programmanager.h"
#include "mainhost.h"
#include "commands/comchangeautosave.h"
#include "commands/comaddgroup.h"
#include "commands/comaddprogram.h"
#include "commands/comremovegroup.h"
#include "commands/comremoveprogram.h"
#include "commands/comprogramstate.h"
#include "connectables/containerprogram.h"

ProgramManager::ProgramManager(MainHost *myHost) :
    QObject(myHost),
    MsgHandler(myHost,FixedObjId::programsManager),
    myHost(myHost),
    nextGroupId(1),
    nextProgId(1),
    currentGroupId(0),
    currentProgId(0),
    currentMidiGroup(0),
    currentMidiProg(0),
    dirtyFlag(false),
    promptAnswer(-1)
{
    updateTimer.setInterval(20);
    updateTimer.setSingleShot(true);
    connect(&updateTimer, SIGNAL(timeout()),
            this, SLOT(UpdateView()));
}

ProgramManager::~ProgramManager()
{
    Clear();
}

void ProgramManager::Clear()
{
    nextGroupId=1;
    nextProgId=1;
    currentGroupId=0;
    currentProgId=0;
    currentMidiGroup=0;
    currentMidiProg=0;
    dirtyFlag=false;
    listGroups.clear();
}

void ProgramManager::ReceiveMsg(const MsgObject &msg)
{
    SetMsgEnabled(true);

    if(msg.prop.contains(MsgObject::GetUpdate)) {
        orderChanged=true;
        updateTimer.start();
        return;
    }

    if(msg.prop.contains(MsgObject::Update)) {

        QUndoCommand *undoCom = new QUndoCommand();

//        QList<Group> oldListGroups = listGroups;
        QList<Group> newListGrp;
//        listGroups.clear();
        QList<int>listGrpId;
        QList<int>listPrgId;

        foreach(const MsgObject &msgGrp, msg.children) {
            Group grp;
            grp.name = msgGrp.prop[MsgObject::Name].toString();

            if(msgGrp.prop.contains(MsgObject::Id)) {
                grp.id = msgGrp.prop[MsgObject::Id].toInt();

                //this is a copy, make a new group
                if(listGrpId.contains(grp.id)) {
                    QByteArray bArray;
                    QDataStream stream(&bArray, QIODevice::WriteOnly);
                    myHost->groupContainer->ProgramToStream(grp.id, stream);
                    grp.id = GetNextGroupId();
                    new ComAddGroup(myHost,grp.id,bArray,undoCom);
                    grp.name += "'";
//                    orderChanged=true;
                }
                listGrpId << grp.id;

            } else {
//                orderChanged=true;
                grp.id = GetNextGroupId();
            }

            if(msgGrp.children.isEmpty()) {
                //create a default program in the group
                Program prg;
                prg.id = GetNextProgId();
                grp.listPrograms << prg;
//                orderChanged=true;
            } else {
                foreach(const MsgObject &msgPrg, msgGrp.children) {
                    Program prg;
                    prg.name = msgPrg.prop[MsgObject::Name].toString();

                    if(msgPrg.prop.contains(MsgObject::Id)) {
                        prg.id = msgPrg.prop[MsgObject::Id].toInt();

                        //this is a copy, make a new prog
                        if(listPrgId.contains(prg.id)) {
                            QByteArray bArray;
                            QDataStream stream(&bArray, QIODevice::WriteOnly);
                            myHost->programContainer->ProgramToStream(prg.id, stream);
                            prg.id = GetNextProgId();
                            new ComAddProgram(myHost,prg.id,bArray,undoCom);
                            prg.name += "'";
//                            orderChanged=true;
                        }
                        listPrgId << prg.id;

                    } else {
//                        orderChanged=true;
                        prg.id = GetNextProgId();
                    }
                    grp.listPrograms << prg;
                }
            }

            newListGrp << grp;
        }

        new ComProgramState(myHost,newListGrp,undoCom);

        //find deleted progs
        foreach(const Group &grp, listGroups) {

            foreach(const Program &prg, grp.listPrograms) {
                if( !listPrgId.contains(prg.id)) {
                    new ComRemoveProgram(myHost,prg.id,undoCom);
                }
            }

            if( !listGrpId.contains(grp.id) ) {
                new ComRemoveGroup(myHost,grp.id,undoCom);
            }
        }

//        oldListGroups.clear();

        myHost->undoStack.push(undoCom);

//        if(orderChanged)
//            updateTimer.start();

        //find the current prog
//        if(FindCurrentProg())
            return;
    }

    if(msg.prop.contains(MsgObject::Group)) {
        int grp = msg.prop[MsgObject::Group].toInt();
        UserChangeGroup(grp);
    }
    if(msg.prop.contains(MsgObject::Program)) {
        int prg = msg.prop[MsgObject::Program].toInt();
        UserChangeProg(prg);
    }

    if(msg.prop.contains(MsgObject::Answer)) {
        promptAnswer=msg.prop[MsgObject::Answer].toInt();
    }

    if(msg.prop.contains(MsgObject::ProgAutosave)) {
        myHost->undoStack.push( new ComChangeAutosave(this,1,static_cast<Qt::CheckState>(msg.prop[MsgObject::ProgAutosave].toInt())) );
    }
    if(msg.prop.contains(MsgObject::GroupAutosave)) {
        myHost->undoStack.push( new ComChangeAutosave(this,0,static_cast<Qt::CheckState>(msg.prop[MsgObject::GroupAutosave].toInt())) );
    }
}

void ProgramManager::SetListGroups(const QList<Group> &grps)
{
    listGroups = grps;
    UserChangeProg(currentMidiProg, currentMidiGroup);
    orderChanged=true;
    updateTimer.start();
}

void ProgramManager::UpdateView()
{
    if(!MsgEnabled())
        return;

    MsgObject msg(GetIndex());

    if(orderChanged) {
        orderChanged=false;
        msg.prop[MsgObject::Update]=1;

        int cpt=0;
        foreach(const Group &grp, listGroups) {
            MsgObject msgGrp(cpt);
            grp.GetInfos(msgGrp);
            msg.children << msgGrp;
            ++cpt;
        }

        msg.prop[MsgObject::GroupAutosave] = groupAutosaveState;
        msg.prop[MsgObject::ProgAutosave] = progAutosaveState;
    }

    msg.prop[MsgObject::Group]=currentMidiGroup;
    msg.prop[MsgObject::Program]=currentMidiProg;

    msgCtrl->SendMsg(msg);
}

void ProgramManager::BuildDefaultPrograms()
{
    Clear();

    for(unsigned int grpn=0; grpn<3; grpn++) {
        Group grp;
        grp.name = QString("Grp%1").arg(grpn);
        grp.id = GetNextGroupId();

        for(unsigned int prgn=0; prgn<5; prgn++) {
            Program prg;
            prg.name = QString("Prg%1").arg(prgn);
            prg.id = GetNextProgId();
            grp.listPrograms << prg;
        }
        listGroups << grp;
    }

    groupAutosaveState=Qt::Checked;
    progAutosaveState=Qt::Checked;

    currentMidiGroup=127;
    currentMidiProg=127;
    if(!ChangeProgNow(0,0)) {
        LOG("default prog not loaded")
    }
    SetDirty(false);

    orderChanged=true;
    updateTimer.start();
}

bool ProgramManager::userWantsToUnloadGroup()
{
    //drop changes
    if(groupAutosaveState == Qt::Unchecked)
        return true;

    //no changes
    //plugins internal state may ahve changed
//    if(!myHost->groupContainer->IsDirty())
//        return true;

    //auto save
    if(groupAutosaveState == Qt::Checked) {
        myHost->groupContainer->SaveProgram();
        SetDirty();
        return true;
    }

    //prompt user
    switch(WaitPromptAnswer(tr("group"))) {
        case QMessageBox::Cancel:
            return false;
        case QMessageBox::Save:
            myHost->groupContainer->SaveProgram();
            SetDirty();
            return true;
        case QMessageBox::Discard:
            return true;
    }
    return true;
}

bool ProgramManager::userWantsToUnloadProgram()
{
    //drop changes
    if(progAutosaveState == Qt::Unchecked)
        return true;

    //no changes
    //plugins internal state may ahve changed
//    if(!myHost->programContainer->IsDirty())
//        return true;

    //auto save
    if(progAutosaveState == Qt::Checked) {
        myHost->programContainer->SaveProgram();
        SetDirty();
        return true;
    }

    //prompt user
    switch(WaitPromptAnswer(tr("program"))) {
        case QMessageBox::Cancel:
            return false;
        case QMessageBox::Save:
            myHost->programContainer->SaveProgram();
            SetDirty();
            return true;
        case QMessageBox::Discard:
            return true;
    }
    return true;
}

bool ProgramManager::userWantsToUnloadProject()
{
    Qt::CheckState onUnsaved = (Qt::CheckState)myHost->settings->GetSetting("onUnsavedProject",Qt::PartiallyChecked).toInt();

    //drop changes
    if(onUnsaved == Qt::Unchecked)
        return true;

    //unload current program
    if(!userWantsToUnloadProgram())
        return false;

    //unload current group
    if(!userWantsToUnloadGroup())
        return false;

    //no changes
//    if(!IsDirty())
//        return true;

    //auto save
    if(onUnsaved == Qt::Checked) {
        myHost->SaveProjectFile();
        return true;
    }

    //prompt user
    switch(WaitPromptAnswer(tr("project"))) {
        case QMessageBox::Cancel:
            return false;
        case QMessageBox::Save:
            return myHost->SaveProjectFile();
        case QMessageBox::Discard:
            return true;
    }
    return true;
}

bool ProgramManager::userWantsToUnloadSetup()
{
    Qt::CheckState onUnsaved = (Qt::CheckState)myHost->settings->GetSetting("onUnsavedSetup",Qt::PartiallyChecked).toInt();

    //drop changes
    if(onUnsaved == Qt::Unchecked)
        return true;

    //no changes
//    if(!myHost->hostContainer->IsDirty())
//        return true;

    //auto save
    if(onUnsaved == Qt::Checked) {
        myHost->SaveSetupFile();
        return true;
    }

    //prompt user
    switch(WaitPromptAnswer(tr("setup"))) {
        case QMessageBox::Cancel:
            return false;
        case QMessageBox::Save:
            return myHost->SaveSetupFile();
        case QMessageBox::Discard:
            return true;
    }
    return true;
}

int ProgramManager::WaitPromptAnswer(const QString &type)
{
    //prompt already opened
    if(promptAnswer!=-1)
        return QMessageBox::Cancel;

    //ask
    MsgObject msg(GetIndex());
    msg.prop[MsgObject::Message]=type;
    msgCtrl->SendMsg(msg);

    //wait for answer
    while(promptAnswer==-1) {
        Sleep(50);
        qApp->processEvents();
    }

    int r=promptAnswer;
    promptAnswer=-1;
    return r;
}

void ProgramManager::UserChangeGroup(quint16 grp)
{
    if(listGroups.count()<=grp)
        grp=listGroups.count()-1;

    quint16 newPrg=currentMidiProg;
    quint16 nbPrg = listGroups[grp].listPrograms.count();
    if(newPrg>=nbPrg)
        newPrg=nbPrg-1;

    UserChangeProg(newPrg,grp);
}

void ProgramManager::UserChangeProg(quint16 prog, quint16 grp)
{
    ValidateProgChange(grp,prog);
}

void ProgramManager::UserChangeProg(quint16 prog)
{
    quint16 nbPrg = listGroups[currentMidiGroup].listPrograms.count();
    if(prog>=nbPrg)
        prog=nbPrg-1;

    UserChangeProg(prog,currentMidiGroup);
}

bool ProgramManager::ChangeProgNow(int midiGroupNum, int midiProgNum)
{
    //if the program has not been changed, just return
    if( !ValidateProgChange( midiGroupNum, midiProgNum ) )
        return false;

    //if program changed, force the host to update
    myHost->UpdateSolverNow();
    return true;
}

bool ProgramManager::ValidateProgChange(int midiGroupNum, int midiProgNum)
{
    if(midiGroupNum<0 || midiProgNum<0)
        return false;

    if(listGroups.count()<=midiGroupNum)
        midiGroupNum=0;

    if(listGroups.count()==0)
        return false;

    if(listGroups[midiGroupNum].listPrograms.count()<=midiProgNum)
        midiProgNum=0;

    if(listGroups[midiGroupNum].listPrograms.count()==0)
        return false;

    if(listGroups[midiGroupNum].id==currentGroupId && listGroups[midiGroupNum].listPrograms[midiProgNum].id==currentProgId)
        return false;

    if(!userWantsToUnloadProgram()) {
//        emit ProgChanged( currentPrg );
        return false;
    }

    if(listGroups[midiGroupNum].id!=currentGroupId) {

        if(!userWantsToUnloadGroup()) {
//            emit ProgChanged( currentPrg );
            return false;
        }

        currentGroupId = listGroups[midiGroupNum].id;
        emit GroupChanged( currentGroupId );
    }

    if(listGroups[midiGroupNum].listPrograms[midiProgNum].id!=currentProgId) {
        currentProgId = listGroups[midiGroupNum].listPrograms[midiProgNum].id;
        emit ProgChanged( currentProgId );

        SetDirty();
        ValidateMidiChange(midiProgNum,midiGroupNum);
    }

    return true;
}

void ProgramManager::ValidateMidiChange(quint16 prog, quint16 grp)
{
    if(grp!=currentMidiGroup) {
        currentMidiGroup=grp;
        emit MidiGroupChanged(currentMidiGroup);
    }
    if(prog!=currentMidiProg) {
        currentMidiProg=prog;
        emit MidiProgChanged(currentMidiProg);
    }

    updateTimer.start();
}

bool ProgramManager::FindCurrentProg()
{
    int grpCpt=0;
    foreach(const Group &grp, listGroups) {
        int prgCpt=0;
        foreach(const Program &prg, grp.listPrograms) {
            if(prg.id==currentProgId) {
                ValidateMidiChange(prgCpt,grpCpt);
                return true;
            }
            prgCpt++;
        }
        ++grpCpt;
    }
    return false;
}

QDataStream & ProgramManager::toStream (QDataStream &out)
{
    out << (quint16)listGroups.count();
    foreach(const Group &grp, listGroups) {
        out << grp.name;
        out << grp.id;


        out << (quint16)grp.listPrograms.count();
        foreach(const Program &prg, grp.listPrograms) {
            out << prg.name;
            out << prg.id;
        }
    }

    out << (quint16)currentMidiGroup;
    out << (quint16)currentMidiProg;
    out << (quint8)groupAutosaveState;
    out << (quint8)progAutosaveState;

    SetDirty(false);
    return out;
}

QDataStream & ProgramManager::fromStream (QDataStream &in)
{
    Clear();

    quint16 nbgrp;
    in >> nbgrp;

    for(unsigned int i=0; i<nbgrp; i++) {

        Group grp;

        in >> grp.name;
        in >> grp.id;
        if(grp.id>=nextGroupId)
            nextGroupId=grp.id+1;

        quint16 nbprog;
        in >> nbprog;
        for(unsigned int j=0; j<nbprog; j++) {

            Program prg;
            in >> prg.name;
            in >> prg.id;
            if(prg.id>=nextProgId)
                nextProgId=prg.id+1;

            grp.listPrograms << prg;
        }

        listGroups << grp;
    }

    currentMidiGroup=127;
    currentMidiProg=127;
    quint16 grp;
    quint16 prg;
    in >> grp;
    in >> prg;
    if(!ChangeProgNow(grp,prg)) {
        LOG("saved prog not loaded")
    }

    in >> (quint8&)groupAutosaveState;
    in >> (quint8&)progAutosaveState;

    SetDirty(false);

    orderChanged=true;
    updateTimer.start();

    return in;
}

QDataStream & operator<< (QDataStream& out, ProgramManager& value)
{
    return value.toStream(out);
}

QDataStream & operator>> (QDataStream& in, ProgramManager& value)
{
    return value.fromStream(in);
}
