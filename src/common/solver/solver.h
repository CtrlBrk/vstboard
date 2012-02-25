#ifndef SOLVER_H
#define SOLVER_H

#include "renderer/pathsolver.h"
#include "renderer/rendermap.h"

class Solver
{
public:
    Solver();
    ~Solver();
    long GetMap(const hashObjects &listObject, const hashCables &lstCables, int nbThreads, RenderMap &rMap);
    void UpdateCpuTimes(RenderMap &rMap, int nbThreads);
    int nbThreads;

private:
    QList<SolverNode*>solverNodes;
    QList<RendererNode2*>rendererNodes;
    QList<RendererNode2*>rendererNodesDeleted;

};

#endif // SOLVER_H
