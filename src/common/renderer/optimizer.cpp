#include "optimizer.h"

Optimizer::Optimizer()
{
}

Optimizer::~Optimizer()
{
    Clear();
}

void Optimizer::SetNbThreads(int nb)
{
    nbThreads=nb;
    foreach(OptimizerStep *st, listOfSteps) {
        st->SetNbThreads(nb);
    }
}

void Optimizer::Clear()
{
    foreach(OptimizerStep *st, listOfSteps) {
        delete st;
    }
    listOfSteps.clear();
}

void Optimizer::NewListOfNodes(QList<SolverNode*> & listNodes)
{
    Clear();

    foreach(SolverNode *node, listNodes) {
        int step = node->minRenderOrder;

        OptimizerStep *oStep = listOfSteps.value(step,0);
        if(!oStep) {
            oStep = new OptimizerStep(step,nbThreads,this);
            listOfSteps.insert(step,oStep);
        }

        oStep->AddNode(node);
    }

    foreach(OptimizerStep *step, listOfSteps) {
        step->MapOnThreadsRandomly();
    }
}

void Optimizer::Optimize()
{
    foreach(OptimizerStep *step, listOfSteps) {
        step->Optimize();
    }
}
