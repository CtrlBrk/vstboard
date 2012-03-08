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

#ifndef COMREMOVEGROUP_H
#define COMREMOVEGROUP_H

class MainHost;
class ComRemoveGroup : public QUndoCommand
{
public:
    ComRemoveGroup(MainHost *myHost,
                   int prgId,
                   QUndoCommand *parent=0);
    void undo();
    void redo();

private:
    MainHost *myHost;
    int prgId;
    QByteArray data;
};

#endif // COMREMOVEGROUP_H
