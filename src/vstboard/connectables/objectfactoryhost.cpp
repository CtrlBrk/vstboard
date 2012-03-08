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

#include "objectfactoryhost.h"

#include "connectables/audiodevicein.h"
#include "connectables/audiodeviceout.h"
#include "connectables/mididevice.h"

using namespace Connectables;

ObjectFactoryHost::ObjectFactoryHost(MainHost *myHost) :
    ObjectFactory(myHost)
{
}

Object *ObjectFactoryHost::CreateOtherObjects(const ObjectInfo &info, int objId)
{
    Object *obj=0;

    switch(info.nodeType) {
        case NodeType::object :

            switch(info.objType) {
                case ObjType::AudioInterfaceIn:
                    obj = new AudioDeviceIn(myHost,objId, info);
                    break;

                case ObjType::AudioInterfaceOut:
                    obj = new AudioDeviceOut(myHost,objId, info);
                    break;

                case ObjType::MidiInterface:
                    obj = new MidiDevice(myHost,objId, info);
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
