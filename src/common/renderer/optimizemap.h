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

    long bestTime;

#ifdef TESTING
    int nbIter;
    int skipedIter;
#endif

private:
    bool MapNodes(QList<OptimizerNode *> &nodes, OptMap &map, int nbUsedThreads=0);
    bool AddNodeToMap(OptimizerNode *node, OptMap &map, int thread, const NodePos &pos);
    void RemoveNodeFromMap(OptimizerNode *node, OptMap &map, int thread, const NodePos &pos);
    long GetRenderingTime(const OptMap &map);
    int nbThreads;
    QMap<int,long>threadTimes;
    long currentMapTime;
    OptMap _map;
};

#ifdef TESTING
QString OptMap2Txt(const OptMap& map);
#endif

#endif // OPTIMIZEMAP_H
