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

#ifndef OPTIMIZERNODE_H
#define OPTIMIZERNODE_H

#include "node.h"

class NodePos
{
public:
    NodePos(int startStep=-1, int endStep=-1) : startStep(startStep), endStep(endStep) {}
    int startStep;
    int endStep;
};

class SolverNode;
class RendererNode2;
class OptimizerNode
{
public:
    OptimizerNode(int id, const SolverNode &c);
    OptimizerNode(const OptimizerNode &c);
    OptimizerNode(const RendererNode2 &c);
//#ifdef TESTING
    OptimizerNode(int id, long cpuTime, int minRenderOrder, int maxRenderOrder) :  id(id), minRenderOrder(minRenderOrder) ,maxRenderOrder(maxRenderOrder), cpuTime(cpuTime) { Init(); }
//#endif

    int id;
    int minRenderOrder;
    int maxRenderOrder;
    QList<NodePos>possiblePositions;
    long cpuTime;
    NodePos selectedPos;

    bool operator ==(const OptimizerNode &c) const {
        return (c.id==id);
    }

    static bool CompareCpuUsage(OptimizerNode *n1, OptimizerNode *n2)
    {
        return n1->cpuTime < n2->cpuTime;
    }

private:
    void Init();
};

#endif // OPTIMIZERNODE_H
