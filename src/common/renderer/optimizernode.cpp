#include "optimizernode.h"
#include "renderernode.h"

OptimizerNode::OptimizerNode(int id, const RendererNode &c) :
    minRenderOrder(c.minRenderOrderOri),
    maxRenderOrder(c.maxRenderOrderOri),
    cpuTime(c.cpuTime),
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
