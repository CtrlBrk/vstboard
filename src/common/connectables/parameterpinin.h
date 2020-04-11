/**************************************************************************
#    Copyright 2010-2020 Raphaël François
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

#ifndef PARAMETERPININ_H
#define PARAMETERPININ_H

//#include "../precomp.h"
#include "parameterpin.h"

namespace Connectables {

    class ParameterPinIn : public ParameterPin
    {
    public:
        ParameterPinIn(Object *parent, int number, float defaultValue, QString name="", bool nameCanChange=false, bool isRemoveable=false);
        ParameterPinIn(Object *parent, int number, const QVariant &defaultVariantValue, QList<QVariant> *listValues, QString name="", bool nameCanChange=false, bool isRemoveable=false);

        virtual void ReceivePinMsg(const PinMessage::Enum msgType,void *data=0);
    };
}

#endif // PARAMETERPININ_H
