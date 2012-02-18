#include "renderernode2.h"
#include "optimizernode.h"

RendererNode2::RendererNode2(int id, int minRenderOrder, int maxRenderOrder, long cpuTime) :
    id(id),
    minRenderOrder(minRenderOrder),
    maxRenderOrder(maxRenderOrder),
    cpuTime(0L)
{
}

RendererNode2::RendererNode2(const OptimizerNode& c) :
    id(c.id),
    minRenderOrder(c.selectedPos.startStep),
    maxRenderOrder(c.selectedPos.endStep),
    cpuTime(c.cpuTime)
{
}

void RendererNode2::NewRenderLoop()
{
    foreach( QWeakPointer<Connectables::Object> ObjPtr, listOfObj) {
        if(ObjPtr) {
            ObjPtr.toStrongRef()->NewRenderLoop();
        }
    }
}

void RendererNode2::Render()
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
