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

#include "myvst2wrapper.h"
#include "vstboardprocessor.h"

MyVst2Wrapper::MyVst2Wrapper(Vst::IAudioProcessor *processor, Vst::IEditController *controller, audioMasterCallback audioMaster, const TUID vst3ComponentID, VstInt32 vst2ID, IPluginFactory *factory) :
    Vst2Wrapper(processor,controller,audioMaster,vst3ComponentID,vst2ID,factory)
{

}

VstInt32 MyVst2Wrapper::processEvents(VstEvents* events)
{
    VstBoardProcessor* proc = static_cast<VstBoardProcessor*>(processor);
    if(!proc)
        return 0;

    return proc->processEvents(events);
}

AudioEffect* MyVst2Wrapper::crt (IPluginFactory* factory, const TUID vst3ComponentID, VstInt32 vst2ID, audioMasterCallback audioMaster)
{
    if (!factory)
        return 0;

    Vst::IAudioProcessor* processor = 0;
    FReleaser factoryReleaser (factory);

    factory->createInstance (vst3ComponentID, Vst::IAudioProcessor::iid, (void**)&processor);
    if (processor)
    {
        Vst::IEditController* controller = 0;
        if (processor->queryInterface (Vst::IEditController::iid, (void**)&controller) != kResultTrue)
        {
            FUnknownPtr<Vst::IComponent> component (processor);
            if (component)
            {
                FUID editorCID;
                if (component->getControllerClassId (editorCID) == kResultTrue)
                {
                    factory->createInstance (editorCID, Vst::IEditController::iid, (void**)&controller);
                }
            }
        }

        MyVst2Wrapper* wrapper = new MyVst2Wrapper (processor, controller, audioMaster, vst3ComponentID, vst2ID, factory);
        if (wrapper->init () == false)
        {
            delete wrapper;
            return 0;
        }

        FUnknownPtr<IPluginFactory2> factory2 (factory);
        if (factory2)
        {
            PFactoryInfo factoryInfo;
            if (factory2->getFactoryInfo (&factoryInfo) == kResultTrue)
                wrapper->setVendorName (factoryInfo.vendor);

            for (int32 i = 0; i < factory2->countClasses (); i++)
            {
                Steinberg::PClassInfo2 classInfo2;
                if (factory2->getClassInfo2 (i, &classInfo2) == Steinberg::kResultTrue)
                {
                    if (memcmp (classInfo2.cid, vst3ComponentID, sizeof (TUID)) == 0)
                    {
                        wrapper->setSubCategories (classInfo2.subCategories);
                        wrapper->setEffectName (classInfo2.name);

                        if (classInfo2.vendor[0] != 0)
                            wrapper->setVendorName (classInfo2.vendor);

                        break;
                    }
                }
            }
        }

        return wrapper;
    }

    return 0;
}

void MyVst2Wrapper::processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames)
{
    if (processData.symbolicSampleSize != Vst::kSample32)
        return;

    setProcessingBuffers<float> (inputs, outputs);
    doProcess (sampleFrames);
}

//-------------------------------------------------------------------------------------------------------
void MyVst2Wrapper::processDoubleReplacing (double** inputs, double** outputs, VstInt32 sampleFrames)
{
    if (processData.symbolicSampleSize != Vst::kSample64)
        return;

    setProcessingBuffers<double> (inputs, outputs);
    doProcess (sampleFrames);
}

inline void MyVst2Wrapper::doProcess (VstInt32 sampleFrames)
{
    if (processor)
    {
        processData.numSamples = sampleFrames;

        if (processing == false)
            startProcess ();

        processData.inputEvents = inputEvents;
        processData.outputEvents = outputEvents;

        setupProcessTimeInfo ();
        setEventPPQPositions ();

        inputTransfer.transferChangesTo (inputChanges);

        processData.inputParameterChanges = &inputChanges;
        processData.outputParameterChanges = &outputChanges;
        outputChanges.clearQueue ();

        // vst 3 process call
        processor->process (processData);

        outputTransfer.transferChangesFrom (outputChanges);
        processOutputEvents ();

        // clear input parameters and events
        inputChanges.clearQueue ();
        if (inputEvents)
            inputEvents->clear ();
    }
}

inline void MyVst2Wrapper::processOutputEvents ()
{
    VstBoardProcessor* proc = static_cast<VstBoardProcessor*>(processor);
    if(!proc)
        return;

    if(proc->processOutputEvents())
        sendVstEventsToHost(proc->getEvents());
}

void MyVst2Wrapper::onTimer (Timer* timer)
{
    if (controller)
    {
        Vst::ParamID id;
        Vst::ParamValue value;
        int32 sampleOffset;

        while (outputTransfer.getNextChange (id, value, sampleOffset))
        {
            setParameterAutomated(id,value);
            controller->setParamNormalized (id, value);
        }
        while (guiTransfer.getNextChange (id, value, sampleOffset))
        {
            controller->setParamNormalized (id, value);
        }
    }
}
