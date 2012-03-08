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

#ifndef SOLVERNODE_H
#define SOLVERNODE_H

#include "node.h"

class PathSolver;
class RendererNode2;
class SolverNode : public Node
{
public:
    SolverNode();
    SolverNode(const SolverNode &c);
    SolverNode(const RendererNode2 &c);

protected:
    void ReconnectChildsTo(SolverNode *newParent);
    void ReconnectParentsTo(SolverNode *newChild);
    bool DetectLoopback(QList<SolverNode*>&listLoop);
    bool DistanceFromRoot(int &steps);
    bool DistanceFromTail(int &steps);
    bool IsRoot();
    bool IsTail();

    void AddChild(SolverNode *child);
    void AddParent(SolverNode *parent);
    void RemoveChild(SolverNode *child);
    void RemoveParent(SolverNode *parent);
    int SetMinRenderOrder(int order);
    void SetMaxRenderOrder(int order);
    long GetParentMaxDelay();
    void UpdateInitialDelay();

    bool MergeWithParentNode();

    bool BridgesOnly();

    bool loopFlag;
    int countSteps;

    QList<SolverNode*>listParents;
    QList<SolverNode*>listChilds;

    void ResetLoopFlags();

    friend class PathSolver;
    friend class UpdateDelays;
};

#endif // SOLVERNODE_H
