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

#include "comremoveprogram.h"
#include "mainhost.h"

ComRemoveProgram::ComRemoveProgram(MainHost *myHost,
                                   int prgId,
                                   QUndoCommand *parent) :
    QUndoCommand(parent),
    myHost(myHost),
    prgId(prgId)

{
    setText(QObject::tr("Remove program"));
}

void ComRemoveProgram::undo()
{
    QDataStream stream(&data, QIODevice::ReadOnly);
    myHost->programContainer->ProgramFromStream(prgId, stream);
}

void ComRemoveProgram::redo()
{
    QDataStream stream(&data, QIODevice::WriteOnly);
    myHost->programContainer->ProgramToStream(prgId, stream);
    myHost->programContainer->RemoveProgram(prgId);
}
