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
/*
 * Based on :
 * ClapSawDemo is Free and Open Source released under the MIT license
 *
 * Copright (c) 2021, Paul Walker
 */

#include "clapmainhost.h"
#include "mainwindow.h"

#include <clap/helpers/plugin.hh>
#include <clap/helpers/plugin.hxx>
#include "connectables/objectfactoryclap.h"

#include "gui.h"

ClapMainHost::ClapMainHost(const clap_host *host) :
    MainHost(0,0),
    clap::helpers::Plugin<clap::helpers::MisbehaviourHandler::Terminate, clap::helpers::CheckingLevel::Maximal>(&desc, host),
    guiWindow(0)

{
    objFactory = new Connectables::ObjectFactoryClap(this);

#if defined(_M_X64) || defined(__amd64__)
    settings = new Settings("x64/plugin/",this);
#else
    settings = new Settings("x86/plugin/",this);
#endif

    paramToValue[pmTest1] = &paramTest1;
    paramToValue[pmTest2] = &paramTest2;

    Init();
    Open();
    LoadDefaultFiles();

}

ClapMainHost::~ClapMainHost()
{

}


void ClapMainHost::SendMsg(const MsgObject &msg)
{
    emit SendMsgSignal(msg);
}

void ClapMainHost::ReceiveMsgSignal(const MsgObject &msg)
{
    ReceiveMsg(msg);
}

const char *features[] = {CLAP_PLUGIN_FEATURE_AUDIO_EFFECT, CLAP_PLUGIN_FEATURE_MULTI_EFFECTS, nullptr};
clap_plugin_descriptor ClapMainHost::desc = {CLAP_VERSION,
                                             "ctrlbrk.clapBoard",
                                             "Clap Host Thing to be named",
                                             "CtrlBrk",
                                             "https://",
                                             "",
                                             "",
                                             "0.0.1",
                                             "Test one two, test.",
                                             features};

bool ClapMainHost::activate(double sampleRate, uint32_t minFrameCount, uint32_t maxFrameCount) noexcept
{
    SetSampleRate(sampleRate);
    return true;
}

bool ClapMainHost::paramsInfo(uint32_t paramIndex, clap_param_info *info) const noexcept
{
    if (paramIndex >= nParams)
        return false;

    /*
     * Our job is to populate the clap_param_info. We set each of our parameters as AUTOMATABLE
     * and then begin setting per-parameter features.
     */
    info->flags = CLAP_PARAM_IS_AUTOMATABLE;

    /*
     * These constants activate polyphonic modulatability on a parameter. Not all the params here
     * support that
     */
    auto mod = CLAP_PARAM_IS_MODULATABLE ;
    //| CLAP_PARAM_IS_MODULATABLE_PER_NOTE_ID | CLAP_PARAM_IS_MODULATABLE_PER_KEY;

    switch (paramIndex)
    {
    case 0:
        info->id = pmTest1;
        strncpy(info->name, "test1", CLAP_NAME_SIZE);
        strncpy(info->module, "main", CLAP_NAME_SIZE);
        info->min_value = 0;
        info->max_value = 1;
        info->default_value = 0;
        // info->flags |= CLAP_PARAM_IS_STEPPED;
        break;
    case 1:
        info->id = pmTest2;
        strncpy(info->name, "test2", CLAP_NAME_SIZE);
        strncpy(info->module, "main", CLAP_NAME_SIZE);
        info->min_value = 0;
        info->max_value = 1;
        info->default_value = 0;
        info->flags |= mod;
        break;
    }
    return true;
}

bool ClapMainHost::audioPortsInfo(uint32_t index, bool isInput, clap_audio_port_info *info) const noexcept
{
    if(isInput) {
        if(index > 1) return false;
    } else {
        if(index > 1) return false;
    }

    info->id = index;
    info->in_place_pair = CLAP_INVALID_ID;

    if(index == 0) {
        strncpy(info->name, "main", sizeof(info->name));
        info->flags = CLAP_AUDIO_PORT_IS_MAIN;
    } else {
        strncpy(info->name, "aux", sizeof(info->name));
    }

    info->channel_count = 2;
    info->port_type = CLAP_PORT_STEREO;
    return true;
}

bool ClapMainHost::notePortsInfo(uint32_t index, bool isInput, clap_note_port_info *info) const noexcept
{
    if (isInput)
    {
        if(index > 1) return false;
        info->id = index+2; //+2 : if the id list is shared with the audio ports ?
        info->supported_dialects = CLAP_NOTE_DIALECT_MIDI | CLAP_NOTE_DIALECT_CLAP;
        info->preferred_dialect = CLAP_NOTE_DIALECT_CLAP;
        strncpy(info->name, "NoteInput", CLAP_NAME_SIZE);
        return true;
    } else {
        if(index > 1) return false;
        info->id = index+2; //+2 : if the id list is shared with the audio ports ?
        info->supported_dialects = CLAP_NOTE_DIALECT_MIDI | CLAP_NOTE_DIALECT_CLAP;
        info->preferred_dialect = CLAP_NOTE_DIALECT_CLAP;
        strncpy(info->name, "NoteOutput", CLAP_NAME_SIZE);
        return true;
    }
    return false;
}

