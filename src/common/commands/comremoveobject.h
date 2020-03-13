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

#ifndef COMREMOVEOBJECT_H
#define COMREMOVEOBJECT_H

#include "precomp.h"
#include <QUndoCommand>
#include "connectables/container.h"


class MainHost;
class ComRemoveObject : public QUndoCommand
{
public:
    ComRemoveObject( MainHost *myHost,
                     const QSharedPointer<Connectables::Object> &objPtr,
                     RemoveType::Enum removeType,
                     QUndoCommand *parent=0);
    void undo ();
    void redo ();

private:
    MainHost *myHost;

    ObjectInfo objectInfo;
    QWeakPointer<Connectables::Container>ContainerPtr;
    RemoveType::Enum removeType;
    ObjectContainerAttribs attr;

    QList< QPair<ConnectionInfo,ConnectionInfo> >listAddedCables;
    QList< QPair<ConnectionInfo,ConnectionInfo> >listRemovedCables;

    QByteArray objState;

    int currentGroup;
    int currentProg;
};

#endif // COMREMOVEOBJECT_H
