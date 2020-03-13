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

#include "public.sdk/source/vst/vst2wrapper/vst2wrapper.h"

#ifndef MYVST2WRAPPER_H
#define MYVST2WRAPPER_H

using namespace Steinberg;

class MyVst2Wrapper : public Vst::Vst2Wrapper
{
public:
    static AudioEffect* crt (IPluginFactory* factory, const TUID vst3ComponentID, VstInt32 vst2ID, audioMasterCallback audioMaster);
    MyVst2Wrapper (Vst::IAudioProcessor* processor, Vst::IEditController* controller, audioMasterCallback audioMaster, const TUID vst3ComponentID, VstInt32 vst2ID, IPluginFactory* factory = 0);
    VstInt32 processEvents (VstEvents* events);
    void processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames);
    void processDoubleReplacing (double** inputs, double** outputs, VstInt32 sampleFrames);
    void onTimer (Timer* timer);

private:
    void doProcess (VstInt32 sampleFrames);
    void processOutputEvents ();
};

#endif // MYVST2WRAPPER_H
