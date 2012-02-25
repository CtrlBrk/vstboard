#include "optimizernode.h"
#include "solvernode.h"
#include "renderernode2.h"

OptimizerNode::OptimizerNode(int id, const SolverNode &c) :
    id(id),
    minRenderOrder(c.minRenderOrder),
    maxRenderOrder(c.maxRenderOrder),
    cpuTime(0)
{
    Init();
}

OptimizerNode::OptimizerNode(const OptimizerNode &c) :
    id(c.id),
    minRenderOrder(c.minRenderOrder),
    maxRenderOrder(c.maxRenderOrder),
    possiblePositions(c.possiblePositions),
    cpuTime(c.cpuTime),
    selectedPos(c.selectedPos)
{

}

OptimizerNode::OptimizerNode(const RendererNode2 &c) :
    id(c.id),
    minRenderOrder(c.minRenderOrderOri),
    maxRenderOrder(c.maxRenderOrderOri),
    cpuTime(0)
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
