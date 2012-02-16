#include "optimizemap.h"

OptimizeMap::OptimizeMap()
{

}

OptMap OptimizeMap::GetBestMap(const QList<OptimizerNode*> &nodes, int th)
{
    threadTimes.clear();
    bestTime=0;
    currentMapTime=0;
    nbThreads=th;

#ifdef TESTING
    nbIter=0;
    skipedIter=0;
#endif


    QList<OptimizerNode*> testingNodes = nodes;
    qSort(testingNodes.begin(), testingNodes.end(), OptimizerNode::CompareCpuUsage);
    OptMap newMap;
    MapNodes(testingNodes,newMap);
    return _map;
}


bool OptimizeMap::MapNodes(QList<OptimizerNode*> &nodes, OptMap &map, int nbUsedThreads)
{
    //all nodes are mapped, get the rendering time
    if(nodes.isEmpty()) {
#ifdef TESTING
        nbIter++;
#endif
        if( (currentMapTime>0 && currentMapTime<bestTime) || bestTime==0) {
            bestTime=currentMapTime;
            _map = map;
        }
        return true;
    }

    //for each nodes, starting with the biggest
    QList<OptimizerNode*> testingNodes = nodes;
    OptimizerNode *testedNode = testingNodes.takeLast();


    //create a list of thread to test
    //include already used thread + one empty thread if one is available
    //order by thread length
    QList<int>lstThread;
    if(threadTimes.isEmpty()) {
        lstThread << 0;
    } else {
        int maxTh = std::min(nbUsedThreads+1,nbThreads);
        QList<long>times = threadTimes.values();
        qSort(times);
        int cpt=0;
        while(!times.empty() && cpt<maxTh) {
            cpt++;
            long t = times.takeLast();

            //fast length test, without steps sync
            long interTime = testedNode->cpuTime + t;
            if(bestTime>0 && interTime>bestTime) {
                //too long, skip this thread
    #ifdef TESTING
                skipedIter++;
    #endif
                continue;
            }

            lstThread << threadTimes.key(t);
        }
    }

    //on each thread, starting with the shortest
    while(!lstThread.isEmpty()) {
        int testedThread = lstThread.takeLast();

        //for each possible position
        foreach(NodePos testedPos, testedNode->possiblePositions) {
            testedNode->selectedPos = testedPos;
            bool addedThread=false;

            //add the node on that thread/position
            if(AddNodeToMap(testedNode, map, testedThread, testedPos)) {

                //placed on an empty thread, increment the number of used threads
                if(testedThread==nbUsedThreads) {
                    addedThread=true;
                    nbUsedThreads++;
                }

                //real length test with steps sync
                currentMapTime = GetRenderingTime(map);
                if(bestTime>0 && currentMapTime>bestTime) {
                    //already too long, skip the remaining nodes
#ifdef TESTING
                    skipedIter++;
#endif
                } else {

                    //add the remaining nodes on the tested map
                    if(!MapNodes(testingNodes,map,nbUsedThreads)) {
                        //solver was canceled
                        return false;
                    }
                }
            }

            //remove the node, reset the nb of used threads, we'll test other positions
            RemoveNodeFromMap(testedNode, map, testedThread, testedPos);
            if(addedThread)
                nbUsedThreads--;
        }
    }

    return true;
}

bool OptimizeMap::AddNodeToMap(OptimizerNode *node, OptMap &map, int thread, const NodePos &pos)
{
    for(int step=pos.startStep; step<=pos.endStep; step++) {
        bool push=false;
        foreach(const OptimizerNode &existingNode, map[step][thread]) {
            //both node are spanned : impossible
            if(pos.endStep > step && existingNode.selectedPos.endStep > step) {
                return false;
            }
            //a node is spanned over this step
            if(existingNode.selectedPos.startStep < step && existingNode.selectedPos.endStep > step) {
                return false;
            }
            //a node is here but i need the node before and after : impossible
            if(pos.startStep < step && pos.endStep > step) {
                return false;
            }
            //must be insterted before a spanning node
            if(existingNode.selectedPos.endStep > step) {
                push=true;
            }
            //spanning ends before existing node
            if(pos.startStep < step) {
                push=true;
            }
        }

        if(push) {
            map[step][thread].prepend(*node);
        } else {
            map[step][thread] << *node;
        }
    }

    return true;
}

void OptimizeMap::RemoveNodeFromMap(OptimizerNode *node, OptMap &map, int thread, const NodePos &pos)
{
    for(int step=pos.startStep; step<=pos.endStep; step++) {
        map[step][thread].removeAll(*node);
    }
}

long OptimizeMap::GetRenderingTime(const OptMap &map)
{
    long globalLength=0;
    threadTimes.clear();

    //step, time
    QMap<int, long>stepGlobalEndTime;

    //step, thread, time
    QMap<int, QMap<int, long> >stepThreadLength;

    OptMap::iterator step = map.begin();
    while(step != map.end()) {

        long maxStepLength=0;
        long currentStepStartTime = stepGlobalEndTime.value(step.key()-1,0);

        for(int thread=0; thread<nbThreads; thread++) {
            long stepLength=0;
            foreach(const OptimizerNode &node, step.value()[thread]) {

                //count only the nodes ending on this step
                if(node.selectedPos.endStep == step.key()) {

                    if(node.selectedPos.startStep < step.key()) {
                        //if the node was started before
                        long startingTime = stepGlobalEndTime.value(node.selectedPos.startStep-1,0) + stepThreadLength[node.selectedPos.startStep][thread];
                        long endingTime = node.cpuTime + startingTime;
                        long remainingTimeInThisStep = endingTime-currentStepStartTime;
                        if(remainingTimeInThisStep>0)
                            stepLength+=remainingTimeInThisStep;
                    } else {
                        stepLength+=node.cpuTime;
                    }

                    if(bestTime>0 && currentStepStartTime+stepLength > bestTime) {
                        //already too long
#ifdef TESTING
                        skipedIter++;
#endif
                        return currentStepStartTime+stepLength;
                    }
                }
            }
            stepThreadLength[step.key()][thread] = stepLength;
            threadTimes[thread]+=stepLength;

            if(stepLength>maxStepLength)
                maxStepLength=stepLength;
        }
        stepGlobalEndTime[step.key()] = maxStepLength + currentStepStartTime;
        globalLength=maxStepLength + currentStepStartTime;

        ++step;
    }
    return globalLength;
}

#ifdef TESTING
QString OptMap2Txt(const OptMap& map)
{
    QMap<int, QMap<int,QString> >str;
    str[0][0] = "       ";

    QMap<int, QMap<int, QList<OptimizerNode> > >::iterator step = map.begin();
    while(step!=map.end()) {
        str[step.key()+1][0]=QString("step:%1").arg(step.key());


        QMap<int, QList<OptimizerNode> >::iterator th = step.value().begin();
        while(th!=step.value().end()) {
            str[0][th.key()+1]=QString("thread:%1").arg(th.key());

            foreach(const OptimizerNode &n, th.value()) {
                str[step.key()+1][th.key()+1] += QString("%1[%2:%3]%4 ").arg(n.id).arg(n.selectedPos.startStep).arg(n.selectedPos.endStep).arg(n.cpuTime);
            }
            ++th;
        }
        ++step;
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
#endif
