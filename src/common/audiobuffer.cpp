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

#include "audiobuffer.h"

//#define DEBUGBUFFERS

#ifdef DEBUGBUFFERS
    #define debugbuf(...) qDebug()<<"AudioBuffer"<<this<<doublePrecision<<__VA_ARGS__
#else
    #define debugbuf(...)
#endif

/*!
  \class AudioBuffer
  \brief a single precision resizeable audio buffer
  */

float const AudioBuffer::blankBuffer[] = {.0f};

/*!
  \param externalAllocation true if the buffer pointer is handle by something else : don't create or delete it
  \todo check if the externalAllocation is still in use
  */
AudioBuffer::AudioBuffer(bool doublePrecision, bool externAlloc) :
    stackSize(0),
    pBuffer(0),
    bufferSize(0),
    allocatedSize(0),
    _maxVal(.0f),
    currentVu(.0f),
    externAlloc(externAlloc),
    doublePrecision(doublePrecision)
{
    debugbuf("new double:"<<doublePrecision<<"extern:"<<externAlloc);
}

/*!
  Delete the buffer if it's not an external
  */
AudioBuffer::~AudioBuffer()
{
    debugbuf("delete");

    if(pBuffer && !externAlloc)
        delete[] pBuffer;
}

/*!
  Set the size of the buffer (number of samples)
  Keep the memory allocated if smaller, reallocate only if the new size is bigger
  Don't allocate anything if it's externally allocated
  \param size the new size
  */
bool AudioBuffer::SetSize(unsigned long newSize, bool forceRealloc)
{
    debugbuf("setsize old:"<<bufferSize<<"new"<<newSize);

    if(!forceRealloc && newSize==bufferSize)
        return true;

    if(!externAlloc) {
        if( newSize <= allocatedSize) {
            bufferSize = newSize;
            return true;
        }

        if(pBuffer)
            delete[] pBuffer;

        if(doublePrecision)
            pBuffer = new double[newSize];
        else
            pBuffer = new float[newSize];

        if(!pBuffer) {
            pBuffer=0;
            return false;
        }
    }

    bufferSize = allocatedSize = newSize;
    if(!externAlloc) {
        if(!doublePrecision)
            memset( pBuffer, 0, sizeof(float)*bufferSize );
        else
            memset( pBuffer, 0, sizeof(double)*bufferSize );
    }
    return true;
}

void AudioBuffer::SetDoublePrecision(bool dbl)
{
    if(doublePrecision==dbl)
        return;

    debugbuf("setdouble"<<dbl);

    doublePrecision=dbl;

    if(doublePrecision)
        allocatedSize/=2;
    else
        allocatedSize*=2;

    SetSize(bufferSize,true);
}

/*!
  Add a buffer to the stack. Can resize the buffer if needed if the current stack is empty
  \param buff buffer to be added to the mix
  */
void AudioBuffer::AddToStack(const AudioBuffer * buff)
{
    debugbuf("addtostak other:"<<buff->GetDoublePrecision());

    if(buff->GetSize()!=bufferSize) {
        if(stackSize==0) {
            SetSize(buff->GetSize());
        } else {
            LOG("not the same size, abort"<<buff->GetSize()<<bufferSize);
            return;
        }
    }

    if(stackSize==0) {
        debugbuf("1st stack");
        //1st in the stack, copying is enough

        if(!doublePrecision) {
            if(!buff->GetDoublePrecision()) {
                memcpy(pBuffer,buff->GetPointer(),sizeof(float)*bufferSize);
            } else {
                //convert from double to float
                float *dest=(float*)pBuffer;
                double *ori=(double*)buff->GetPointer();
                long i=bufferSize;
                while(i>0) {
                    *dest=(float)*ori;
                    ++dest;
                    ++ori;
                    --i;
                }
            }
        } else {
            if(buff->GetDoublePrecision()) {
                memcpy(pBuffer,buff->GetPointer(),sizeof(double)*bufferSize);
            } else {
                //convert from float to double
                double *dest=(double*)pBuffer;
                float *ori=(float*)buff->GetPointer();
                long i=bufferSize;
                while(i>0) {
                    *dest=(double)*ori;
                    ++dest;
                    ++ori;
                    --i;
                }
            }
        }
    } else {
        debugbuf("add to stack");
        //add the next buffers to the stack

        if(!doublePrecision) {
            if(!buff->GetDoublePrecision()) {
                float *buffToAdd = (float*)buff->GetPointer();
                float *myBuff = (float*)pBuffer;
                long i=bufferSize;
                while(i>0) {
                    *myBuff += *buffToAdd;
                    ++myBuff;
                    ++buffToAdd;
                    --i;
                }
            } else  {
                //convert from double to float
                double *buffToAdd = (double*)buff->GetPointer();
                float *myBuff = (float*)pBuffer;
                long i=bufferSize;
                while(i>0) {
                    *myBuff += (float)*buffToAdd;
                    ++myBuff;
                    ++buffToAdd;
                    --i;
                }
            }
        } else {
            if(buff->GetDoublePrecision()) {
                double *buffToAdd = (double*)buff->GetPointer();
                double *myBuff = (double*)pBuffer;
                long i=bufferSize;
                while(i>0) {
                    *myBuff += *buffToAdd;
                    ++myBuff;
                    ++buffToAdd;
                    --i;
                }
            } else {
                //convert from float to double
                float *buffToAdd = (float*)buff->GetPointer();
                double *myBuff = (double*)pBuffer;
                long i=bufferSize;
                while(i>0) {
                    *myBuff += (double)*buffToAdd;
                    ++myBuff;
                    ++buffToAdd;
                    --i;
                }
            }
        }
    }
    stackSize++;
}

