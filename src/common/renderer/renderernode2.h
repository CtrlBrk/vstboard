#ifndef RENDERERNODE2_H
#define RENDERERNODE2_H

#include "connectables/object.h"

class SemaphoreInverted;
class OptimizerNode;
class RendererNode2
{
public:
#ifdef TESTING
    RendererNode2(int id, int minRenderOrder, int maxRenderOrder, long cpuTime);
#endif
    RendererNode2(const OptimizerNode& c);
    RendererNode2(const RendererNode2& c);
    ~RendererNode2();
    void NewRenderLoop() const;
    void Render() const;
    void GetInfo(MsgObject &msg) const;

    qint32 id;
    qint32 minRenderOrder;
    qint32 maxRenderOrder;
    qint32 minRenderOrderOri;
    qint32 maxRenderOrderOri;
    mutable qint32 cpuTime;

    SemaphoreInverted *startSemaphore;
    SemaphoreInverted *nextStepSemaphore;

    QList< QWeakPointer<Connectables::Object> >listOfObj;
};

#endif // RENDERERNODE2_H
