/**************************************************************************
#    Copyright 2010-2012 Raphaël François
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

#ifndef OPTIMIZEMAP_H
#define OPTIMIZEMAP_H

//#include "precomp.h"
#include "optimizernode.h"
#define LOW_CPU_USAGE 100L

//step, thread, nodes
typedef QMap<int, QMap<int, QList<OptimizerNode> > > OptMap;

class OptimizeMap
{
public:
    OptimizeMap(const QList<OptimizerNode*> &nodes, int th);
    ~OptimizeMap();
    void GetBestMap(OptMap &map);

    long bestTime;

#ifdef TESTING
    int nbIter;
    int skipedIter;
#endif

    bool operator ==(const OptimizeMap &c) const;
    bool operator !=(const OptimizeMap &c) const {
        return !(*this==c);
    }

//#ifdef TESTING
    static QString OptMap2Txt(const OptMap& map);
//#endif

private:
    bool MapNodes(QList<OptimizerNode *> &nodes, OptMap &map, int nbUsedThreads=0);
    bool AddNodeToMap(OptimizerNode *node, OptMap &map, int thread, const NodePos &pos);
    void RemoveNodeFromMap(OptimizerNode *node, OptMap &map, int thread, const NodePos &pos);
    long GetRenderingTime(const OptMap &map);

    QList<OptimizerNode*>nodes;
    QMap<int,long>threadTimes;
    int nbThreads;
    OptMap bestMap;
    long currentMapTime;
    bool solvingDone;
};

#endif // OPTIMIZEMAP_H
