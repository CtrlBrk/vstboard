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

#ifndef RENDERMAP_H
#define RENDERMAP_H

//class RendererNode2;
#include "renderernode2.h"
typedef QList< QSharedPointer<RendererNode2> > RenderStep;
typedef QMap<int, RenderStep > ThreadNodes;
typedef QMap<int, ThreadNodes > RendererMap;

class OptimizerNode;
typedef QMap<int, QMap<int, QList<OptimizerNode > > > OptMap;

class SolverNode;
#include "msgobject.h"

class RenderMap
{
public:
	RenderMap() {}
	RenderMap(const RenderMap &c) { *this = c; }
    RenderMap(const OptMap &oMap, const QList<SolverNode*> &solverNodes);

//    ~RenderMap();
//    void Clear();
    QString ToTxt();
    void GetInfo(MsgObject &msg) const;

//    RenderMap & operator= (const RenderMap & c);

    RendererMap map;
};

#endif // RENDERMAP_H
