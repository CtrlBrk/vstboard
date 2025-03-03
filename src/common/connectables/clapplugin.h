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

#ifndef CLAPPLUGIN_H
#define CLAPPLUGIN_H

#include <clap/clap.h>
#include <clap/helpers/event-list.hh>
// #include <clap/helpers/reducing-param-queue.hh>
#include <clap/helpers/plugin-proxy.hh>

#include "object.h"
// #include "claphost.h"
#include "clappluginparam.h"


constexpr auto PluginHost_MH = clap::helpers::MisbehaviourHandler::Terminate;
constexpr auto PluginHost_CL = clap::helpers::CheckingLevel::Maximal;

using BaseHost = clap::helpers::Host<PluginHost_MH, PluginHost_CL>;
// extern template class clap::helpers::Host<PluginHost_MH, PluginHost_CL>;


using PluginProxy = clap::helpers::PluginProxy<PluginHost_MH, PluginHost_CL>;
extern template class clap::helpers::PluginProxy<PluginHost_MH, PluginHost_CL>;



namespace Connectables {

class ClapPlugin : public Object, public BaseHost
{
    Q_OBJECT;

public:
    ClapPlugin(MainHost *myHost,int index, const ObjectInfo & info);
    ~ClapPlugin();
    bool Open() override;
    bool Close();
    void Render() override;
    void SetSleep(bool sleeping) override;
    void MidiMsgFromInput(long msg) override;
    Pin* CreatePin(const ConnectionInfo &info) override;
    void processNoteOn(int sampleOffset, int channel, int key, int velocity);
    void processNoteOff(int sampleOffset, int channel, int key, int velocity);

    void initThreadPool();
    void terminateThreadPool();
    void threadPoolEntry();

    static void checkForMainThread();
    static void checkForAudioThread();
protected:
    // clap_host
    void requestRestart() noexcept override;
    void requestProcess() noexcept override;
    void requestCallback() noexcept override;

    // clap_host_thread_check
    bool threadCheckIsMainThread() const noexcept override;
    bool threadCheckIsAudioThread() const noexcept override;
private:
    clap::helpers::EventList _evIn;
    clap::helpers::EventList _evOut;
    clap_audio_buffer _audioIn = {};
    clap_audio_buffer _audioOut = {};
    clap_process _process;

    /* thread pool */
    std::vector<std::unique_ptr<QThread>> _threadPool;
    std::atomic<bool> _threadPoolStop = {false};
    std::atomic<int> _threadPoolTaskIndex = {0};
    QSemaphore _threadPoolSemaphoreProd;
    QSemaphore _threadPoolSemaphoreDone;

    qint32 bufferSize;
    float sampleRate;

    void paramsRescan(uint32_t flags) noexcept override;
    double getParamValue(const clap_param_info &info);
    void checkValidParamValue(const ClapPluginParam &param, double value);

    static bool clapParamsRescanMayValueChange(uint32_t flags) {
        return flags & (CLAP_PARAM_RESCAN_ALL | CLAP_PARAM_RESCAN_VALUES);
    }
    static bool clapParamsRescanMayInfoChange(uint32_t flags) {
        return flags & (CLAP_PARAM_RESCAN_ALL | CLAP_PARAM_RESCAN_INFO);
    }

    QLibrary _library;
    std::unordered_map<clap_id, std::unique_ptr<ClapPluginParam>> _params;
    const clap_plugin_entry *_pluginEntry = nullptr;
    const clap_plugin_factory *_pluginFactory = nullptr;
    std::unique_ptr<PluginProxy> _plugin;

    QList<QVariant>listIsLearning;

    bool _scheduleProcess = false;
    bool _scheduleDeactivate = false;

signals:
    void paramsChanged();
public slots:
    void SetBufferSize(qint32 size) override;
    void SetSampleRate(float rate=44100.0) override;
};

}
#endif // CLAPPLUGIN_H
