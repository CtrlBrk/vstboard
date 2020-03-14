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

#include "solver.h"
#include "renderer/renderernode2.h"
#include "renderer/optimizemap.h"
#include "renderer/optimizernode.h"

Solver::Solver() :
    nbThreads(0)
{
}

Solver::~Solver()
{
    foreach(RendererNode2 *n, rendererNodesDeleted) {
        delete n;
    }
    foreach(RendererNode2 *n, rendererNodes) {
        delete n;
    }
}

long Solver::GetMap(const hashObjects &listObject, const hashCables &listCables, int nbTh, RenderMap &rMap)
{
    nbThreads=nbTh;

//
//    solverNodes.clear();
    QList<SolverNode*>solverNodes;
    PathSolver path;
    long globalDelay = path.GetNodes(listObject, listCables, solverNodes);

    QList<OptimizerNode*>optimizerNodes;
    int cpt=0;
    foreach(const SolverNode *n, solverNodes) {
        optimizerNodes << new OptimizerNode(cpt++, *n);
    }
    OptimizeMap optMap(optimizerNodes,nbThreads);
    OptMap oMap;
    optMap.GetBestMap(oMap);
    rMap=RenderMap(oMap,solverNodes);

    qDeleteAll(optimizerNodes);
    qDeleteAll(solverNodes);

    LOG( OptimizeMap::OptMap2Txt(oMap) )
//    LOG( RMap2Txt(rMap) )

    return globalDelay;
}

void Solver::UpdateCpuTimes(RenderMap &rMap, int nbThreads)
{
    QList<OptimizerNode*>optimizerNodes;
    QList<SolverNode*>solverNodes;

    int cpt=0;
    RendererMap::iterator thread = rMap.map.begin();
    while(thread != rMap.map.end()) {
       ThreadNodes::iterator step = thread.value().begin();
        while(step!=thread.value().end()) {
            foreach(const QSharedPointer<RendererNode2>node, step.value()) {
                solverNodes << new SolverNode(*node);
                OptimizerNode *oN = new OptimizerNode(*node);
                oN->id = cpt++;
                optimizerNodes << oN;
            }
            ++step;
        }
        ++thread;
    }

    OptimizeMap optMap(optimizerNodes,nbThreads);
    OptMap oMap;
    optMap.GetBestMap(oMap);
    rMap=RenderMap(oMap,solverNodes);

    qDeleteAll(optimizerNodes);
    qDeleteAll(solverNodes);

    LOG( OptimizeMap::OptMap2Txt(oMap) )
//    LOG( RMap2Txt(rMap) )
}

