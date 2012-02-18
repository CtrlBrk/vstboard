#ifndef OPTIMIZERNODE_H
#define OPTIMIZERNODE_H

#include "node.h"

class NodePos
{
public:
    NodePos(int startStep=-1, int endStep=-1) : startStep(startStep), endStep(endStep) {}
    int startStep;
    int endStep;
};

class SolverNode;
class OptimizerNode
{
public:
    OptimizerNode(int id, const SolverNode &c);

//#ifdef TESTING
    OptimizerNode(int id, long cpuTime, int minRenderOrder, int maxRenderOrder) :  id(id), minRenderOrder(minRenderOrder) ,maxRenderOrder(maxRenderOrder), cpuTime(cpuTime) { Init(); }
//#endif

    int id;
    int minRenderOrder;
    int maxRenderOrder;
    QList<NodePos>possiblePositions;
    long cpuTime;
    NodePos selectedPos;

    bool operator ==(const OptimizerNode &c) const {
        return (c.id==id);
    }

    static bool CompareCpuUsage(OptimizerNode *n1, OptimizerNode *n2)
    {
        return n1->cpuTime < n2->cpuTime;
    }

private:
    void Init();
};

#endif // OPTIMIZERNODE_H
