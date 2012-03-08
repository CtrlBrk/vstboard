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
#include "circularbuffer.h"
//#include "precomp.h"

CircularBuffer::CircularBuffer(long size) :
    buffSize(0),
    buffer(0),
    filledSize(0),
    bufStart(0),
    bufEnd(0),
    readPos(0),
    writePos(0)
{
    SetSize(size);
}

CircularBuffer::~CircularBuffer()
{
    if(buffer)
        delete[] buffer;
}

void CircularBuffer::SetSize(long size)
{
    if(size==buffSize)
        return;

    if(size!=0) {
        float *newBuf = new float[size];

        //skip the oldest data if needed
        if(filledSize>size)
            Skip(filledSize-size);

        //copy the data in the new buffer
        if(filledSize>0) {
            long filled = filledSize;
            Get(newBuf,filledSize);
            filledSize = filled;
        }

        if(buffer)
            delete[] buffer;

        buffer = newBuf;
        buffSize = size;
        bufStart = &buffer[0];
        bufEnd = &buffer[buffSize-1];
        readPos = bufStart;
        writePos = bufStart+filledSize;
        while(writePos>bufEnd)
            writePos-=buffSize;
    } else {
        if(buffer)
            delete[] buffer;

        filledSize = 0;
        buffer = 0;
        buffSize = 0;
        bufStart = 0;
        bufEnd = 0;
        readPos = 0;
        writePos = 0;
    }
}

void CircularBuffer::Clear()
{
    if(!buffer)
        return;

    readPos = bufStart;
    writePos = bufStart;
    filledSize=0;
    buffer[0]=.0f;
}

bool CircularBuffer::Put(double *buf, long size)
{
    if(!buffer)
        return false;

    if(size>buffSize) {
        size=buffSize;
    }

    if((buffSize-filledSize)<size) {
//       LOG("CircularBuffer::Put not enough free space");
       long overlapping = sizeof(float)*(size-(buffSize-filledSize));
       readPos+=overlapping;
       filledSize-=overlapping;
       while(readPos>bufEnd)
           readPos-=(buffSize*sizeof(float));
    }

    if(writePos+(size*sizeof(float)) <= bufEnd) {
        memcpy(writePos,buf,size*sizeof(float));
        writePos+=(size*sizeof(float));
        while(writePos>bufEnd)
            writePos-=(buffSize*sizeof(float));
    } else {
        for(float *dest = writePos; dest<=bufEnd; dest++) {
            writePos = (float*)buf;
            ++writePos;
            ++buf;
            --size;
        }
        writePos=bufStart;
        for(long p=0; p<size; p++) {
            writePos = (float*)buf;
            ++writePos;
            ++buf;
        }

    }
    filledSize+=size;
    return true;
}

long CircularBuffer::Put(CircularBuffer &buf, long size)
{
    if(!buffer)
        return false;

    float *currentVal = writePos-1;
    if(currentVal<bufStart)
        currentVal=bufEnd;

    float val=.0f;
    do{
        buf.Get(&val,1);
        --size;
    } while(size>0 && abs(val-*currentVal)>0.01f);


    if(size>buffSize) {
        LOG("buffer too long")
        size=buffSize;
    }

    if((buffSize-filledSize)<size) {
//       LOG("CircularBuffer::Put not enough free space");
       long overlapping = size-(buffSize-filledSize);
       readPos+=overlapping;
       filledSize-=overlapping;
       while(readPos>bufEnd)
           readPos-=buffSize;
    }

    if(writePos+size <= bufEnd) {
        buf.Get(writePos,size);
        writePos+=size;
    } else {
        long size1 = bufEnd - writePos +1;
        long size2 = size - size1;
        if(size1<0 || size2<0) {
            size1=0;
            size2=0;
            return false;
        }
        buf.Get(writePos,size1);
        buf.Get(writePos,size2);
        writePos=bufStart+size2;
    }
    while(writePos>bufEnd)
        writePos-=buffSize;
    filledSize+=size;
    return size;
}

bool CircularBuffer::Put(float *buf, long size)
{
    if(!buffer)
        return false;

    if(size>buffSize) {
        LOG("buffer too long")
        size=buffSize;
    }

    if((buffSize-filledSize)<size) {
//       LOG("CircularBuffer::Put not enough free space");
       long overlapping = size-(buffSize-filledSize);
       readPos+=overlapping;
       filledSize-=overlapping;
       while(readPos>bufEnd)
           readPos-=buffSize;
    }

    if(writePos+size <= bufEnd) {
        memcpy(writePos,buf,size*sizeof(float));
        writePos+=size;
    } else {
        long size1 = bufEnd - writePos +1;
        long size2 = size - size1;
        if(size1<0 || size2<0) {
            size1=0;
            size2=0;
            return false;
        }
        memcpy(writePos,buf,size1*sizeof(float));
        memcpy(bufStart,buf+size1,size2*sizeof(float));
        writePos=bufStart+size2;
    }
    while(writePos>bufEnd)
        writePos-=buffSize;
    filledSize+=size;
    return true;
}

bool CircularBuffer::Get(float *buf, long size)
{
    if(!buffer)
        return false;

    if(filledSize<size) {
        LOG("not enough data");
        return false;
    }

    float* readEnd = readPos+size-1;
    if(readEnd <= bufEnd) {
        memcpy(buf,readPos,size*sizeof(float));
        readPos+=size;
    } else {
        long size1 = bufEnd - readPos +1;
        long size2 = size - size1;
        memcpy(buf,readPos,size1*sizeof(float));
        memcpy(buf+size1,bufStart,size2*sizeof(float));
        readPos=bufStart+size2;
    }
    filledSize-=size;

    while(readPos>bufEnd)
        readPos-=buffSize;
    return true;
}

bool CircularBuffer::Get(double *buf, long size)
{
    if(!buffer)
        return false;

    if(filledSize<size) {
        LOG("not enough data");
        return false;
    }


    for(long i=0; i<size; i++) {
        *buf=(double)*readPos;
        buf++;
        readPos++;

        if(readPos>bufEnd) {
            readPos=bufStart;
        }
    }

    filledSize-=size;
    return true;
}

bool CircularBuffer::Skip(long size)
{
    if(!buffer)
        return false;

    if(filledSize<size) {
        LOG("can't skip more than filledsize");
        return false;
    }

    readPos+=size;
    if(readPos>bufEnd) {
        readPos=bufStart+(readPos-bufEnd);
    }
    filledSize-=size;
    return true;
}
