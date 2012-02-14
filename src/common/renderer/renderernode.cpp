/**************************************************************************
#    Copyright 2010-2011 Raphaël François
#    Contact : ctrlbrk76@gmail.com
#
#    This file is part of VstBoard.
#
#    VstBoard is free software: you can redistribute it and/or modify
#    it under the terms of the under the terms of the GNU Lesser General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    VstBoard is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    under the terms of the GNU Lesser General Public License for more details.
#
#    You should have received a copy of the under the terms of the GNU Lesser General Public License
#    along with VstBoard.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#include "renderernode.h"
#include "solvernode.h"

Q_DECLARE_METATYPE( QList<int> )

RendererNode::RendererNode() :
    Node(),
    minRenderOrderOri(-1),
    maxRenderOrderOri(-1),
    modelNeedUpdate(true),
    cpuTime(0),
    benchCount(0)
{
}

RendererNode::RendererNode(const SolverNode &c) :
    Node(c),
    minRenderOrderOri(c.minRenderOrder),
    maxRenderOrderOri(c.maxRenderOrder),
    modelNeedUpdate(true),
    cpuTime(0),
    benchCount(0)
{
}

RendererNode::RendererNode(const RendererNode &c) :
    Node(c),
    minRenderOrderOri(c.minRenderOrderOri),
    maxRenderOrderOri(c.maxRenderOrderOri),
    modelNeedUpdate(true),
    cpuTime(c.cpuTime),
    benchCount(c.benchCount)
{
    foreach( RendererNode *merged, c.listOfMergedNodes) {
        listOfMergedNodes << new RendererNode(*merged);
    }
}

RendererNode::~RendererNode()
{
    foreach(RendererNode *node, listOfMergedNodes) {
        delete node;
    }
    listOfMergedNodes.clear();
}

void RendererNode::NewRenderLoop()
{
    foreach( QWeakPointer<Connectables::Object> ObjPtr, listOfObj) {
        if(ObjPtr) {
            ObjPtr.toStrongRef()->NewRenderLoop();
        }
    }
    foreach(RendererNode *mergedNode, listOfMergedNodes) {
        mergedNode->NewRenderLoop();
    }
}

void RendererNode::Render()
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
        benchCount++;
        modelNeedUpdate=true;
    }
#endif

    foreach(RendererNode *mergedNode, listOfMergedNodes) {
        mergedNode->Render();
    }
}

long RendererNode::GetTotalCpuUsage()
{
    long cpu = cpuTime;

    foreach(RendererNode *merged, listOfMergedNodes) {
        cpu+=merged->cpuTime;
    }

    return cpu;
}

void RendererNode::AddMergedNode(RendererNode *merged)
{
    listOfMergedNodes << merged;
    minRenderOrder = merged->minRenderOrder = std::max(minRenderOrder, merged->minRenderOrder);
    maxRenderOrder = merged->maxRenderOrder = std::min(maxRenderOrder, merged->maxRenderOrder);
}

void RendererNode::ClearMergedNodes()
{
    listOfMergedNodes.clear();
}

void RendererNode::UpdateModel(QStandardItemModel *model)
{
    if(!modelNeedUpdate || !modelIndex.isValid())
        return;

    modelNeedUpdate=false;

    QString str = QString("[%1:%2][%3:%4](%5)\ndelay(%6:%7)")
            .arg(minRenderOrder)
            .arg(maxRenderOrder)
            .arg(minRenderOrderOri)
            .arg(maxRenderOrderOri)
            .arg(cpuTime)
            .arg(internalDelay)
            .arg(totalDelayAtOutput);

    foreach( QSharedPointer<Connectables::Object> objPtr, listOfObj) {
        if(objPtr && !objPtr->GetSleep()) {
            str.append("\n" + objPtr->objectName());
        }
    }

    model->setData(modelIndex, str);

    foreach(RendererNode *mergedNode, listOfMergedNodes) {
        mergedNode->UpdateModel(model);
    }
}

void RendererNode::GetInfo(MsgObject &msg) const
{

    QList<int>lstVals;
    lstVals << minRenderOrder;
    lstVals << maxRenderOrder;
    lstVals << minRenderOrderOri;
    lstVals << maxRenderOrderOri;
    lstVals << cpuTime;
    lstVals << internalDelay;
    lstVals << totalDelayAtOutput;

    msg.prop[MsgObject::Value] = QVariant::fromValue(lstVals);

    QString str;
    foreach( QSharedPointer<Connectables::Object> objPtr, listOfObj) {
        if(objPtr && !objPtr->GetSleep()) {
            str.append("\n" + objPtr->objectName());
        }
    }

    foreach(RendererNode *mergedNode, listOfMergedNodes) {
        MsgObject c;
        mergedNode->GetInfo(c);
        str+=c.prop[MsgObject::Name].toString();
//        msg.children << c;
    }


    msg.prop[MsgObject::Name]=str;
}
