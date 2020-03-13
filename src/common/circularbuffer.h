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

#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H
#include "precomp.h"
class CircularBuffer
{
public:
    CircularBuffer(long size=0);
    ~CircularBuffer();
    void Clear();
    void SetSize(long size);
    bool Put(float *buf, long size);
    long Put(CircularBuffer &buf, long size);
    bool Put(double *buf, long size);
    bool Get(float *buf, long size);
    bool Get(double *buf, long size);
    bool Skip(long size);
    long buffSize;
    long filledSize;


private:
    float *readPos;
    float *writePos;
    float *bufStart;
    float *bufEnd;
    float *buffer;

};

#endif // CIRCULARBUFFER_H
