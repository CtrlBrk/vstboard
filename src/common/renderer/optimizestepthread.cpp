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

#include "optimizestepthread.h"
#include "renderernode.h"

OptimizeStepThread::OptimizeStepThread() :
    cpuTime(0),
    spanFor(0)
{
}

OptimizeStepThread::OptimizeStepThread(const OptimizeStepThread &th)
{
    cpuTime = th.cpuTime;
    spanFor = th.spanFor;
    listOfNodes = th.listOfNodes;
}

bool OptimizeStepThread::GetMergedNode(RendererNode **node)
{
    if(listOfNodes.isEmpty()) {
        return false;
    }

    QList<RendererNode*>newLst = listOfNodes;
    RendererNode *n = new RendererNode( *newLst.takeFirst() );
    n->ClearMergedNodes();

    while(!newLst.isEmpty()) {
        RendererNode *merged = new RendererNode( *newLst.takeFirst() );
        merged->ClearMergedNodes();
        n->AddMergedNode( merged );
    }

    *node=n;

    return true;
}

//void OptimizeStepThread::AddToModel(QStandardItemModel *model, int row, int col)
//{
//    if(spanFor) {
//        model->setItem(row,col, new QStandardItem("+") );
//        return;
//    }

//    foreach(RendererNode *node, listOfNodes) {
//        if(node) {
//            QStandardItem *item = new QStandardItem(".");
//            model->setItem(row,col,item);
//            node->modelIndex=item->index();
//        }
//        row++;
//    }
//}

//void OptimizeStepThread::UpdateView( QStandardItemModel *model )
//{
//    foreach(RendererNode *node, listOfNodes) {
//        if(node) {
//            node->UpdateModel( model );
//        }
//    }
//}
