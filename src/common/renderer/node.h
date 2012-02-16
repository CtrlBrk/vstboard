#ifndef NODE_H
#define NODE_H

//#include "precomp.h"
#include "connectables/object.h"

class Node
{
public:
    Node();
    Node(const Node &c);

#ifdef TESTING
    Node(int minRenderOrder, int maxRenderOrder) : minRenderOrder(minRenderOrder), maxRenderOrder(maxRenderOrder) {}
#endif

    int minRenderOrder;
    int maxRenderOrder;
    long internalDelay;
    long totalDelayAtOutput;
    QList< QWeakPointer<Connectables::Object> >listOfObj;
};

#endif // NODE_H
