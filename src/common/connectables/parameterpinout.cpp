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


#include "parameterpinout.h"
#include "object.h"
#include "../globals.h"

using namespace Connectables;

//parameter is a float
ParameterPinOut::ParameterPinOut(Object *parent, int number, float defaultValue, const QString &name, bool nameCanChange, bool isRemoveable) :
        ParameterPin(parent, PinDirection::Output, number, defaultValue, name, nameCanChange, isRemoveable)
{
}

//parameter is a int, index in a list of values
ParameterPinOut::ParameterPinOut(Object *parent, int number, const QVariant &defaultVariantValue, QList<QVariant> *listValues, const QString &name, bool nameCanChange, bool isRemoveable) :
        ParameterPin(parent, PinDirection::Output, number, defaultVariantValue, listValues, name, nameCanChange, isRemoveable)
{
}

//value changed, send msg to connected pins
void ParameterPinOut::OnValueChanged(float val)
{
    ParameterPin::OnValueChanged(val);
    SendMsg(PinMessage::ParameterValue,(void*)&outValue);
}
