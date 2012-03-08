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


#include "objectparameter.h"

using namespace Connectables;

QDataStream & ObjectParameter::toStream(QDataStream& out) const
{
    out << value;
    out << (qint16)index;
    out << visible;

    out << limitInMin;
    out << limitInMax;
    out << limitOutMin;
    out << limitOutMax;
    return out;
}

QDataStream & ObjectParameter::fromStream(QDataStream& in)
{
    in >> value;
    in >> (qint16&)index;
    in >> visible;

    in >> limitInMin;
    in >> limitInMax;
    in >> limitOutMin;
    in >> limitOutMax;
    return in;
}


QDataStream & operator<< (QDataStream& out, const Connectables::ObjectParameter& param)
{
    return param.toStream(out);
}

QDataStream & operator>> (QDataStream& in, Connectables::ObjectParameter& param)
{
    return param.fromStream(in);
}
