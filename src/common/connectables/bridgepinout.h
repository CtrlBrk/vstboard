/**************************************************************************
#    Copyright 2010-2012 RaphaÃ«l FranÃ§ois
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

#ifndef BRIDGEPINOUT_H
#define BRIDGEPINOUT_H

#include "pin.h"

namespace Connectables {

    class BridgePinOut : public Pin
    {
    public:
        BridgePinOut(Object *parent, int number, bool bridge=false);
        void SendMsg(const PinMessage::Enum msgType,void *data=0);
        float GetValue();

    protected:
        /// type of the last message transmited
        PinType::Enum valueType;
    };
}
#endif // BRIDGEPINOUT_H
