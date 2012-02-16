#include "optimizemap.h"

OptimizeMap::OptimizeMap()
{

}

OptMap OptimizeMap::GetBestMap(const QList<OptimizerNode*> &nodes, int th)
{
    nbThreads=th;
    nbIter=0;
    maxNbResult=50;
    bestTime=0;
    QList<OptimizerNode*> testingNodes = nodes;
    qSort(testingNodes.begin(), testingNodes.end(), OptimizerNode::CompareCpuUsage);
    OptMap newMap;
    MapNodes(testingNodes,newMap);
    return _map;
}


bool OptimizeMap::MapNodes(QList<OptimizerNode*> &nodes, OptMap &map, int nbUsedThreads)
{
    bool added=false;

    //all nodes are mapped, get the rendering time
    if(nodes.isEmpty()) {
        nbIter++;
        long t = GetRenderingTime(map);
        if( (t>0 && t<bestTime) || bestTime==0) {
            bestTime=t;
            _map = map;
            LOG("res"<<maxNbResult<<"iter"<<nbIter<<"time:"<<t)
            LOG( OptMap2Txt(_map) )
            maxNbResult--;
            if(maxNbResult==0)
                return true;
        }
        return false;
    }

    //for each nodes
    QList<OptimizerNode*> testingNodes = nodes;
    OptimizerNode *testedNode = testingNodes.takeLast();

    //on each thread
    for(int testedThread=0; testedThread<nbThreads && testedThread<=nbUsedThreads; testedThread++) {
        if(testedThread==nbUsedThreads)
            nbUsedThreads++;

        //for each position
        foreach(NodePos testedPos, testedNode->possiblePositions) {
            testedNode->selectedPos = testedPos;

            //create a version of the map with the node on that thread/position
            if(AddNodeToMap(testedNode, map, testedThread, testedPos)) {

                long interTime = GetRenderingTime(map);
                if(bestTime>0 && interTime>bestTime) {
                    //already too long
                } else {
                    added=true;
                    //add the remaining nodes on the tested map
                    if(MapNodes(testingNodes,map,nbUsedThreads))
                        return true;
                }
            }
            RemoveNodeFromMap(testedNode, map, testedThread, testedPos);
        }
    }

//    if(!added) {
//        return false;
//    }
    return false;
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
                }
            }
            stepThreadLength[step.key()][thread] = stepLength;

            if(stepLength>maxStepLength)
                maxStepLength=stepLength;
        }
        stepGlobalEndTime[step.key()] = maxStepLength + currentStepStartTime;
        globalLength=maxStepLength + currentStepStartTime;

//        //already too long
//        if(bestTime>0 && globalLength>bestTime) {
//            return globalLength;
//        }

        ++step;
    }
//    LOG(globalLength)
//    LOG(OptMap2Txt(map))
    return globalLength;
}

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
                str[step.key()+1][th.key()+1] += QString("%1[%2:%3]%4 ").arg(n.id).arg(n.minRenderOrder).arg(n.maxRenderOrder).arg(n.cpuTime);
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
        QMap<int, QString >::iterator th = step3.value().begin();
        while(th!=step3.value().end()) {
            out += QString(" ").repeated( cols[th.key()] - th.value().length() );
            out += th.value();
            out += " | ";
            ++th;
        }
        out += "\n";
        ++step3;
    }
    return out;
}
