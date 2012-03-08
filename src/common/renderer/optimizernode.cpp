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

#include "optimizernode.h"
#include "solvernode.h"
#include "renderernode2.h"

OptimizerNode::OptimizerNode(int id, const SolverNode &c) :
    id(id),
    minRenderOrder(c.minRenderOrder),
    maxRenderOrder(c.maxRenderOrder),
    cpuTime(0)
{
    Init();
}

OptimizerNode::OptimizerNode(const OptimizerNode &c) :
    id(c.id),
    minRenderOrder(c.minRenderOrder),
    maxRenderOrder(c.maxRenderOrder),
    possiblePositions(c.possiblePositions),
    cpuTime(c.cpuTime),
    selectedPos(c.selectedPos)
{

}

OptimizerNode::OptimizerNode(const RendererNode2 &c) :
    id(c.id),
    minRenderOrder(c.minRenderOrderOri),
    maxRenderOrder(c.maxRenderOrderOri),
    cpuTime(c.cpuTime)
{
    Init();
}

void OptimizerNode::Init()
{
    for(int i=minRenderOrder; i<=maxRenderOrder; i++) {
        for(int j=maxRenderOrder; j>=i; j--) {
            possiblePositions << NodePos(i,j);
        }
    }
}
