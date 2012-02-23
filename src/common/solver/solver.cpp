#include "solver.h"
#include "renderer/renderernode2.h"
#include "renderer/optimizemap.h"
#include "renderer/optimizernode.h"

Solver::Solver() :
    nbThreads(0)
{
}

long Solver::GetMap(const hashObjects &listObject, const hashCables &listCables, int nbTh, RenderMap &rMap)
{
    nbThreads=nbTh;

    qDeleteAll(solverNodes);
    solverNodes.clear();
    PathSolver path;
    long globalDelay = path.GetNodes(listObject, listCables, solverNodes);

    qDeleteAll(optimizerNodes);
    optimizerNodes.clear();
    int cpt=0;
    foreach(const SolverNode *n, solverNodes) {
        optimizerNodes << new OptimizerNode(cpt, *n);
        ++cpt;
    }
    OptimizeMap optMap(optimizerNodes,nbThreads);
    OptMap oMap;
    optMap.GetBestMap(oMap);
    LOG( OptimizeMap::OptMap2Txt(oMap) )

    OptimzerMap2RenderMap(oMap,rMap);

    return globalDelay;
}

void Solver::UpdateMap(const QList<RendererNode2*> &rNodes)
{
    int cpt=0;
    foreach(RendererNode2 *rNode, rNodes) {
        if(cpt != rNode->id || cpt != optimizerNodes[cpt]->id ) {
            LOG("problem...")
            return;
        }
        optimizerNodes[cpt]->cpuTime = rNode->cpuTime;
        ++cpt;
    }

    OptimizeMap optMap(optimizerNodes,nbThreads);
    OptMap oMap;
    optMap.GetBestMap(oMap);
    LOG( OptimizeMap::OptMap2Txt(oMap) )
}

void Solver::OptimzerMap2RenderMap(const OptMap &oMap, RenderMap &rMap)
{
    OptMap::iterator step = oMap.begin();
    while(step != oMap.end()) {
        QMap<int, QList<OptimizerNode> >::iterator thread = step.value().begin();
        while(thread!=step.value().end()) {
            foreach(const OptimizerNode &node, thread.value()) {
                if(node.selectedPos.startStep!=step.key())
                    continue;
                RendererNode2 *rNode = new RendererNode2(node);
                rNode->listOfObj = solverNodes[rNode->id]->listOfObj;
                rMap[thread.key()][step.key()] << rNode;
            }

            ++thread;
        }
        ++step;
    }
}

QString Solver::RMap2Txt(const RenderMap& map)
{
    QMap<int, QMap<int,QString> >str;
    str[0][0] = "       ";

    RenderMap::iterator thread = map.begin();
    while(thread!=map.end()) {
        str[0][thread.key()+1]=QString("thread:%1").arg(thread.key());


        QMap<int, QList<RendererNode2*> >::iterator step = thread.value().begin();
        while(step!=thread.value().end()) {
            str[step.key()+1][0]=QString("step:%1").arg(step.key());

            foreach(const RendererNode2 *n, step.value()) {
                str[step.key()+1][thread.key()+1] += QString("%1[%2:%3]%4 ").arg(n->id).arg(n->minRenderOrder).arg(n->maxRenderOrder).arg(n->cpuTime);
            }
            ++step;
        }
        ++thread;
    }

    QMap<int,int>cols;
    QMap<int, QMap<int, QString > >::iterator step2 = str.begin();
    while(step2!=str.end()) {
        QMap<int, QString >::iterator th = step2.value().begin();
        while(th!=step2.value().end()) {
            if(cols[th.key()] < th.value().length())
                cols[th.key()] = th.value().length();
            ++th;
        }
        ++step2;
    }

    QString out("\n");
    QMap<int, QMap<int, QString > >::iterator step3 = str.begin();
    while(step3!=str.end()) {
        int cpt=0;
        QMap<int, QString >::iterator th = step3.value().begin();
        while(th!=step3.value().end()) {
            while(th.key()>cpt) {
                out += QString(" ").repeated( cols[cpt] );
                out += " | ";
                cpt++;
            }
            out += QString(" ").repeated( cols[th.key()] - th.value().length() );
            out += th.value();
            out += " | ";
            ++th;
            cpt++;
        }
        out += "\n";
        ++step3;
    }
    return out;
}
