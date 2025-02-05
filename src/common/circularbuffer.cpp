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
#include "circularbuffer.h"
//#include "precomp.h"

CircularBuffer::CircularBuffer(qint32 size) :
    buffSize(0),
    filledSize(0),
    readPos(0),
    writePos(0),
    bufStart(0),
    bufEnd(0),
    buffer(0)
{
    SetSize(size);
}

CircularBuffer::~CircularBuffer()
{
    if(buffer)
        delete[] buffer;
}

void CircularBuffer::SetSize(qint32 size)
{
    if(size == buffSize)
        return;

    if(size!=0) {
        float *newBuf = new float[size];

        //skip the oldest data if needed
        if(filledSize>size)
            Skip(filledSize-size);

        //copy the data in the new buffer
        if(filledSize>0) {
            qint32 filled = filledSize;
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

void CircularBuffer::Fit(qint32 size) {
    if((buffSize-filledSize)<size) {
        qint32 newSize= filledSize + 2*size;
#ifdef DEBUG_BUFFERS
        LOG(QString("resize ring buf %1=>%2")
            .arg(buffSize)
            .arg(newSize));
#endif
        SetSize(newSize);
    }
}

bool CircularBuffer::Put(double *buf, qint32 size)
{
//    if(!buffer)
//        return false;
/*
    if(size>buffSize) {
        size=buffSize;
    }

    if((buffSize-filledSize)<size) {
#ifdef DEBUG_BUFFERS
//       LOG("CircularBuffer::Put not enough free space");
#endif
       qint32 overlapping = sizeof(float)*(size-(buffSize-filledSize));
       readPos+=overlapping;
       filledSize-=overlapping;
       while(readPos>bufEnd)
           readPos-=(buffSize*sizeof(float));
    }
*/

    Fit(size);

    //TODO: double to float : dither
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
        for(qint32 p=0; p<size; p++) {
            writePos = (float*)buf;
            ++writePos;
            ++buf;
        }

    }
    filledSize+=size;
    return true;
}

qint32 CircularBuffer::Put(CircularBuffer &buf, qint32 size)
{
//    if(!buffer)
//        return false;

    float *currentVal = writePos-1;
    if(currentVal<bufStart)
        currentVal=bufEnd;

    float val=.0f;
    do{
        buf.Get(&val,1);
        --size;
    } while(size>0 && abs(val-*currentVal)>0.01f);



//    if(size>buffSize) {
#ifdef DEBUG_BUFFERS
//        LOG("buffer too long")
#endif
//        size=buffSize;
//    }

//    if((buffSize-filledSize)<size) {
//       LOG("CircularBuffer::Put not enough free space");
//       qint32 overlapping = size-(buffSize-filledSize);
//       readPos+=overlapping;
//       filledSize-=overlapping;
//       while(readPos>bufEnd)
//           readPos-=buffSize;
//    }

    Fit(size);

    if(writePos+size <= bufEnd) {
        buf.Get(writePos,size);
        writePos+=size;
    } else {
        qint32 size1 = bufEnd - writePos +1;
        qint32 size2 = size - size1;
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

bool CircularBuffer::Put(float *buf, qint32 size)
{
//    if(!buffer)
//        return false;
/*
    if(size>buffSize) {
#ifdef DEBUG_BUFFERS
        LOG("buffer too long")
#endif
        size=buffSize;
    }

    if((buffSize-filledSize)<size) {
#ifdef DEBUG_BUFFERS
//       LOG("CircularBuffer::Put not enough free space");
#endif
        qint32 overlapping = size-(buffSize-filledSize);
       readPos+=overlapping;
       filledSize-=overlapping;
       while(readPos>bufEnd)
           readPos-=buffSize;
    }
*/
    Fit(size);

    if(writePos+size <= bufEnd) {
        memcpy(writePos,buf,size*sizeof(float));
        writePos+=size;
    } else {
        qint32 size1 = bufEnd - writePos +1;
        qint32 size2 = size - size1;
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

bool CircularBuffer::Get(float *buf, qint32 size)
{

    if(!buffer)
        return false;

    if(filledSize<size) {
#ifdef DEBUG_BUFFERS
        LOG("not enough data");
#endif
        return false;
    }

    float* readEnd = readPos+size-1;
    if(readEnd <= bufEnd) {
        memcpy(buf,readPos,size*sizeof(float));
        readPos+=size;
    } else {
        qint32 size1 = bufEnd - readPos +1;
        qint32 size2 = size - size1;
        memcpy(buf,readPos,size1*sizeof(float));
        memcpy(buf+size1,bufStart,size2*sizeof(float));
        readPos=bufStart+size2;
    }
    filledSize-=size;

    while(readPos>bufEnd)
        readPos-=buffSize;

    return true;
}

bool CircularBuffer::Get(double *buf, qint32 size)
{
    if(!buffer)
        return false;

    if(filledSize<size) {
#ifdef DEBUG_BUFFERS
        LOG("not enough data");
#endif
        return false;
    }


    for(qint32 i=0; i<size; i++) {
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

bool CircularBuffer::Skip(qint32 size)
{
    if(!buffer)
        return false;

    if(filledSize<size) {
#ifdef DEBUG_BUFFERS
        LOG("can't skip more than filledsize");
#endif
        return false;
    }

    readPos+=size;
    if(readPos>bufEnd) {
        readPos=bufStart+(readPos-bufEnd);
    }
    filledSize-=size;
    return true;
}
