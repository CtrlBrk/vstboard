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


#include "objectprogram.h"
#include "parameterpin.h"

using namespace Connectables;

ObjectProgram::ObjectProgram(PinsList *in, PinsList *out)
{
    ResetDirty();

    ObjectParameter param;
    QMap<quint16,Pin*>::const_iterator i = in->listPins.constBegin();
    while(i!=in->listPins.constEnd()) {
        static_cast<ParameterPin*>(i.value())->GetDefault(param);
        listParametersIn.insert(i.key(), param);
        ++i;
    }

    QMap<quint16,Pin*>::const_iterator j = out->listPins.constBegin();
    while(j!=out->listPins.constEnd()) {
        static_cast<ParameterPin*>(j.value())->GetDefault(param);
        listParametersOut.insert(j.key(), param);
        ++j;
    }
}

void ObjectProgram::Load(PinsList *in, PinsList *out)
{
    QMap<ushort,ObjectParameter>::ConstIterator i=listParametersIn.constBegin();
    while(i!=listParametersIn.constEnd()) {
        Pin *pin=0;

        if(!in->listPins.contains(i.key())) {
            pin = in->AddPin(i.key());
        } else {
            pin =  in->listPins.value(i.key());
        }
        if(!pin) {
            ++i;
            continue;
        }

        //program is set when a connected pin send a signal, don't set it now
        if(i.key()==FixedPinNumber::vstProgNumber) {
            ++i;
            continue;
        }


        static_cast<ParameterPin*>(pin)->Load(i.value());

        ++i;
    }

    QMap<ushort,ObjectParameter>::ConstIterator j=listParametersOut.constBegin();
    while(j!=listParametersOut.constEnd()) {
        Pin *pin=0;
        if(!out->listPins.contains(j.key())) {
            pin = out->AddPin(j.key());
        } else {
            pin = out->listPins.value(j.key());
        }
        if(!pin) {
            ++j;
            continue;
        }
        static_cast<ParameterPin*>(pin)->Load(j.value());
        ++j;
    }

    //no parameters ? hide the pin :
    QMap<quint16,Pin*>::iterator k = in->listPins.begin();
    while(k!=in->listPins.end()) {
        if(!listParametersIn.contains(k.key())) {
            k.value()->SetVisible(false);
        }
        ++k;
    }

    QMap<quint16,Pin*>::iterator l = out->listPins.begin();
    while(l!=out->listPins.end()) {
        if(!listParametersOut.contains(l.key())) {
            l.value()->SetVisible(false);
        }
        ++l;
    }

    ResetDirty();
}

void ObjectProgram::Save(PinsList *in,PinsList *out)
{
    listParametersIn.clear();
    QMap<quint16,Pin*>::ConstIterator i = in->listPins.constBegin();
    while(i!=in->listPins.constEnd()) {
        static_cast<ParameterPin*>(i.value())->GetValues( listParametersIn[i.key()] );
        ++i;
    }

    listParametersOut.clear();
    QMap<quint16,Pin*>::ConstIterator j = out->listPins.constBegin();
    while(j!=out->listPins.constEnd()) {
        static_cast<ParameterPin*>(j.value())->GetValues( listParametersOut[j.key()] );
        ++j;
    }

    ResetDirty();
}


QDataStream & ObjectProgram::toStream(QDataStream& out) const
{
    out << (quint16)listParametersIn.size();
    QMap<ushort,ObjectParameter>::ConstIterator i = listParametersIn.constBegin();
    while(i!=listParametersIn.constEnd()) {
        out << (quint16)i.key();
        out << i.value();
        ++i;
    }

    out << (quint16)listParametersOut.size();
    QMap<ushort,ObjectParameter>::ConstIterator j = listParametersOut.constBegin();
    while(j!=listParametersOut.constEnd()) {
        out << (quint16)j.key();
        out << j.value();
        ++j;
    }

    out << (quint16)listOtherValues.size();
    QMap<int, QVariant>::ConstIterator k = listOtherValues.constBegin();
    while(k!=listOtherValues.constEnd()) {
        out << (quint16)k.key();
        out << k.value();
        ++k;
    }

    return out;
}

QDataStream & ObjectProgram::fromStream(QDataStream& in)
{
    quint16 nbParam;
    in >> nbParam;
    for(int i=0; i<nbParam; i++) {
        ObjectParameter param;
        quint16 id;
        in >> id;
        in >> param;
        listParametersIn.insert(id, param);
    }
    in >> nbParam;
    for(int i=0; i<nbParam; i++) {
        ObjectParameter param;
        quint16 id;
        in >> id;
        in >> param;
        listParametersOut.insert(id, param);
    }

    in >> nbParam;
    for(int i=0; i<nbParam; i++) {
        quint16 id;
        in >> id;
        QVariant value;
        in >> value;

        listOtherValues.insert(id,value);
    }

    ResetDirty();
    return in;
}


QDataStream & operator<< (QDataStream& out, const Connectables::ObjectProgram& prog)
{
    return prog.toStream(out);
}

QDataStream & operator>> (QDataStream& in, Connectables::ObjectProgram& prog)
{
    return prog.fromStream(in);
}
