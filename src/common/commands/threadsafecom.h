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

#ifndef THREADSAFECOM_H
#define THREADSAFECOM_H

#include "precomp.h"
#include "connectables/objects/container.h"

class MainHost;
class ThreadSafeCom : public QObject, public QUndoCommand
{
    Q_OBJECT
public:
    ThreadSafeCom( MainHost *myHost, QUndoCommand  *parent=0);
    void undo ();
    void redo ();

protected:
    Q_INVOKABLE virtual void tUndo()=0;
    Q_INVOKABLE virtual void tRedo()=0;
    MainHost *myHost;
};

#endif // THREADSAFECOM_H
