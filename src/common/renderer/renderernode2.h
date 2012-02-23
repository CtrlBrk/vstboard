#ifndef RENDERERNODE2_H
#define RENDERERNODE2_H

#include "connectables/object.h"

class SemaphoreInverted;
class OptimizerNode;
class RendererNode2
{
public:
    RendererNode2(int id, int minRenderOrder, int maxRenderOrder, long cpuTime);
    RendererNode2(const OptimizerNode& c);
    void NewRenderLoop() const;
    void Render() const;

    int id;
    int minRenderOrder;
    int maxRenderOrder;
    mutable long cpuTime;

    SemaphoreInverted *startSemaphore;
    SemaphoreInverted *nextStepSemaphore;

    QList< QWeakPointer<Connectables::Object> >listOfObj;
};

#endif // RENDERERNODE2_H
