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

#ifndef OBJECTPARAMETER_H
#define OBJECTPARAMETER_H

#include "../precomp.h"

namespace Connectables {

    class ObjectParameter
    {

    public:
        ObjectParameter() : value(.0f), index(0), visible(false), limitInMin(.0f), limitInMax(1.0f), limitOutMin(.0f), limitOutMax(1.0f) {}
        ObjectParameter(float value, bool visible=false) : value(value), index(0), visible(visible), limitInMin(.0f), limitInMax(1.0f), limitOutMin(.0f), limitOutMax(1.0f) {}
        ObjectParameter(int index, bool visible=false) : value(.0f), index(index), visible(visible), limitInMin(.0f), limitInMax(1.0f), limitOutMin(.0f), limitOutMax(1.0f) {}
        ObjectParameter(bool visible) : value(.0f), index(0), visible(visible), limitInMin(.0f), limitInMax(1.0f), limitOutMin(.0f), limitOutMax(1.0f) {}

        ObjectParameter(const ObjectParameter &c) {
            *this = c;
        }

        float value;
        int index;
        bool visible;

        float limitInMin;
        float limitInMax;
        float limitOutMin;
        float limitOutMax;

        QDataStream & toStream(QDataStream& out) const;
        QDataStream & fromStream(QDataStream& in);
    };
}

QDataStream & operator<< (QDataStream& out, const Connectables::ObjectParameter& param);
QDataStream & operator>> (QDataStream& in, Connectables::ObjectParameter& param);

#endif // OBJECTPARAMETER_H
