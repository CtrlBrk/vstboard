#include "renderernode2.h"
#include "optimizernode.h"

#ifdef TESTING
RendererNode2::RendererNode2(qint32 id, qint32 minRenderOrder, qint32 maxRenderOrder, qint32 cpuTime) :
    id(id),
    minRenderOrder(minRenderOrder),
    maxRenderOrder(maxRenderOrder),
    minRenderOrderOri(0),
    maxRenderOrderOri(0),
    cpuTime(cpuTime),
    startSemaphore(0),
    nextStepSemaphore(0)
{
}
#endif

RendererNode2::RendererNode2(const OptimizerNode& c) :
    id(c.id),
    minRenderOrder(c.selectedPos.startStep),
    maxRenderOrder(c.selectedPos.endStep),
    minRenderOrderOri(c.minRenderOrder),
    maxRenderOrderOri(c.maxRenderOrder),
    cpuTime(c.cpuTime),
    startSemaphore(0),
    nextStepSemaphore(0)
{
}

RendererNode2::RendererNode2(const RendererNode2& c) :
    id(c.id),
    minRenderOrder(c.minRenderOrder),
    maxRenderOrder(c.maxRenderOrder),
    minRenderOrderOri(c.minRenderOrderOri),
    maxRenderOrderOri(c.maxRenderOrderOri),
    cpuTime(c.cpuTime),
    startSemaphore(0),
    nextStepSemaphore(0),
    listOfObj(c.listOfObj)
{

}

RendererNode2::~RendererNode2()
{
}

void RendererNode2::NewRenderLoop() const
{
    foreach( QWeakPointer<Connectables::Object> ObjPtr, listOfObj) {
        if(ObjPtr) {
            ObjPtr.toStrongRef()->NewRenderLoop();
        }
    }
}

void RendererNode2::Render() const
{
#ifdef _WIN32
    unsigned long timerStart=0;
    FILETIME creationTime, exitTime, kernelTime, userTime;
    if( GetThreadTimes( GetCurrentThread(), &creationTime, &exitTime, &kernelTime, &userTime) !=0 ) {
        timerStart = kernelTime.dwLowDateTime + userTime.dwLowDateTime;
    }
#endif

    foreach( QSharedPointer<Connectables::Object> objPtr, listOfObj) {
        if(objPtr && !objPtr->GetSleep()) {
            objPtr->Render();
        }
    }

#ifdef _WIN32
    if( GetThreadTimes( GetCurrentThread(), &creationTime, &exitTime, &kernelTime, &userTime) !=0 ) {
        cpuTime += ( (kernelTime.dwLowDateTime + userTime.dwLowDateTime) - timerStart )/1000;
    }
#endif
}

void RendererNode2::GetInfo(MsgObject &msg) const
{
    msg.prop[MsgObject::Value] = minRenderOrder;
    msg.prop[MsgObject::Value1] = maxRenderOrder;
    msg.prop[MsgObject::Value2] = minRenderOrderOri;
    msg.prop[MsgObject::Value3] = maxRenderOrderOri;
    msg.prop[MsgObject::Value4] = cpuTime;

    QString str;
    foreach( QSharedPointer<Connectables::Object> objPtr, listOfObj) {
        if(objPtr && !objPtr->GetSleep()) {
#ifndef QT_NO_DEBUG
            str.append(QString("\n%1 (%2)").arg(objPtr->objectName()).arg(objPtr->GetIndex()) );
#else
            str.append( "\n"+objPtr->objectName() );
#endif
        }
    }

    msg.prop[MsgObject::Name]=str;
}
