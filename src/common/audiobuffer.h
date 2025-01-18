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

#ifndef AUDIOBUFFER_H
#define AUDIOBUFFER_H

#define BLANK_BUFFER_SIZE 44100

//#include "precomp.h"

#include "views/audiograph.h"


class AudioBuffer
{
public:
        AudioBuffer(bool doublePrecision, bool externAlloc);
        ~AudioBuffer();
        void SetBufferPointer(void *pt) {if(!externAlloc)return; pBuffer=pt;}
        bool SetSize(unsigned long size, bool forceRealloc=false);
        void AddToStack(const AudioBuffer * buff);
        void SetBufferContent(float *buff, unsigned long count);
        void SetBufferContent(double *buff, unsigned long count);
        void DumpToBuffer(float *buff, unsigned long count);
        void DumpToBuffer(double *buff, unsigned long count);

        void *GetPointer() const {return pBuffer;}
        void *GetPointerWillBeFilled();
        void *ConsumeStack();
        void SetDoublePrecision(bool dbl);
        inline bool GetDoublePrecision() const {return doublePrecision;}
        float GetVu();

        void AddGraph();

        /// \return the current buffer size
        inline unsigned long GetSize() const {return bufferSize;}

        /*!
         Get the last vu-meter value. Don't reset the peak value (used by bridges)
         \return the vu-meter value
        */
        float GetCurrentVu() {return currentVu;}

        /*!
          Clear the stack. Empty the buffer
          */
        inline void ResetStackCounter() {stackSize=0;}

        /// a blank buffer
        static float const blankBuffer[BLANK_BUFFER_SIZE];

protected:

        View::AudioGraph *audiograph;

        /// the stack size
        unsigned long stackSize;

        /// pointer to the audio buffer
        void * pBuffer;

        /// buffer size
        unsigned long bufferSize;

        /// allocated buffer size, can be bigger than the useable buffer size
        unsigned long allocatedSize;

        /// vu-meter peak
        float _maxVal;

        /// vu-meter value
        float currentVu;

        /// true if we're not the owned of the buffer
        bool externAlloc;

        bool doublePrecision;
};


#endif // AUDIOBUFFER_H
