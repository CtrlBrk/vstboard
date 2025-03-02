#include "objectfactoryclap.h"

#include "connectables/clapaudiodevicein.h"
#include "connectables/clapaudiodeviceout.h"
// #include "connectables/clapautomation.h"
// #include "connectables/clapmididevice.h"

using namespace Connectables;

ObjectFactoryClap::ObjectFactoryClap(MainHost *myHost) :
    ObjectFactory(myHost)
{

}

Object *ObjectFactoryClap::CreateOtherObjects(const ObjectInfo &info, int objId)
{
    Object *obj=0;

    switch(info.nodeType) {
    case NodeType::object :

        switch(info.objType) {
        case ObjType::AudioInterfaceIn:
             obj = new ClapAudioDeviceIn(myHost,objId, info);
            break;

        case ObjType::AudioInterfaceOut:
            obj = new ClapAudioDeviceOut(myHost,objId, info);
            break;

        case ObjType::VstAutomation:
            // obj = new VstAutomation(myHost,objId);
            break;

        case ObjType::MidiInterface:
            // obj = new VstMidiDevice(myHost,objId, info);
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }

    return obj;
}
