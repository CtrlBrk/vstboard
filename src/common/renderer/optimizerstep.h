#ifndef OPTIMIZERSTEP_H
#define OPTIMIZERSTEP_H

#include "optimizestepthread.h"

class Optimizer;
class OptimizerStep
{
public:
    OptimizerStep(int step, int nbThreads, Optimizer *parent);
    OptimizerStep(const OptimizerStep &s);
    ~OptimizerStep();
    void AddNode(RendererNode *node);
    void MapOnThreadsRandomly();
    void Optimize();
    void OptimizeSpannedNodes();
    void SetNbThreads(int nb);
    bool GetMergedNode(int thread, RendererNode **node);
    QList<RendererNode*> GetListOfNodes();
//    void AddToModel( QStandardItemModel *model );
//    void UpdateView( QStandardItemModel *model );
    void ClearNodes();

    static bool CompareNodeSpeed(RendererNode *n1, RendererNode *n2);
    static bool CompareThreadSpeed(OptimizeStepThread *t1, OptimizeStepThread *t2);


protected:
    void ClearThreads();
    void CreateThreads();
    bool NextStepCanAcceptSpannedNode(RendererNode *node, long cpuGainForPreviousStep);
    bool NextStepCanAcceptPostponedNode(RendererNode *node, long cpuGainForPreviousStep);
    QList<RendererNode*>listOfNodes;
    QList<OptimizeStepThread*>listOfThreads;
    int step;
    int nbThreads;
    Optimizer *parent;
    OptimizerStep *nextStep;
};

#endif // OPTIMIZERSTEP_H
