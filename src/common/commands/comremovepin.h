/**************************************************************************
#    Copyright 2010-2025 Raphaël François
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

#ifndef COMREMOVEPIN_H
#define COMREMOVEPIN_H

#include <QUndoCommand>
#include "connectables/connectioninfo.h"

class MainHost;
class ComRemovePin : public QUndoCommand
{
public:
    ComRemovePin(MainHost *myHost,
                 const ConnectionInfo &pinInfo,
                 QUndoCommand  *parent=0);
    void undo ();
    void redo ();
private:
    MainHost *myHost;
    ConnectionInfo pinInfo;
    int currentGroup;
    int currentProg;
    QList<ConnectionInfo>listConnectedPins;
    QVariant value;
};

#endif // COMREMOVEPIN_H
