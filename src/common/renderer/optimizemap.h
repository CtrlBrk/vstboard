#ifndef OPTIMIZEMAP_H
#define OPTIMIZEMAP_H

#include "precomp.h"
#include "optimizernode.h"

//step, thread, nodes
typedef QMap<int, QMap<int, QList<OptimizerNode> > > OptMap;

class OptimizeMap
{
public:
    OptimizeMap();
    OptMap GetBestMap(const QList<OptimizerNode*> &nodes, int th);

    int nbIter;
    int maxNbResult;
    long bestTime;

private:
    bool MapNodes(QList<OptimizerNode *> &nodes, OptMap &map, int nbUsedThreads=0);
    bool AddNodeToMap(OptimizerNode *node, OptMap &map, int thread, const NodePos &pos);
    void RemoveNodeFromMap(OptimizerNode *node, OptMap &map, int thread, const NodePos &pos);
    long GetRenderingTime(const OptMap &map);
    bool OptimizeMap::CompareNodeSpeed(OptimizerNode *n1, OptimizerNode *n2);
    OptimizerNode *TakeNextNodeForStep(QList<OptimizerNode*> &nodes, int step);

    int nbThreads;
    OptMap _map;
};

QString OptMap2Txt(const OptMap& map);

#endif // OPTIMIZEMAP_H
