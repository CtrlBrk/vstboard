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

#include "buffer.h"
#include "mainhost.h"

//using namespace Connectables;
namespace Connectables
{
Buffer::Buffer(MainHost *host, int index, const ObjectInfo &info) :
    Object(host,index,ObjectInfo(NodeType::object, ObjType::Buffer, tr("Delay"))),
    delayChanged(false),
    addedSize(0),
    offset(0),
    adjustDelay(0),
    countWait(0),
    desiredSize(0)
{
    QMutexLocker l(&mutexBuffer);
    delaySize = info.initDelay;
    buffer.SetSize(myHost->GetBufferSize() + delaySize);
    listAudioPinIn->SetNbPins(1);
    listAudioPinOut->SetNbPins(1);
    listParameterPinIn->listPins.insert(0, new ParameterPinIn(this,0,(float)delaySize/50000,"Delay",true));
    resizeBuffer.SetSize(4000);
}

Buffer::~Buffer()
{
    QMutexLocker l(&mutexBuffer);
    if(adjustDelay)
        delete[] adjustDelay;
}

void Buffer::SetDelay(long d)
{
    QMutexLocker l(&mutexBuffer);
    desiredSize=d;
    delayChanged=10;
    static_cast<ParameterPin*>(listParameterPinIn->GetPin(0))->ChangeValue((float)d/50000, true);
}

void Buffer::Resize()
{
    if(desiredSize<delaySize) {
        //fast forward
        long tmpSize = delaySize-desiredSize;
        if(tmpSize>buffer.filledSize) {
            tmpSize = buffer.filledSize;
        }
        delaySize-=tmpSize;
        buffer.SetSize(myHost->GetBufferSize() + delaySize);
    }

    if(desiredSize>delaySize) {
        AudioBuffer *pinInBuf = listAudioPinIn->GetBuffer(0);
        long tmpSize = desiredSize-delaySize;
        if(tmpSize>pinInBuf->GetSize()) {
            tmpSize = pinInBuf->GetSize();
        }

        resizeBuffer.Put( (float*)pinInBuf->GetPointer(), tmpSize );

        if(resizeBuffer.filledSize>3000) {
            long canAdd=resizeBuffer.filledSize;
            if(delaySize+canAdd > desiredSize)
                canAdd = desiredSize-delaySize;

            delaySize+=canAdd;
            buffer.SetSize(myHost->GetBufferSize() + delaySize);
            buffer.Put(resizeBuffer,canAdd);
        }
    }
}

void Buffer::Render()
{
    QMutexLocker l(&mutexBuffer);

    AudioBuffer *pinInBuf = listAudioPinIn->GetBuffer(0);
    AudioBuffer *pinOutBuf = listAudioPinOut->GetBuffer(0);

    if(buffer.buffSize < delaySize+pinOutBuf->GetSize()) {
        buffer.SetSize(delaySize+pinOutBuf->GetSize());
    }

    float *buf = (float*)pinInBuf->ConsumeStack();
    buffer.Put( buf, pinInBuf->GetSize() );
    pinInBuf->ResetStackCounter();

    if(delayChanged>0) {
        --delayChanged;
    }
    if(delayChanged==0) {
        Resize();
    }

    if(buffer.filledSize>=delaySize+pinOutBuf->GetSize() ) {
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
    QMutexLocker l(&mutexBuffer);

    Object::OnParameterChanged(pinInfo,value);
    desiredSize = listParameterPinIn->listPins.value(0)->GetValue()*50000;
    delayChanged=10;
}
}
