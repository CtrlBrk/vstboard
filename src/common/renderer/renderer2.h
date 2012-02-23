#ifndef RENDERER2_H
#define RENDERER2_H

#include "semaphoreinverted.h"
#include "waitall.h"

class RendererThread2;
class RendererNode2;
typedef QMap<int, QList<RendererNode2*> > ThreadNodes;
typedef QMap<int, ThreadNodes > RenderMap;

class Renderer2 : public QObject
{
    Q_OBJECT
public:
    Renderer2(QObject *parent=0);
    ~Renderer2();
    void SetMap(const RenderMap &map, int nbThreads);
    void StartRender();
    void SetEnabled(bool enab) {LOG("enable here"<<enab)}
RenderMap currentMap;
private:
    void ThreadCleanup();
    void ChangeNbOfThreads(int newNbThreads);
    QList<RendererThread2*>threads;
    QList<RendererThread2*>threadsToDelete;
    QMutex mutexThreadList;
    int nbThreads;
    int nbSteps;

    WaitAll waitThreadReady;
    WaitAll waitThreadEnd;
    QList<SemaphoreInverted*>stepCanStart;
friend class RendererThread2;
};

#endif // RENDERER2_H