bool ClapMainHost::stateSave(const clap_ostream *stream) noexcept
{
    return true;
}

bool ClapMainHost::stateLoad(const clap_istream *stream) noexcept
{
    return true;
}

clap_process_status ClapMainHost::process(const clap_process *process) noexcept
{
    SetBufferSize( process->frames_count );

    if (process->audio_outputs_count <= 0)
        return CLAP_PROCESS_SLEEP;

    //get audio from input
    if (process->frames_count > 0) {

        uint32_t bufCpt=0;

        Q_FOREACH(Connectables::ClapAudioDeviceIn* dev, lstAudioIn) {

            if(bufCpt < process->audio_inputs_count) {
                if(doublePrecision) {
                    dev->SetBuffersD(process->audio_inputs[bufCpt].data64, process->frames_count);
                } else {
                    dev->SetBuffers(process->audio_inputs[bufCpt].data32, process->frames_count);
                }
                bufCpt++;
            }

        }
    }

    Render();


    //put audio to outputs
    if (process->frames_count > 0) {

        uint32_t bufCpt=0;

        Q_FOREACH(Connectables::ClapAudioDeviceOut* dev, lstAudioOut) {

            if(bufCpt < process->audio_outputs_count) {
                if(doublePrecision) {
                    dev->GetBuffersD(process->audio_outputs[bufCpt].data64, process->frames_count);
                } else {
                    dev->GetBuffers(process->audio_outputs[bufCpt].data32, process->frames_count);
                }
            }
            bufCpt++;
        }
    }

    return CLAP_PROCESS_CONTINUE;
}

void ClapMainHost::paramsFlush(const clap_input_events *in, const clap_output_events *out) noexcept
{

}

bool ClapMainHost::startProcessing() noexcept
{
    return true;
}

void ClapMainHost::stopProcessing() noexcept
{

}

bool ClapMainHost::guiIsApiSupported(const char *api, bool isFloating) noexcept
{
    // if (isFloating)
        // return false;

#if IS_MAC
    // if (strcmp(api, CLAP_WINDOW_API_COCOA) == 0)
        // return true;
#endif

#if WIN64
    if (strcmp(api, CLAP_WINDOW_API_WIN32) == 0)
        return true;
#endif

#if IS_LINUX
    // if (_host.canUseTimerSupport() && _host.canUsePosixFdSupport() &&
        // strcmp(api, CLAP_WINDOW_API_X11) == 0)
        // return true;
#endif

    return false;
}

bool ClapMainHost::guiCreate(const char *api, bool isFloating) noexcept
{
    guiWindow = new Gui(this);

    return true;
}

void ClapMainHost::guiDestroy() noexcept
{

}

bool ClapMainHost::guiSetParent(const clap_window *window) noexcept
{
    return guiWindow->attached((void*)window->ptr);
}

bool ClapMainHost::guiSetScale(double scale) noexcept
{
    return true;
}

bool ClapMainHost::guiAdjustSize(uint32_t *width, uint32_t *height) noexcept
{
    return true;
}
bool ClapMainHost::guiSetSize(uint32_t width, uint32_t height) noexcept
{
    return true;
}
bool ClapMainHost::guiGetSize(uint32_t *width, uint32_t *height) noexcept
{
    return true;
}


bool ClapMainHost::addAudioIn(Connectables::ClapAudioDeviceIn *dev)
{
    QMutexLocker l(&mutexDevices);

    if(lstAudioIn.contains(dev))
        return false;

    if(lstAudioIn.count() == NB_MAIN_BUSES_IN)
        return false;

    dev->setObjectName( QString("Audio in %1").arg( lstAudioIn.count()+1 ) );
    lstAudioIn << dev;
    return true;
}

bool ClapMainHost::addAudioOut(Connectables::ClapAudioDeviceOut *dev)
{
    QMutexLocker l(&mutexDevices);

    if(lstAudioOut.contains(dev))
        return false;

    if(lstAudioOut.count() == NB_MAIN_BUSES_OUT)
        return false;

    dev->setObjectName( QString("Audio out %1").arg( lstAudioOut.count()+1 ) );
    lstAudioOut << dev;
    return true;
}

bool ClapMainHost::removeAudioIn(Connectables::ClapAudioDeviceIn *dev)
{
    QMutexLocker l(&mutexDevices);
    lstAudioIn.removeAll(dev);
    return true;
}

bool ClapMainHost::removeAudioOut(Connectables::ClapAudioDeviceOut *dev)
{
    QMutexLocker l(&mutexDevices);
    lstAudioOut.removeAll(dev);
    return true;
}
