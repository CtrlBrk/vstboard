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

#include "../precomp.h"

#ifdef _MSC_VER
#pragma warning( push, 1 )
#endif
//#include "pluginterfaces/vst2.x/aeffect.h"
#include "public.sdk/source/vst2.x/audioeffectx.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

namespace vst
{
    typedef AEffect *(*vstPluginFuncPtr)(audioMasterCallback host);
    typedef VstIntPtr (*dispatcherFuncPtr)(AEffect *effect, VstInt32 opCode, VstInt32 index, VstInt32 value, void *ptr, float opt);


    #if defined(VST_2_4_EXTENSIONS)
    #else
    typedef int VstInt32;                   /* this one's heavily used in V2.4++ */
    // ... and Steinberg goofed big time by making this 'typedef int VstInt32' in
    // the original VST SDK 2.4, which is architecture-dependent...
    #endif


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


}

#endif // VSTCONST_H
