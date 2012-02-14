#ifndef OPTIMIZESTEPTHREAD_H
#define OPTIMIZESTEPTHREAD_H

class RendererNode;
class OptimizeStepThread
{
public:
    OptimizeStepThread();
    OptimizeStepThread(const OptimizeStepThread &th);
    bool GetMergedNode(RendererNode **node);
//    void AddToModel(QStandardItemModel *model, int row, int col);
//    void UpdateView( QStandardItemModel *model );

    QList<RendererNode*>listOfNodes;
    long cpuTime;
    RendererNode* spanFor;
};

#endif // OPTIMIZESTEPTHREAD_H
