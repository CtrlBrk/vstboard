/******************************************************************************
#    Copyright 2010 Rapha�l Fran�ois
#    Contact : ctrlbrk76@gmail.com
#
#    This file is part of VstBoard.
#
#    VstBoard is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    VstBoard is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with VstBoard.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#ifndef OBJECTPROGRAM_H
#define OBJECTPROGRAM_H

#include "../precomp.h"
#include "objectparameter.h"

namespace Connectables {

    class ParameterPin;

    typedef QHash<ushort,ParameterPin*> hashListParamPin;

    class ObjectProgram
    {

    public:
        ObjectProgram(int progId) : progId(progId) {};
        ObjectProgram(int progId,const hashListParamPin& listIn, const hashListParamPin& listOut);

        ObjectProgram(const ObjectProgram &c) {
            *this = c;
        }

        void Load(hashListParamPin& listIn, hashListParamPin& listOut);
        void Save(const hashListParamPin& listIn, const hashListParamPin& listOut);

        int progId;

        QList<ObjectParameter> listParametersIn;
        QList<ObjectParameter> listParametersOut;

        QDataStream & toStream(QDataStream& stream) const;
        QDataStream & fromStream(QDataStream& stream);
    };

}

QDataStream & operator<< (QDataStream& stream, const Connectables::ObjectProgram& prog);
QDataStream & operator>> (QDataStream& stream, Connectables::ObjectProgram& prog);

#endif // OBJECTPROGRAM_H
