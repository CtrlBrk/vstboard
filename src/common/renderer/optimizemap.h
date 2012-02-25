#ifndef OPTIMIZEMAP_H
#define OPTIMIZEMAP_H

//#include "precomp.h"
#include "optimizernode.h"
#define LOW_CPU_USAGE 100L

//step, thread, nodes
typedef QMap<int, QMap<int, QList<OptimizerNode> > > OptMap;

class OptimizeMap
{
public:
    OptimizeMap(const QList<OptimizerNode*> &nodes, int th);
    ~OptimizeMap();
    void GetBestMap(OptMap &map);

    long bestTime;

#ifdef TESTING
    int nbIter;
    int skipedIter;
#endif

    bool operator ==(const OptimizeMap &c) const;
    bool operator !=(const OptimizeMap &c) const {
        return !(*this==c);
    }

//#ifdef TESTING
    static QString OptMap2Txt(const OptMap& map);
//#endif

private:
    bool MapNodes(QList<OptimizerNode *> &nodes, OptMap &map, int nbUsedThreads=0);
    bool AddNodeToMap(OptimizerNode *node, OptMap &map, int thread, const NodePos &pos);
    void RemoveNodeFromMap(OptimizerNode *node, OptMap &map, int thread, const NodePos &pos);
    long GetRenderingTime(const OptMap &map);

    QList<OptimizerNode*>nodes;
    QMap<int,long>threadTimes;
    int nbThreads;
    OptMap bestMap;
    long currentMapTime;
    bool solvingDone;
};

#endif // OPTIMIZEMAP_H
