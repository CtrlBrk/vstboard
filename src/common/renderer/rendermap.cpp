/**************************************************************************
#    Copyright 2010-2020 Raphaël François
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

#include "rendermap.h"
#include "solvernode.h"
//#include "renderernode2.h"
#include "optimizernode.h"

RenderMap::RenderMap()
{
}

//RenderMap::RenderMap(const RenderMap &c)
//{
//    *this=c;
//}

//RenderMap & RenderMap::operator= (const RenderMap & c)
//{
//    RendererMap newMap;
//    RendererMap::const_iterator thread = c.map.constBegin();
//    while(thread != c.map.constEnd()) {
//        ThreadNodes::const_iterator step = thread.value().constBegin();
//        while(step != thread.value().constEnd()) {
//            foreach(const RendererNode2 *n, step.value()) {
//                newMap[thread.key()][step.key()] << new RendererNode2(*n);
//            }
//            ++step;
//        }
//        ++thread;
//    }
//    Clear();
//    map=newMap;
//    return *this;
//}

//void RenderMap::Clear()
//{
//    RendererMap::const_iterator thread = map.constBegin();
//    while(thread != map.constEnd()) {
//        ThreadNodes::const_iterator step = thread.value().constBegin();
//        while(step != thread.value().constEnd()) {
//            foreach(const RendererNode2 *n, step.value()) {
//                delete n;
//            }
//            ++step;
//        }
//        ++thread;
//    }
//    map.clear();
//}

RenderMap::RenderMap(const OptMap &oMap, const QList<SolverNode*> &solverNodes)
{
    OptMap::const_iterator step = oMap.constBegin();
    while(step != oMap.constEnd()) {
        QMap<int, QList<OptimizerNode> >::const_iterator thread = step.value().constBegin();
        while(thread != step.value().constEnd()) {
            foreach(const OptimizerNode &node, thread.value()) {
                if(node.selectedPos.startStep!=step.key())
                    continue;
                QSharedPointer<RendererNode2>rNode(new RendererNode2(node));
                rNode->listOfObj = solverNodes[rNode->id]->listOfObj;
                map[thread.key()][step.key()] << rNode;
            }

            ++thread;
        }
        ++step;
    }
}

//RenderMap::~RenderMap()
//{
//    Clear();
//}

QString RenderMap::ToTxt()
{
    QMap<int, QMap<int,QString> >str;
    str[0][0] = "       ";

    RendererMap::const_iterator thread = map.constBegin();
    while(thread!=map.constEnd()) {
        str[0][thread.key()+1]=QString("thread:%1").arg(thread.key());


        ThreadNodes::const_iterator step = thread.value().constBegin();
        while(step!=thread.value().constEnd()) {
            str[step.key()+1][0]=QString("step:%1").arg(step.key());

            foreach(const QSharedPointer<RendererNode2> &n, step.value()) {
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

void RenderMap::GetInfo(MsgObject &msg) const
{
    int nbSteps = 0;
    int nbThreads = 0;

//    MsgObject msg(FixedObjId::mainWindow);
    msg.prop[MsgObject::SolverMap]=1;

    RendererMap::const_iterator thread = map.constBegin();
    while(thread!=map.constEnd()) {
        MsgObject msgThread(thread.key());
        if(thread.key()>nbThreads)
            nbThreads=thread.key();


        ThreadNodes::const_iterator step = thread.value().constBegin();
        while(step!=thread.value().constEnd()) {
            MsgObject msgStep(step.key());
            if(step.key()>nbSteps)
                nbSteps=step.key();

            foreach(const QSharedPointer<RendererNode2> &node, step.value()) {
                MsgObject msgNode;
                node->GetInfo(msgNode);
                msgStep.children << msgNode;
            }
            msgThread.children << msgStep;
            ++step;
        }
        msg.children << msgThread;
        ++thread;
    }
    msg.prop[MsgObject::Row]=nbSteps+1;
    msg.prop[MsgObject::Col]=nbThreads+1;
//    myHost->SendMsg(msg);
}

