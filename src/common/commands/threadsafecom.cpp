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

#include "mainhost.h"
#include "threadsafecom.h"

ThreadSafeCom::ThreadSafeCom( MainHost *myHost, QUndoCommand  *parent) :
    QObject(),
    QUndoCommand(parent),
    myHost(myHost)
{}

void ThreadSafeCom::undo ()
{
    if(QThread::currentThread()!=myHost->thread()) {
        QMetaObject::invokeMethod(myHost,"invokeUndo",Qt::QueuedConnection,Q_ARG(QUndoCommand*, this));
    } else {
        tUndo();
    }
}

void ThreadSafeCom::redo ()
{
    if(QThread::currentThread()!=myHost->thread()) {
        QMetaObject::invokeMethod(myHost,"invokeRedo",Qt::QueuedConnection,Q_ARG(QUndoCommand*, this));
    } else {
        tRedo();
    }
}
