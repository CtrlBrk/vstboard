#ifndef OBJECTFACTORYCLAP_H
#define OBJECTFACTORYCLAP_H

#include "connectables/objectfactory.h"

namespace Connectables {

    class ObjectFactoryClap : public ObjectFactory
    {
    public:
        ObjectFactoryClap(MainHost *myHost);
    protected:
        Object *CreateOtherObjects(const ObjectInfo &info, int objId);
    };
}

#endif // OBJECTFACTORYCLAP_H
