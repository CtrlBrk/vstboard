#ifndef SOLVER_H
#define SOLVER_H

#include "pathsolver.h"
#include "rendermap.h"

class Solver
{
public:
    Solver();
    ~Solver();
    long GetMap(const hashObjects &listObject, const hashCables &lstCables, int nbThreads, RenderMap &rMap);
    void UpdateCpuTimes(RenderMap &rMap, int nbThreads);
    int nbThreads;

private:
    QList<RendererNode2*>rendererNodes;
    QList<RendererNode2*>rendererNodesDeleted;

};

#endif // SOLVER_H
