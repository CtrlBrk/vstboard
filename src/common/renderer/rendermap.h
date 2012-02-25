#ifndef RENDERMAP_H
#define RENDERMAP_H

//class RendererNode2;
#include "renderernode2.h"
typedef QList< QSharedPointer<RendererNode2> > RenderStep;
typedef QMap<int, RenderStep > ThreadNodes;
typedef QMap<int, ThreadNodes > RendererMap;

class OptimizerNode;
typedef QMap<int, QMap<int, QList<OptimizerNode > > > OptMap;

class SolverNode;
#include "msgobject.h"

class RenderMap
{
public:
    RenderMap();
//    RenderMap(const RenderMap &c);
    RenderMap(const OptMap &oMap, const QList<SolverNode*> &solverNodes);
//    ~RenderMap();
//    void Clear();
    QString ToTxt();
    void GetInfo(MsgObject &msg) const;

//    RenderMap & operator= (const RenderMap & c);

    RendererMap map;
};

#endif // RENDERMAP_H
