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
//        memset(newBuf,0,size*sizeof(float));
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

bool CircularBuffer::Put(float *buf, long size)
{
    if(!buffer)
        return false;

    if(size>buffSize) {
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

//long CircularBuffer::PutAtZero(float *buf, long size)
//{
//    float *lastPos = writePos;
//    if(lastPos==bufStart)
//        lastPos=bufEnd;
//    else
//        --lastPos;

//    //find the last zero crossing on the ring buffer
//    bool currentSign = (*lastPos>0);
//    float *destPos=lastPos;
//    while( (*destPos>0)==currentSign && destPos!=writePos) {
//        --destPos;
//        if(destPos<bufStart)
//            destPos+=buffSize;
//    }
//    if(destPos==writePos) {
//        LOG("zero not found");
//        destPos=bufEnd;
//    }

//    //find the first zero crossing on the added buffer
//    float *startPos=buf;
//    currentSign = (*startPos>0);
//    while( (*startPos>0)==currentSign && startPos<buf+size) {
//        ++startPos;
//    }
//    if(startPos==buf+size) {
//        LOG("zero not found");
//        startPos=buf;
//    }


//}


bool CircularBuffer::Get(float *buf, long size)
{
    if(!buffer)
        return false;

    if(filledSize<size) {
        LOG("not enough data");
        return false;
    }

    if(readPos+size <= bufEnd) {
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

bool CircularBuffer::SetWritePosToLastZeroCrossing()
{
    float *pos = writePos;
    if(writePos==bufStart)
        pos=bufEnd;
    else
        --pos;

    long cpt=0;
    bool sign=(*pos>0);
    while((*pos>0)==sign) {
        --pos;
        if(pos<bufStart)
            pos=bufEnd;

        if(pos==writePos || pos==readPos) {
//            LOG("zero point not found");
            return false;
        }
        ++cpt;
    }
    ++pos;
    ++pos;
    --cpt;
    writePos=pos;
    filledSize-=cpt;

    while(writePos>bufEnd)
        writePos-=buffSize;

    return true;
}

//bool CircularBuffer::Keep(unsigned int size) {
//    if(filledSize<size) {
//        debug("CircularBuffer::Get can't keep more than filledsize")
//        return false;
//    }

//    filledStart=filledEnd-size;
//    if(filledStart<bufStart) {
//        filledStart=bufEnd-(bufStart-filledStart);
//    }
//    filledSize=size;
//    return true;
//}


///*!
//  Stretch the filled data to the needed size
//*/
//void CircularBuffer::Stretch(unsigned int neededSize)
//{
//    if(filledSize==0)
//        return;

//    //get what we have
//    float *tmpOri = new float[filledSize];
//    float *tmpDest = new float[neededSize];
//    unsigned int filled = filledSize;
//    filledSize=buffSize;
//    Get(tmpOri,filledSize);

//    unsigned int chunkSize=3000;

//    if(filled<chunkSize)
//        chunkSize=filled;

//    float ratio = (float)filled/neededSize;
//    int nbSteps = neededSize/chunkSize;
//    for(int dest=nbSteps-1; dest>=0; --dest) {
//        int ori = ratio*dest;
//        memcpy(tmpDest+(dest*chunkSize), tmpOri+(ori*chunkSize), chunkSize*sizeof(float));
//    }
//    memcpy(tmpDest+neededSize-chunkSize, tmpOri+filled-chunkSize, chunkSize*sizeof(float));
//    filledSize=0;
//    Put(tmpDest,neededSize);
//    delete[] tmpOri;
//    delete[] tmpDest;
//}
