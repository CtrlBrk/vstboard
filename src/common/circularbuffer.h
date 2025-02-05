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

#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H
//#include "precomp.h"
class CircularBuffer
{
public:
    CircularBuffer(qint32 size=0);
    ~CircularBuffer();
    void Clear();
    void SetSize(qint32 size);
    bool Put(float *buf, qint32 size);
    qint32 Put(CircularBuffer &buf, qint32 size);
    bool Put(double *buf, qint32 size);
    bool Get(float *buf, qint32 size);
    bool Get(double *buf, qint32 size);
    bool Skip(qint32 size);
    qint32 buffSize;
    qint32 filledSize;


private:
    void Fit(qint32 size);
    float *readPos;
    float *writePos;
    float *bufStart;
    float *bufEnd;
    float *buffer;

};

#endif // CIRCULARBUFFER_H
