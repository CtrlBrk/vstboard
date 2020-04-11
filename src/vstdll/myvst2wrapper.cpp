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

#include "myvst2wrapper.h"
#include "vstboardprocessor.h"

MyVst2Wrapper::MyVst2Wrapper(BaseWrapper::SVST3Config& config, audioMasterCallback audioMaster, VstInt32 vst2ID) :
    Vst2Wrapper(config,audioMaster,vst2ID)
{

}

VstInt32 MyVst2Wrapper::processEvents(VstEvents* events)
{
    VstBoardProcessor* proc = 0; //static_cast<VstBoardProcessor*>(mProcessor);
    if(!proc)
        return 0;

    return proc->processEvents(events);
}

AudioEffect* MyVst2Wrapper::crt (IPluginFactory* factory, const TUID vst3ComponentID, VstInt32 vst2ID, audioMasterCallback audioMaster)
{
    if (!factory)
        return 0;

    SVST3Config conf;
    conf.factory = factory;
    conf.processor = nullptr;
    conf.controller = nullptr;
//    conf.vst3ComponentID = vst3ComponentID;

//    Vst::IAudioProcessor* processor = 0;
//    FReleaser factoryReleaser (factory);

//    factory->createInstance (vst3ComponentID, Vst::IAudioProcessor::iid, (void**)&processor);
//    if (processor)
//    {
//        Vst::IEditController* controller = 0;
//        if (processor->queryInterface (Vst::IEditController::iid, (void**)&controller) != kResultTrue)
//        {
//            FUnknownPtr<Vst::IComponent> component (processor);
//            if (component)
//            {
//                FUID editorCID;
//                if (component->getControllerClassId (editorCID) == kResultTrue)
//                {
//                    factory->createInstance (editorCID, Vst::IEditController::iid, (void**)&controller);
//                }
//            }
//        }

//        MyVst2Wrapper* wrapper = new MyVst2Wrapper (conf, audioMaster, vst2ID);
//        if (wrapper->init () == false)
//        {
//            delete wrapper;
//            return 0;
//        }

//        FUnknownPtr<IPluginFactory2> factory2 (factory);
//        if (factory2)
//        {
//            PFactoryInfo factoryInfo;
//            if (factory2->getFactoryInfo (&factoryInfo) == kResultTrue)
//                wrapper->setVendorName (factoryInfo.vendor);

//            for (int32 i = 0; i < factory2->countClasses (); i++)
//            {
//                Steinberg::PClassInfo2 classInfo2;
//                if (factory2->getClassInfo2 (i, &classInfo2) == Steinberg::kResultTrue)
//                {
//                    if (memcmp (classInfo2.cid, vst3ComponentID, sizeof (TUID)) == 0)
//                    {
//                        wrapper->setSubCategories (classInfo2.subCategories);
//                        wrapper->setEffectName (classInfo2.name);

//                        if (classInfo2.vendor[0] != 0)
//                            wrapper->setVendorName (classInfo2.vendor);

//                        break;
//                    }
//                }
//            }
//        }

//        return wrapper;
//    }

    return 0;
}

void MyVst2Wrapper::processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames)
{
    if (mProcessData.symbolicSampleSize != Vst::kSample32)
        return;

    setProcessingBuffers<float> (inputs, outputs);
    doProcess (sampleFrames);
}

//-------------------------------------------------------------------------------------------------------
void MyVst2Wrapper::processDoubleReplacing (double** inputs, double** outputs, VstInt32 sampleFrames)
{
    if (mProcessData.symbolicSampleSize != Vst::kSample64)
        return;

    setProcessingBuffers<double> (inputs, outputs);
    doProcess (sampleFrames);
}

inline void MyVst2Wrapper::doProcess (VstInt32 sampleFrames)
{
    if (mProcessor)
    {
        mProcessData.numSamples = sampleFrames;

//        if (processing == false)
//            startProcess ();

        mProcessData.inputEvents = mInputEvents;
        mProcessData.outputEvents = mOutputEvents;

        setupProcessTimeInfo ();
        setEventPPQPositions ();

        mInputTransfer.transferChangesTo (mInputChanges);

        mProcessData.inputParameterChanges = &mInputChanges;
        mProcessData.outputParameterChanges = &mOutputChanges;
        mOutputChanges.clearQueue ();

        // vst 3 process call
        mProcessor->process (mProcessData);

        mOutputTransfer.transferChangesFrom (mOutputChanges);
        processOutputEvents ();

        // clear input parameters and events
        mInputChanges.clearQueue ();
        if (mInputEvents)
            mInputEvents->clear ();
    }
}

inline void MyVst2Wrapper::processOutputEvents ()
{
    VstBoardProcessor* proc = 0;//static_cast<VstBoardProcessor*>(mProcessor);
    if(!proc)
        return;

    if(proc->processOutputEvents())
        sendVstEventsToHost(proc->getEvents());
}

void MyVst2Wrapper::onTimer (Timer* timer)
{
    if (mController)
    {
        Vst::ParamID id;
        Vst::ParamValue value;
        int32 sampleOffset;

        while (mOutputTransfer.getNextChange (id, value, sampleOffset))
        {
            setParameterAutomated(id,value);
            mController->setParamNormalized (id, value);
        }
        while (mGuiTransfer.getNextChange (id, value, sampleOffset))
        {
            mController->setParamNormalized (id, value);
        }
    }
}
