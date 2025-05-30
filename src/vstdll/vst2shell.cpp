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

#include "vst2shell.h"

AudioEffect *createShell(audioMasterCallback audioMaster)
{
    return new Vst2Shell(audioMaster);
}

Vst2Shell::Vst2Shell(audioMasterCallback audioMaster) :
    AudioEffectX(audioMaster,0,0),
    currentShellPlugin(0L)
{
}

VstPlugCategory Vst2Shell::getPlugCategory()
{
    return kPlugCategShell;
}

VstInt32 Vst2Shell::getNextShellPlugin(char *name)
{
    switch(currentShellPlugin++) {
    case 0:
        strcpy_s(name,256,"VstBoard Effect");
        return uniqueIDEffect;
    case 1:
        strcpy_s(name,256,"VstBoard Instrument");
        return uniqueIDInstrument;
    }
    return 0;
}

void Vst2Shell::processReplacing (float** , float** , VstInt32 )
{
}
