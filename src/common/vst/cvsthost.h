/******************************************************************************
#   Copyright 2006 Hermann Seib
#   Copyright 2010 RaphaÃ«l FranÃ§ois
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
******************************************************************************/

/*****************************************************************************/
/* CVSTHost.h: interface for the CVSTHost class.                             */
/*****************************************************************************/

#if !defined(VSTHOST_H__INCLUDED_)
#define VSTHOST_H__INCLUDED_

#ifdef _MSC_VER
#pragma warning( disable: 4100 )
#endif

#include "../precomp.h"
#include "const.h"
//#include "vstbank.h"

namespace vst
{

    /*****************************************************************************/
    /* CVSTHost class declaration                                                */
    /*****************************************************************************/

    class CVSTHost
    {
    friend class CEffect;
    public:
        static CVSTHost *Get() {return pHost;}
        CVSTHost();
        virtual ~CVSTHost();

        void SetTimeInfo(const VstTimeInfo *info);
        void SetTempo(int tempo, int sign1=0, int sign2=0);
        void GetTempo(int &tempo, int &sign1, int &sign2);
        void UpdateTimeInfo(double timer, int addSamples, double sampleRate);

        int loopLenght;
        VstTimeInfo vstTimeInfo;
        int currentBar;
        float barLengthq;

    protected:
        static CVSTHost * pHost;
        static VstIntPtr VSTCALLBACK AudioMasterCallback(AEffect *effect, VstInt32 opcode, VstInt32  index, VstIntPtr  value, void *ptr, float opt);
    };

}
#endif // !defined(VSTHOST_H__INCLUDED_)
