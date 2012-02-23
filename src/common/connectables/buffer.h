/**************************************************************************
#    Copyright 2010-2011 Raphaël François
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

#ifndef BUFFER_H
#define BUFFER_H

#include "object.h"
#include "circularbuffer.h"

namespace Connectables
{

    class Buffer : public Object
    {
        Q_OBJECT
    public:
        Buffer(MainHost *host, int index, const ObjectInfo &info);
        ~Buffer();
        void Render();
        void SetDelay(long d);
        QString GetParameterName(ConnectionInfo /*pinInfo*/) {return QString::number(delaySize);}

    private:
        bool CutBufferAtZeroCrossing(float *buffer, long size);
        CircularBuffer buffer;
        volatile bool delayChanged;
        long delaySize;
        long addedSize;
        long offset;
        float *adjustDelay;
        int countWait;
    public slots:
        void OnParameterChanged(ConnectionInfo pinInfo, float value);
    };

}
#endif // BUFFER_H
