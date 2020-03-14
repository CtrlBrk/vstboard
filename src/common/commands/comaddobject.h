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

#ifndef COMADDOBJECT_H
#define COMADDOBJECT_H

#include "precomp.h"
#include <QUndoCommand>
#include "connectables/container.h"

class MainHost;
class ComAddObject : public QUndoCommand
{
public:
    ComAddObject(MainHost *myHost,
                 const ObjectInfo & objInfo,
                 const QSharedPointer<Connectables::Container> &container,
                 const QSharedPointer<Connectables::Object> &targetObj,
                 InsertionType::Enum insertType = InsertionType::NoInsertion,
                 QUndoCommand  *parent=0);
    void undo ();
    void redo ();

    void ReloadObject(const ObjectInfo &info);

private:
    MainHost *myHost;

    ObjectInfo objectInfo;
    ObjectInfo targetInfo;

    QWeakPointer<Connectables::Container>ContainerPtr;
    InsertionType::Enum insertType;

    QList< QPair<ConnectionInfo,ConnectionInfo> >listAddedCables;
    QList< QPair<ConnectionInfo,ConnectionInfo> >listRemovedCables;

    ObjectContainerAttribs attr;
    QByteArray objState;

    ObjectContainerAttribs targetAttr;
    QByteArray targetState;

    int currentGroup;
    int currentProg;
};

#endif // COMADDOBJECT_H
