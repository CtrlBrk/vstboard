#include "solver.h"
#include "renderer/renderernode2.h"
#include "renderer/optimizemap.h"
#include "renderer/optimizernode.h"

Solver::Solver() :
    nbThreads(0)
{
}

void Solver::GetMap(const hashObjects &listObject, const hashCables &listCables, int nbTh, RenderMap &rMap)
{
    nbThreads=nbTh;

    qDeleteAll(solverNodes);
    solverNodes.clear();
    PathSolver path;
    path.GetNodes(listObject, listCables, solverNodes);

    qDeleteAll(optimizerNodes);
    optimizerNodes.clear();
    int cpt=0;
    foreach(const SolverNode *n, solverNodes) {
        optimizerNodes << new OptimizerNode(cpt, *n);
        ++cpt;
    }
    OptimizeMap optMap(optimizerNodes,nbThreads);
    OptMap oMap;
    optMap.GetBestMap(oMap);
    LOG( OptimizeMap::OptMap2Txt(oMap) )

    OptimzerMap2RenderMap(oMap,rMap);
}

void Solver::UpdateMap(const QList<RendererNode2*> &rNodes)
{
    int cpt=0;
    foreach(RendererNode2 *rNode, rNodes) {
        if(cpt != rNode->id || cpt != optimizerNodes[cpt]->id ) {
            LOG("problem...")
            return;
        }
        optimizerNodes[cpt]->cpuTime = rNode->cpuTime;
        ++cpt;
    }

    OptimizeMap optMap(optimizerNodes,nbThreads);
    OptMap oMap;
    optMap.GetBestMap(oMap);
    LOG( OptimizeMap::OptMap2Txt(oMap) )
}

void Solver::OptimzerMap2RenderMap(const OptMap &oMap, RenderMap &rMap)
{
    OptMap::iterator step = oMap.begin();
    while(step != oMap.end()) {
        QMap<int, QList<OptimizerNode> >::iterator thread = step.value().begin();
        while(thread!=step.value().end()) {
            foreach(const OptimizerNode &node, thread.value()) {
                if(node.selectedPos.startStep!=step.key())
                    continue;
                RendererNode2 *rNode = new RendererNode2(node);
                rNode->listOfObj = solverNodes[rNode->id]->listOfObj;
                rMap[thread.key()][step.key()] << rNode;
            }

            ++thread;
        }
        ++step;
    }
}
