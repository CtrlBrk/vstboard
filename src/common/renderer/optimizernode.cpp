#include "optimizernode.h"
#include "solvernode.h"

OptimizerNode::OptimizerNode(int id, const SolverNode &c) :
    minRenderOrder(c.minRenderOrder),
    maxRenderOrder(c.maxRenderOrder),
    cpuTime(0),
    id(id)
{
    Init();
}

void OptimizerNode::Init()
{
    for(int i=minRenderOrder; i<=maxRenderOrder; i++) {
        for(int j=maxRenderOrder; j>=i; j--) {
            possiblePositions << NodePos(i,j);
        }
    }
}
