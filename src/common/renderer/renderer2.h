#ifndef RENDERER2_H
#define RENDERER2_H

#include "semaphoreinverted.h"

class RendererThread2;
class RendererNode2;
typedef QMap<int, QMap<int, QList<RendererNode2*> > > RenderMap;

class Renderer2 : public QObject
{
    Q_OBJECT
public:
    Renderer2(QObject *parent=0);
    ~Renderer2();
    void SetMap(const RenderMap &map, int nbThreads);
    void StartRender();
    void ThreadCleanup();

private:
    void ChangeNbOfThreads(int newNbThreads);
    RenderMap currentMap;
    RenderMap newMap;
    SemaphoreInverted renderInProgress;
    QWaitCondition condStartRender;
    QMutex mutexStartRender;
    QList<SemaphoreInverted*>stepCanStart;
    QList<RendererThread2*>threads;
    QList<RendererThread2*>threadsToDelete;
    QMutex mutexThreadList;
friend class RendererThread2;
};

#endif // RENDERER2_H
