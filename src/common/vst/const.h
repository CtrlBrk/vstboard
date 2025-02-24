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

#ifndef VSTCONST_H
#define VSTCONST_H

// #include "../precomp.h"

#ifdef VST24SDK
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning (disable: 4100)
#endif
#include "public.sdk/source/vst2.x/audioeffectx.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#else
#include "../vestige.h"
#endif

namespace vst
{
    typedef AEffect *(*vstPluginFuncPtr)(audioMasterCallback host);
    typedef int (*dispatcherFuncPtr)(AEffect *effect, int opCode, int index, int value, void *ptr, float opt);

/*

    #if !defined(VST_2_1_EXTENSIONS)
    struct VstFileSelect;
    //---Structure and enum used for keyUp/keyDown-----
    struct VstKeyCode
    {
            long character;
            unsigned char virt;     // see enum VstVirtualKey
            unsigned char modifier; // see enum VstModifierKey
    };
    struct MidiProgramName;
    struct MidiProgramCategory;
    struct MidiKeyName;
    #endif
*/

}

#endif // VSTCONST_H
