/**************************************************************************
#    Copyright 2010-2025 Raphaël François
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
#include "pinfactory.h"

using namespace Connectables;

Pin* PinFactory::MakePin(const pinConstructArgs &conf)
{
    switch(conf.type) {
        case PinType::Midi :
            switch(conf.direction) {
            case PinDirection::Input :
                return new MidiPinIn(conf);
            case PinDirection::Output :
                return new MidiPinOut(conf);
            default:
                return 0;
            }
        break;

        case PinType::Audio :
            return new AudioPin(conf);
        break;

        case PinType::Bridge :
            switch(conf.direction) {
            case PinDirection::Input :
                return new BridgePinIn(conf);
            case PinDirection::Output :
                return new BridgePinOut(conf);
            default:
                return 0;
            }
        break;

        case PinType::Parameter :
            switch(conf.direction) {
            case PinDirection::Input :
                return new ParameterPinIn(conf);
            case PinDirection::Output :
                return new ParameterPinOut(conf);
            default:
                return 0;
            }
        break;

        default:
            return 0;
    }

   return 0;
}
