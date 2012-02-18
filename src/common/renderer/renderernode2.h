#ifndef RENDERERNODE2_H
#define RENDERERNODE2_H

#include "connectables/object.h"

class OptimizerNode;
class RendererNode2
{
public:
    RendererNode2(int id, int minRenderOrder, int maxRenderOrder, long cpuTime);
    RendererNode2(const OptimizerNode& c);
    void NewRenderLoop();
    void Render();

    int id;
    int minRenderOrder;
    int maxRenderOrder;
    long cpuTime;

    QList< QWeakPointer<Connectables::Object> >listOfObj;
};

#endif // RENDERERNODE2_H
