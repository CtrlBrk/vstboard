/**************************************************************************
#    Copyright 2010-2012 Raphaël François
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

#ifndef IDS_H
#define IDS_H

#include "pluginterfaces/base/funknown.h"

#define uniqueIDEffect CCONST('V','b','P','l')
#define uniqueIDInstrument CCONST('V','b','I','s')

static const Steinberg::FUID VstBoardProcessorUID       (0x6183A41C, 0x584C4795, 0xA819AB96, 0xF04CBA61);
static const Steinberg::FUID VstBoardInstProcessorUID   (0x302F1D1A, 0x68A511E1, 0x9D995F2E, 0x4924019B);
static const Steinberg::FUID VstBoardControllerUID      (0x999B210E, 0x45964a25, 0x91A5ACDC, 0xF8EB9481);

#endif // IDS_H
