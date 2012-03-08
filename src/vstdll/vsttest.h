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

#ifndef VSTTEST_H
#define VSTTEST_H

#include "base/source/fobject.h"
#include "pluginterfaces/test/itest.h"

namespace Steinberg {
    namespace Vst {
        class IPlugProvider;
    }
}

using namespace Steinberg;

//-----------------------------------------------------------------------------
class VstBoardTest : public ITest, public FObject
{
public:
        VstBoardTest (Vst::IPlugProvider* plugProvider);

        //--ITest
        virtual bool PLUGIN_API setup ();
        virtual bool PLUGIN_API run (ITestResult* testResult);
        virtual bool PLUGIN_API teardown ();
        virtual const tchar* PLUGIN_API getDescription ();

        OBJ_METHODS(VstBoardTest, FObject)
        DEF_INTERFACES_1(ITest, FObject)
        REFCOUNT_METHODS(FObject)
protected:
        Vst::IPlugProvider* plugProvider;
};

//-----------------------------------------------------------------------------
class VstBoardTestFactory : public ITestFactory, public FObject
{
public:
        //--ITestFactory
        tresult PLUGIN_API createTests (FUnknown* context, ITestSuite* parentSuite);

        static FUnknown* createInstance (void*) { return (ITestFactory*)new VstBoardTestFactory (); }

        static FUID cid;

        OBJ_METHODS(VstBoardTestFactory, FObject)
        DEF_INTERFACES_1(ITestFactory, FObject)
        REFCOUNT_METHODS(FObject)
};

#endif // VSTTEST_H