/*!
  Get the pointer of the audio buffer
  \param willBeFilled true if we get this pointer to replace the content of the buffer with a new sound
  */
void * AudioBuffer::GetPointerWillBeFilled()
{
    stackSize=1;
    return pBuffer;
}

/*!
  Flatten the stack and return the resulting pointer.
  All the stacks are mixed together in one resulting buffer
  Update the vu-meter value
  \return pointer to the resulting buffer
  */
void *AudioBuffer::ConsumeStack()
{
    //debugbuf("consume");

    if(!doublePrecision) {
        float ma = .0f;
        float mi = .0f;
        float *buf;

        if(stackSize==0) {
            //empty stack : return a blank buffer
            memset( pBuffer, 0, sizeof(float)*bufferSize );
            _maxVal = 0.0f;
            return pBuffer;
        } else {
            //find max value
            buf = (float*)pBuffer;
            long i=bufferSize;
            while(i>0) {
                if(*buf > ma)
                    ma = *buf;
                if(mi > *buf)
                    mi = *buf;
                ++buf;
                --i;
            }
        }

        float m = std::max(std::abs(ma),std::abs(mi));
        if( m > _maxVal)
            _maxVal = m;

#ifdef BUFFER_ZERODB_CLIPPING
        //if we're off-limits : here is a limiter
        if(_maxVal > 1.0f) {
            buf = (float*)pBuffer;
            long i=bufferSize;
            while(i>0) {
                if(*buf > 1.0f)
                    *buf = .8f;
                if(*buf < -1.0f)
                    *buf = -.8f;
               ++buf;
               --i;
            }
            _maxVal = 1.0f;
        }
#endif
    } else {
        double ma = .0;
        double mi = .0;
        double *buf;

        if(stackSize==0) {
            //empty stack : return a blank buffer
            memset( pBuffer, 0, sizeof(double)*bufferSize );
            _maxVal = 0.0f;
            return pBuffer;
        } else {
            //find max value
            buf = (double*)pBuffer;
            long i=bufferSize;
            while(i>0) {
                if(mi > *buf)
                    mi = *buf;
                if(*buf > ma)
                    ma = *buf;
                ++buf;
                --i;
            }
        }

        float m = (float)std::max(std::abs(ma),std::abs(mi));
        if( m > _maxVal)
            _maxVal = m;

#ifdef BUFFER_ZERODB_CLIPPING
        //if we're off-limits : here is a limiter
        if(_maxVal > 1.0) {
            buf = (double*)pBuffer;
            long i=bufferSize;
            while(i>0) {
                if(*buf > 1.0)
                    *buf = .8;
                if(*buf < -1.0)
                    *buf = -.8;
               ++buf;
               --i;
            }
            _maxVal = 1.0;
        }
#endif
    }

    return pBuffer;
}

/*!
  Get the vu-meter value and reset the peak
  */
float AudioBuffer::GetVu()
{
    currentVu=_maxVal;
    _maxVal=.0f;
    return currentVu;
}

void AudioBuffer::SetBufferContent(float *buff, int count)
{
    debugbuf("setcontent float");

    if(count!=bufferSize) {
//        LOG("resize buffer"<<bufferSize<<count);
        SetSize(count);
    }

    if(doublePrecision) {
        double *dest = (double*)pBuffer;
        long i = count;
        while(i>0) {
            *dest=(double)*buff;
            ++dest;
            ++buff;
            --i;
        }
    } else {
        memcpy(pBuffer,buff,count*sizeof(float));
    }
    stackSize=1;
}

void AudioBuffer::SetBufferContent(double *buff, int count)
{
    debugbuf("setcontent double");

    if(count!=bufferSize) {
        LOG("resize buffer"<<bufferSize<<count);
        SetSize(count);
    }

    if(doublePrecision) {
        memcpy(pBuffer,buff,count*sizeof(double));
    } else {
        float *dest = (float*)pBuffer;
        long i = count;
        while(i>0) {
            *dest=(float)*buff;
            ++dest;
            ++buff;
            --i;
        }
    }
    stackSize=1;
}

void AudioBuffer::DumpToBuffer(float *buff, unsigned long count)
{
    debugbuf("dump float");

    if(count>bufferSize) {
//        LOG("buffer too small");
        count=bufferSize;
    }

    if(doublePrecision) {
        double *ori = (double*)pBuffer;
        unsigned long i = count;
        while(i>0) {
            *buff=(float)*ori;
            ++ori;
            ++buff;
            --i;
        }
    } else {
        memcpy(buff,pBuffer,count*sizeof(float));
    }
}

void AudioBuffer::DumpToBuffer(double *buff, unsigned long count)
{
    debugbuf("dump double");

    if(count>bufferSize) {
//        LOG("buffer too small");
        count=bufferSize;
    }

    if(doublePrecision) {
        memcpy(buff,pBuffer,count*sizeof(double));
    } else {
        float *ori = (float*)pBuffer;
        unsigned long i = count;
        while(i>0) {
            *buff=(double)*ori;
            ++ori;
            ++buff;
            --i;
        }
    }
}
