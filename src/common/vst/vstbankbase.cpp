/******************************************************************************
#   Copyright 2006 Hermann Seib
#   Copyright 2010 Raphaël François
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
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning (disable: 4100)
#endif
#include "vstbankbase.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

using namespace vst;


/*===========================================================================*/
/* CFxBase class members                                                     */
/*===========================================================================*/

static char szChnk[] = "CcnK";          /* set up swapping flag              */
static long lChnk = 'CcnK';
bool CFxBase::NeedsBSwap = !!memcmp(szChnk, &lChnk, 4);

/*****************************************************************************/
/* SwapBytes : swaps bytes for big/little-endian difference                  */
/*****************************************************************************/

void CFxBase::SwapBytes(long &l)
{
    unsigned char *b = (unsigned char *)&l;
    long intermediate =  ((long)b[0] << 24) |
                         ((long)b[1] << 16) |
                         ((long)b[2] << 8) |
                         (long)b[3];
    l = intermediate;
}

void CFxBase::SwapBytes(float &f)
{
    long *pl = (long *)&f;
    SwapBytes(*pl);
}

#if defined(VST_2_4_EXTENSIONS)
void CFxBase::SwapBytes(VstInt32 &vi)
{
    unsigned char *b = (unsigned char *)&vi;
    VstInt32 intermediate =  ((VstInt32)b[0] << 24) |
                             ((VstInt32)b[1] << 16) |
                             ((VstInt32)b[2] << 8) |
                             (VstInt32)b[3];
    vi = intermediate;
}
#endif

