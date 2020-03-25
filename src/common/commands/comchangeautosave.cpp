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

#include "comchangeautosave.h"
#include "programmanager.h"

ComChangeAutosave::ComChangeAutosave(ProgramManager *model,
                                     int type,
                                     Qt::CheckState newState,
                                     QUndoCommand *parent) :
    QUndoCommand(parent),
    model(model),
    type(type),
	oldState(Qt::Unchecked),
	newState(newState)
{
    setText(QObject::tr("Change Autosave"));

    if(type==0) {
        oldState = model->groupAutosaveState;
    }
    if(type==1) {
        oldState = model->progAutosaveState;
    }
}

void ComChangeAutosave::undo()
{
    MsgObject msg(model->GetIndex());

    if(type==0) {
        model->groupAutosaveState = oldState;
        msg.prop[MsgObject::GroupAutosave] = oldState;
    }
    if(type==1) {
        model->progAutosaveState = oldState;
        msg.prop[MsgObject::ProgAutosave] = oldState;
    }

    model->msgCtrl->SendMsg(msg);
}

void ComChangeAutosave::redo()
{
    MsgObject msg(model->GetIndex());

    if(type==0) {
        model->groupAutosaveState = newState;
        msg.prop[MsgObject::GroupAutosave]=newState;
    }
    if(type==1) {
        model->progAutosaveState = newState;
        msg.prop[MsgObject::ProgAutosave] = newState;
    }

    model->msgCtrl->SendMsg(msg);
}
