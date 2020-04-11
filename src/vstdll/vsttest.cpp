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

//#include "vsttest.h"
//#include "vstboardcontroller.h"
//#include "vstboardprocessor.h"
//#include "base/source/fstring.h"
//#include "public.sdk/source/vst/testsuite/vsttestsuite.h"

//namespace Steinberg {

//    DEF_CLASS_IID(ITest)
//    DEF_CLASS_IID(ITestSuite)
//    DEF_CLASS_IID(ITestFactory)

////    namespace Vst {
////        DEF_CLASS_IID(IPlugProvider)
////    }
//}

//FUID VstBoardTestFactory::cid (0x03574CA9, 0x0ADB4c80, 0xB2B7D80B, 0x417D227D);

//using namespace Steinberg;

////-----------------------------------------------------------------------------
//tresult PLUGIN_API VstBoardTestFactory::createTests (FUnknown* context, ITestSuite* parentSuite)
//{
//        FUnknownPtr<Vst::IPlugProvider> plugProvider (context);
//        if (plugProvider)
//        {
//                parentSuite->addTest ("VstBoardTest", new VstBoardTest (plugProvider));
//        }
//        return kResultTrue;
//}

////-----------------------------------------------------------------------------
////-----------------------------------------------------------------------------
////-----------------------------------------------------------------------------
//VstBoardTest::VstBoardTest (Vst::IPlugProvider* plugProvider)
//: plugProvider (plugProvider)
//{
//}

////-----------------------------------------------------------------------------
//bool PLUGIN_API VstBoardTest::setup ()
//{
//        return true;
//}

////-----------------------------------------------------------------------------
//bool PLUGIN_API VstBoardTest::run (ITestResult* testResult)
//{
////        VstBoardController* controller = FCast<VstBoardController> (plugProvider->getController ());
////        VstBoardProcessor* processor = FCast<VstBoardProcessor> (plugProvider->getComponent ());
////        if (controller)
////                testResult->addMessage (String ("Correct IEditController"));
////        else
////                testResult->addErrorMessage (String ("Unknown IEditController"));
////        if (processor)
////                testResult->addMessage (String ("Correct IComponent"));
////        else
////                testResult->addErrorMessage (String ("Unknown IComponent"));
////        return (controller && processor);
//    return 0;
//}

////-----------------------------------------------------------------------------
//bool PLUGIN_API VstBoardTest::teardown ()
//{
//        return true;
//}

////-----------------------------------------------------------------------------
//const tchar* PLUGIN_API VstBoardTest::getDescription ()
//{
//        static String description ("VstBoard Test");
//        return description.text16 ();
//}
