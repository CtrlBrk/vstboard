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

#include "buffer.h"
#include "mainhost.h"

using namespace Connectables;

Buffer::Buffer(MainHost *host, int index, const ObjectInfo &info) :
    Object(host,index,ObjectInfo(NodeType::object, ObjType::Buffer, tr("Delay"))),
    delayChanged(false),
    addedSize(0),
    offset(0),
    adjustDelay(0),
    countWait(0)
{
    delaySize = info.initDelay;
    buffer.SetSize(myHost->GetBufferSize()*2 + delaySize);
    listAudioPinIn->SetNbPins(1);
    listAudioPinOut->SetNbPins(1);
    listParameterPinIn->listPins.insert(0, new ParameterPinIn(this,0,(float)delaySize/50000,"Delay",true));
}

Buffer::~Buffer()
{
    if(adjustDelay)
        delete[] adjustDelay;
}

void Buffer::SetDelay(long d)
{
//    objMutex.lock();
    addedSize+=(d-delaySize);
    delaySize=d;
    delayChanged=true;
    static_cast<ParameterPin*>(listParameterPinIn->GetPin(0))->ChangeValue((float)d/50000, true);
//    objMutex.unlock();
}

void Buffer::Render()
{
//    QMutexLocker l(&objMutex);

    if(delayChanged) {
        if(addedSize<500 && countWait<10) {
            //wait for more
            ++countWait;
        } else {
            countWait=0;
            delayChanged=false;
            buffer.SetSize(myHost->GetBufferSize()*2 + delaySize);
        }

        if(addedSize<0) {
            addedSize=0;
            offset=0;
        }


    }

    //get buffer from input
    AudioBuffer *pinInBuf = listAudioPinIn->GetBuffer(0);
    if(pinInBuf->GetDoublePrecision())
        buffer.Put( (double*)pinInBuf->ConsumeStack(), pinInBuf->GetSize() );
    else {
        float *buf = (float*)pinInBuf->ConsumeStack();
        buffer.Put( buf, pinInBuf->GetSize() );

        if(addedSize>0 && countWait==0) {
            if(!adjustDelay)
                adjustDelay = new float[4000];

            memcpy( adjustDelay+offset, buf, pinInBuf->GetSize()*sizeof(float) );
            offset+=pinInBuf->GetSize();

            float *start=0;
            long size=0;
            if(offset>=addedSize) {
                start=adjustDelay;
                size=addedSize;
            } else if(offset>=buffer.buffSize) {
                start=adjustDelay;
                size=buffer.buffSize;
            } else if(offset>=1500) {
                start=adjustDelay;
                size=offset;
            }
            if(size>0) {
//                CutBufferAtZeroCrossing(start,size);
//                buffer.SetWritePosToLastZeroCrossing();
                buffer.Put(start, size);
                addedSize-=size;
                offset=0;
                if(addedSize<=0 && adjustDelay) {
                    delete[] adjustDelay;
                    adjustDelay=0;
                }
            }
        }
    }
    pinInBuf->ResetStackCounter();

    AudioBuffer *pinOutBuf = listAudioPinOut->GetBuffer(0);
    if(buffer.filledSize>=delaySize+pinOutBuf->GetSize() || addedSize>0 ) {
        //set buffer to output
        if(pinOutBuf->GetDoublePrecision())
            buffer.Get( (double*)pinOutBuf->GetPointerWillBeFilled(), pinOutBuf->GetSize() );
        else
            buffer.Get( (float*)pinOutBuf->GetPointerWillBeFilled(), pinOutBuf->GetSize() );

        pinOutBuf->ConsumeStack();
        static_cast<AudioPin*>(listAudioPinOut->listPins.value(0))->SendAudioBuffer();
    }
}

void Buffer::OnParameterChanged(ConnectionInfo pinInfo, float value)
{
    Object::OnParameterChanged(pinInfo,value);
    long d = listParameterPinIn->listPins.value(0)->GetValue()*50000;
//    objMutex.lock();
    if(abs(d-delaySize)>1) {
        addedSize+=(d-delaySize);
        delaySize=d;
        delayChanged=true;
    }
//    objMutex.unlock();
}

bool Buffer::CutBufferAtZeroCrossing(float *buffer, long size)
{
    if(size<10)
        return false;

    if(size<200) {
        for(int i=10;i>=0;--i) {
            buffer[i]*=(float)i/10;
            buffer[size-1-i]*=(float)i/10;
        }
        return true;
    }

    float *start=buffer;
    float *end=buffer+size-1;

    bool sign=(*start>0);
    while((*start>0)==sign) {
        ++start;
        if(start==end) {
//            LOG("zero not found");
            return false;
        }
    }
    --start;

    sign=(*end>0);
    while((*end<0)==sign) {
        --end;
        if(start==end) {
//            LOG("zero not found");
            return false;
        }
    }
    ++end;

    buffer=start;
    size=end-start+1;
    return true;
}
