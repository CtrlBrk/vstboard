#include "optimizernode.h"
#include "renderernode.h"

OptimizerNode::OptimizerNode(int id, const RendererNode &c) :
    cpuTime(c.cpuTime),
    id(id)
{
    Init();
}

void OptimizerNode::Init()
{
    for(int i=minRenderOrder; i<=maxRenderOrder; i++) {
        for(int j=i; j<=maxRenderOrder; j++) {
            possiblePositions << NodePos(i,j);
        }
    }
}
