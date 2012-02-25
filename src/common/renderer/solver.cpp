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
        optimizerNodes << new OptimizerNode(cpt, *n);
        ++cpt;
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

    RendererMap::iterator thread = rMap.map.begin();
    while(thread != rMap.map.end()) {
       ThreadNodes::iterator step = thread.value().begin();
        while(step!=thread.value().end()) {
            foreach(const QSharedPointer<RendererNode2>node, step.value()) {
                solverNodes << new SolverNode(*node);
                optimizerNodes << new OptimizerNode(*node);
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

