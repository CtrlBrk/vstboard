#ifndef SOLVER_H
#define SOLVER_H

#include "renderer/pathsolver.h"

typedef QMap<int, QMap<int, QList<OptimizerNode> > > OptMap;
class RendererNode2;
typedef QMap<int, QMap<int, QList<RendererNode2*> > > RenderMap;

class Solver
{
public:
    Solver();
    long GetMap(const hashObjects &listObject, const hashCables &lstCables, int nbThreads, RenderMap &rMap);
    void UpdateMap(const QList<RendererNode2*> &rNodes);
    static QString RMap2Txt(const RenderMap& map);
private:
    void OptimzerMap2RenderMap(const OptMap &oMap, RenderMap &rMap);
    QList<SolverNode*>solverNodes;
    hashCables lstCables;
    QList<OptimizerNode*>optimizerNodes;
    int nbThreads;

};

#endif // SOLVER_H
