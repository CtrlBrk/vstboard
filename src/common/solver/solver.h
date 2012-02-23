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
    ~Solver();
    long GetMap(const hashObjects &listObject, const hashCables &lstCables, int nbThreads, RenderMap &rMap);
//    void UpdateMap(const RenderMap &rMap);
    void UpdateCpuTimes(RenderMap &rMap, int nbThreads);
    static QString RMap2Txt(const RenderMap& map);
    void GetInfo(const RenderMap &map, MsgObject &msg) const;

private:
    void OptimzerMap2RenderMap(const OptMap &oMap, RenderMap &rMap);
    QList<SolverNode*>solverNodes;
    hashCables lstCables;
    QList<OptimizerNode*>optimizerNodes;
    QList<RendererNode2*>rendererNodes;
    QList<RendererNode2*>rendererNodesDeleted;
    int nbThreads;

};

#endif // SOLVER_H
