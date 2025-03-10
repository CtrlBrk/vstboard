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

#ifndef CLAPMAINHOST_H
#define CLAPMAINHOST_H

#include <clap/helpers/plugin.hh>
#include <QObject>
#include <mainhost.h>

#include "connectables/clapaudiodevicein.h"
#include "connectables/clapaudiodeviceout.h"
#include "connectables/clapmididevice.h"
// #include "connectables/clapautomation.h"

#define NB_MAIN_BUSES_IN 4
#define NB_MAIN_BUSES_OUT 4
// #define NB_AUX_BUSES_IN 1
// #define NB_AUX_BUSES_OUT 1
#define NB_MIDI_BUSES_IN 1
#define NB_MIDI_BUSES_OUT 1

class Gui;

class ClapMainHost :
    public MainHost,
    public clap::helpers::Plugin <clap::helpers::MisbehaviourHandler::Terminate, clap::helpers::CheckingLevel::Maximal>
{
    Q_OBJECT
public:
    ClapMainHost(const clap_host *host, clap_plugin_descriptor &descc);
    ~ClapMainHost();
    void SendMsg(const MsgObject &msg) override;

    static clap_plugin_descriptor desc;
    static clap_plugin_descriptor descInst;

    bool activate(double sampleRate, uint32_t minFrameCount, uint32_t maxFrameCount) noexcept override;


    //params
    enum paramIds : uint32_t
    {
        pmTest1 = 1111,
        pmTest2 = 2222
    };

    static constexpr int nParams = 2;

    bool implementsParams() const noexcept override { return true; }
    bool isValidParamId(clap_id paramId) const noexcept override
    {
        return paramToValue.find(paramId) != paramToValue.end();
    }
    uint32_t paramsCount() const noexcept override { return nParams; }
    bool paramsInfo(uint32_t paramIndex, clap_param_info *info) const noexcept override;
    bool paramsValue(clap_id paramId, double *value) noexcept override
    {
        *value = *paramToValue[paramId];
        return true;
    }

    //audio in out
    bool implementsAudioPorts() const noexcept override { return true; }
    uint32_t audioPortsCount(bool isInput) const noexcept override { return isInput ? 2 : 2; }
    bool audioPortsInfo(uint32_t index, bool isInput, clap_audio_port_info *info) const noexcept override;

    //midi in out
    bool implementsNotePorts() const noexcept override { return true; }
    uint32_t notePortsCount(bool isInput) const noexcept override { return isInput ? 2 : 2; }
    bool notePortsInfo(uint32_t index, bool isInput, clap_note_port_info *info) const noexcept override;

    //state chunks
    bool implementsState() const noexcept override { return true; }
    bool stateSave(const clap_ostream *) noexcept override;
    bool stateLoad(const clap_istream *) noexcept override;

    //proc
    clap_process_status process(const clap_process *process) noexcept override;
    void paramsFlush(const clap_input_events *in, const clap_output_events *out) noexcept override;

    //start stop
    bool startProcessing() noexcept override;
    void stopProcessing() noexcept override;


    bool addAudioIn(Connectables::ClapAudioDeviceIn *dev);
    bool addAudioOut(Connectables::ClapAudioDeviceOut *dev);
    bool removeAudioIn(Connectables::ClapAudioDeviceIn *dev);
    bool removeAudioOut(Connectables::ClapAudioDeviceOut *dev);

    bool addMidiIn(Connectables::ClapMidiDevice *dev);
    bool addMidiOut(Connectables::ClapMidiDevice *dev);
    bool removeMidiIn(Connectables::ClapMidiDevice *dev);
    bool removeMidiOut(Connectables::ClapMidiDevice *dev);

protected:
    bool implementsGui() const noexcept override { return true; }
    bool guiIsApiSupported(const char *api, bool isFloating) noexcept override;

    bool guiCreate(const char *api, bool isFloating) noexcept override;
    void guiDestroy() noexcept override;
    bool guiSetParent(const clap_window *window) noexcept override;

    bool guiSetScale(double scale) noexcept override;
    bool guiCanResize() const noexcept override { return true; }
    bool guiAdjustSize(uint32_t *width, uint32_t *height) noexcept override;
    bool guiSetSize(uint32_t width, uint32_t height) noexcept override;
    bool guiGetSize(uint32_t *width, uint32_t *height) noexcept override;

    QList<Connectables::ClapAudioDeviceIn*>lstAudioIn;
    QList<Connectables::ClapAudioDeviceOut*>lstAudioOut;
    QList<Connectables::ClapMidiDevice*>lstMidiIn;
    QList<Connectables::ClapMidiDevice*>lstMidiOut;
    // QList<Connectables::ClapAutomation*>lstVstAutomation;

    Gui* guiWindow;
    QMutex mutexDevices;

private:
    double paramTest1{1}, paramTest2{1};
    std::unordered_map<clap_id, double *> paramToValue;


signals:
    void SendMsgSignal(const MsgObject &msg);

public slots:
    void ReceiveMsgSignal(const MsgObject &msg);
};

#endif // CLAPMAINHOST_H
