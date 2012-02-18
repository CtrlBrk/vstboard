#ifndef RENDERERTHREAD2_H
#define RENDERERTHREAD2_H

//#include "precomp.h"

class RendererNode2;
typedef QMap<int, QList<RendererNode2*> > ThreadNodes;

class Renderer2;
class RendererThread2 : public QThread
{
    Q_OBJECT

public:
    RendererThread2(Renderer2 *renderer, int id);
    ~RendererThread2();
    void run();
    void SetListOfNodes(const ThreadNodes &n);
    void Suspend(bool s=true);
    bool IsSuspended();
    void LockAllSteps();
    void Stop();

private:
    void Render();
    bool CheckIfStopping();
    bool WaitNextStart();
    void WaitIfSuspended();
    void ApplyNewNodes();

    ThreadNodes currentNodes;

    ThreadNodes newNodes;
    QMutex *mutexNewNodes;

    bool stop;
    QMutex *mutexStop;

    bool suspended;
    bool suspendAsked;
    QMutex *mutexSuspend;
    QWaitCondition *condSuspend;

    Renderer2 *renderer;

    int id;
};

#endif // RENDERERTHREAD2_H
