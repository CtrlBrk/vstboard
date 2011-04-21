/**************************************************************************
#    Copyright 2010-2011 Rapha�l Fran�ois
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

#include "objectfactoryvst.h"

#include "connectables/vstaudiodevicein.h"
#include "connectables/vstaudiodeviceout.h"
#include "connectables/vstautomation.h"
#include "connectables/vstmididevice.h"

using namespace Connectables;

ObjectFactoryVst::ObjectFactoryVst(MainHost *myHost) :
    ObjectFactory(myHost)
{
}

Object *ObjectFactoryVst::CreateOtherObjects(const ObjectInfo &info)
{
    int objId = cptListObjects;
    if(info.forcedObjId) {
        objId = info.forcedObjId;
    }

    Object *obj=0;

    switch(info.nodeType) {
        case NodeType::object :

            switch(info.objType) {
                case ObjType::AudioInterfaceIn:
                    obj = new VstAudioDeviceIn(myHost,objId, info);
                    break;

                case ObjType::AudioInterfaceOut:
                    obj = new VstAudioDeviceOut(myHost,objId, info);
                    break;

                case ObjType::VstAutomation:
                    obj = new VstAutomation(myHost,objId);
                    break;

                case ObjType::MidiInterface:
                    obj = new VstMidiDevice(myHost,objId, info);
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

