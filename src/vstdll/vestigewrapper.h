//------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Helpers
// Filename    : public.sdk/source/vst/aaxwrapper/aaxwrapper.h
// Created by  : Steinberg, 08/2017
// Description : VST 3 -> AAX Wrapper
//
//-----------------------------------------------------------------------------
// LICENSE
// (c) 2024, Steinberg Media Technologies GmbH, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this
//     software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

///**************************************************************************
//#    Copyright 2010-2020 Raphaël François
//#    Contact : ctrlbrk76@gmail.com
//#
//#    This file is part of VstBoard.
//#
//#    VstBoard is free software: you can redistribute it and/or modify
//#    it under the terms of the under the terms of the GNU Lesser General Public License as published by
//#    the Free Software Foundation, either version 3 of the License, or
//#    (at your option) any later version.
//#
//#    VstBoard is distributed in the hope that it will be useful,
//#    but WITHOUT ANY WARRANTY; without even the implied warranty of
//#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#    under the terms of the GNU Lesser General Public License for more details.
//#
//#    You should have received a copy of the under the terms of the GNU Lesser General Public License
//#    along with VstBoard.  If not, see <http://www.gnu.org/licenses/>.
//**************************************************************************/

//#pragma warning ( push, 1 )
//#include "public.sdk/source/vst/z vst2wrapper/vst2wrapper.h"
//#pragma warning ( pop )

#include "../vestige.h"
#include "public.sdk/source/vst/basewrapper/basewrapper.h"
//#include "public.sdk/source/main/pluginfactory.h"
#ifndef VestigeWrapper_H
#define VestigeWrapper_H

using namespace Steinberg;
using namespace Steinberg::Vst;

intptr_t static_dispatcher(AEffect*, int, int, intptr_t, void*, float);
void static_setParameter(AEffect*, int, float);
float static_getParameter(AEffect*, int);
void static_process(AEffect*, float**, float**, int) ;
void static_processReplacing(AEffect*, float**, float**, int);

class VestigeEditorWrapper;

class VestigeWrapper  : public BaseWrapper
{
public:
   VestigeWrapper (BaseWrapper::SVST3Config config, audioMasterCallback audioMaster);
    static VestigeWrapper* create (IPluginFactory* factory, const TUID vst3ComponentID, audioMasterCallback audioMaster);

    bool init () override;
    tresult PLUGIN_API getName (String128 name) override;
    bool _sizeWindow (int32 width, int32 height) override;
    void setupProcessTimeInfo () override;
    tresult PLUGIN_API beginEdit (ParamID tag) override;
    tresult PLUGIN_API performEdit (ParamID tag, ParamValue valueNormalized) override;
    tresult PLUGIN_API endEdit (ParamID tag) override;
    int processEvents (VstEvents* events);
    void processMidiEvent (Event& toAdd, char* midiData, bool isLive, int32 noteLength, float noteOffVelocity, float detune, int size);

    intptr_t __dispatcher(int, int, intptr_t, void*, float);
    float __getParameter( int index);
    void __setParameter(int index, float value);
    void __processReplacing (float** inputs, float** outputs, int sampleFrames);

    VstPlugCategory getPlugCategory ();
    int startProcess ();
    int stopProcess ();


    // VestigeEditorWrapper* _editor {0};
    Steinberg::int32 _getChunk (void** data, bool isPreset) SMTG_OVERRIDE;
    Steinberg::int32 _setChunk (void* data, Steinberg::int32 byteSize, bool isPreset) SMTG_OVERRIDE;


    char mSubCategories[PClassInfo2::kSubCategoriesSize];
    char mName[PClassInfo::kNameSize];
    char mVendor[PFactoryInfo::kNameSize];
    int32 mVersion = 0;

    AEffect Effect;
private:
    struct GetChunkMessage;

};

#endif // VestigeWrapper_H
