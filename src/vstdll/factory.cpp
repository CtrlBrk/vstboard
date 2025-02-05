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
#include "public.sdk/source/main/pluginfactory.h"
#include "vstboardcontroller.h"
#include "vstboardprocessor.h"
#include "ids.h"
#include "vsttest.h"

BEGIN_FACTORY_DEF ("CtrlBrk",
                   "http://vstboard.blogspot.com",
                   "mailto:ctrlbrk76@gmail.com")

        DEF_CLASS2 (INLINE_UID_FROM_FUID(VstBoardProcessorUID),
                                PClassInfo::kManyInstances,
                                kVstAudioEffectClass,
                                "VstBoard",
                                Vst::kDistributable,
                                Vst::PlugType::kFx,
                                QString("%1.%2.%3").arg(APP_VERSION_MAJOR).arg(APP_VERSION_MINOR).arg(APP_VERSION_BUILD).toLatin1(),
                                kVstVersionString,
                                VstBoardProcessor::createInstance)

        DEF_CLASS2 (INLINE_UID_FROM_FUID(VstBoardInstProcessorUID),
                               PClassInfo::kManyInstances,
                               kVstAudioEffectClass,
                               "VstBoardinstrument",
                               Vst::kDistributable,
                               Vst::PlugType::kFxInstrument,
                               QString("%1.%2.%3").arg(APP_VERSION_MAJOR).arg(APP_VERSION_MINOR).arg(APP_VERSION_BUILD).toLatin1(),
                               kVstVersionString,
                               VstBoardProcessor::createInstance)

        DEF_CLASS2 (INLINE_UID_FROM_FUID(VstBoardControllerUID),
                                PClassInfo::kManyInstances,
                                kVstComponentControllerClass,
                                "VstBoard",
                                0,
                                "",
                                QString("%1.%2.%3").arg(APP_VERSION_MAJOR).arg(APP_VERSION_MINOR).arg(APP_VERSION_BUILD).toLatin1(),
                                kVstVersionString,
                                VstBoardController::createInstance)

//        DEF_CLASS2 (INLINE_UID_FROM_FUID(VstBoardTestFactory::cid),
//                                PClassInfo::kManyInstances,
//                                kTestClass,
//                                "VstBoard Test",
//                                0,
//                                "",
//                                "",
//                                "",
//                                VstBoardTestFactory::createInstance)

END_FACTORY



//the 2.4 factory creates plugin & instrument classes
IPluginFactory* PLUGIN_API GetPluginFactoryVst24() {
	if (!gPluginFactory)
	{
		static PFactoryInfo factoryInfo("CtrlBrk", "http://vstboard.blogspot.com", "mailto:ctrlbrk76@gmail.com", Vst::kDefaultFactoryFlags);
		gPluginFactory = new CPluginFactory(factoryInfo);

		DEF_CLASS2(INLINE_UID_FROM_FUID(VstBoardProcessorUID),
			PClassInfo::kManyInstances,
			kVstAudioEffectClass,
			"VstBoard Effect",
			Vst::kDistributable,
			Vst::PlugType::kFx,
			QString("%1.%2.%3").arg(APP_VERSION_MAJOR).arg(APP_VERSION_MINOR).arg(APP_VERSION_BUILD).toLatin1(),
			kVstVersionString,
			VstBoardProcessor::createInstance)
		
		DEF_CLASS2 (INLINE_UID_FROM_FUID(VstBoardInstProcessorUID),
			PClassInfo::kManyInstances,
			kVstAudioEffectClass,
			"VstBoard Instrument",
			Vst::kDistributable,
			Vst::PlugType::kInstrument,
			QString("%1.%2.%3").arg(APP_VERSION_MAJOR).arg(APP_VERSION_MINOR).arg(APP_VERSION_BUILD).toLatin1(),
			kVstVersionString,
			VstBoardProcessor::createInstance)

		DEF_CLASS2(INLINE_UID_FROM_FUID(VstBoardControllerUID),
			PClassInfo::kManyInstances,
			kVstComponentControllerClass,
			"VstBoard",
			0,
			"",
			QString("%1.%2.%3").arg(APP_VERSION_MAJOR).arg(APP_VERSION_MINOR).arg(APP_VERSION_BUILD).toLatin1(),
			kVstVersionString,
			VstBoardController::createInstance)
	}
	else gPluginFactory->addRef(); 
	return gPluginFactory; 
}

bool InitModule () { return true; }
bool DeinitModule () { return true; }

