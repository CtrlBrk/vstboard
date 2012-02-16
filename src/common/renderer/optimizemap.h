#ifndef OPTIMIZEMAP_H
#define OPTIMIZEMAP_H

#include "precomp.h"
#include "optimizernode.h"

#define LOW_CPU_USAGE 100L

//step, thread, nodes
typedef QMap<int, QMap<int, QList<OptimizerNode> > > OptMap;

class OptimizeMap
{
public:
    OptimizeMap(const QList<OptimizerNode*> &nodes, int th);
    OptMap GetBestMap();

    long bestTime;

#ifdef TESTING
    int nbIter;
    int skipedIter;
#endif

    bool operator ==(const OptimizeMap &c) const {
        if(c.nbThreads != nbThreads)
            return false;

        if(c.nodes.count()!=nodes.count())
            return false;

        for(int i=0; i<nodes.count(); i++) {
            if(nodes.at(i)->minRenderOrder != c.nodes.at(i)->minRenderOrder)
                return false;
            if(nodes.at(i)->maxRenderOrder != c.nodes.at(i)->maxRenderOrder)
                return false;

            //compare cpu times : tolerate 5%
            long t1 = nodes.at(i)->cpuTime;
            long t2 = c.nodes.at(i)->cpuTime;
            if(t1 != t2) {
                float dif = 200*abs(t1-t2);
                dif/=(t1+t2);
                if(dif>5)
                    return false;
            }
        }

        return true;
    }

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
