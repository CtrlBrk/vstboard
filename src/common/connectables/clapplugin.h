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
#include <clap/helpers/reducing-param-queue.hh>

constexpr auto PluginHost_MH = clap::helpers::MisbehaviourHandler::Terminate;
constexpr auto PluginHost_CL = clap::helpers::CheckingLevel::Maximal;

using BaseHost = clap::helpers::Host<PluginHost_MH, PluginHost_CL>;
// extern template class clap::helpers::Host<PluginHost_MH, PluginHost_CL>;


using PluginProxy = clap::helpers::PluginProxy<PluginHost_MH, PluginHost_CL>;
extern template class clap::helpers::PluginProxy<PluginHost_MH, PluginHost_CL>;

namespace View {
class ClapPluginWindow;
}


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
    void processCC(int sampleOffset, int channel, int cc, int value);
    QString GetParameterName(ConnectionInfo pinInfo) override;
    void OnParameterChanged(ConnectionInfo pinInfo, float value) override;

    void initThreadPool();
    void terminateThreadPool();
    void threadPoolEntry();

    void setParentWindow(WId parentWindow);
    void setPluginWindowVisibility(bool isVisible);

    static void checkForMainThread();
    static void checkForAudioThread();

    void idle();
    void ReceiveMsg(const MsgObject &msg) override;

    void setParamValueByHost(ClapPluginParam &param, double value);
    void setParamModulationByHost(ClapPluginParam &param, double value);

    View::ClapPluginWindow *editorWnd;

    bool canActivate() const;
    void activate(int32_t sample_rate, int32_t blockSize);
    void deactivate();

    void fromJson(QJsonObject &json) override;
    void toJson(QJsonObject &json)  override;

protected:
    // clap_host
    void requestRestart() noexcept override;
    void requestProcess() noexcept override;
    void requestCallback() noexcept override;

    // clap_host_gui
    bool implementsGui() const noexcept override { return true; }
    void guiResizeHintsChanged() noexcept override;
    bool guiRequestResize(uint32_t width, uint32_t height) noexcept override;
    bool guiRequestShow() noexcept override;
    bool guiRequestHide() noexcept override;
    void guiClosed(bool wasDestroyed) noexcept override;

    // clap_host_params
    bool implementsParams() const noexcept override { return true; }
    void paramsRescan(clap_param_rescan_flags flags) noexcept override;
    void paramsClear(clap_id paramId, clap_param_clear_flags flags) noexcept override;
    void paramsRequestFlush() noexcept override;

    // clap_host_state
    bool implementsState() const noexcept override { return true; }
    void stateMarkDirty() noexcept override;

    // clap_host_thread_check
    bool threadCheckIsMainThread() const noexcept override;
    bool threadCheckIsAudioThread() const noexcept override;

    // clap_host_thread_pool
    bool implementsThreadPool() const noexcept override { return true; }
    bool threadPoolRequestExec(uint32_t numTasks) noexcept override;

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

    void paramFlushOnMainThread();
    double getParamValue(const clap_param_info &info);
    void checkValidParamValue(const ClapPluginParam &param, double value);
    void handlePluginOutputEvents();
    void ParamChangedFromPlugin(int pinNum,float val);
    void CreateEditorWindow();
    void generatePluginInputEvents();


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

    static const char *getCurrentClapGuiApi();

    enum PluginState {
        // The plugin is inactive, only the main thread uses it
        Inactive,

        // Activation failed
        InactiveWithError,

        // The plugin is active and sleeping, the audio engine can call set_processing()
        ActiveAndSleeping,

        // The plugin is processing
        ActiveAndProcessing,

        // The plugin did process but is in error
        ActiveWithError,

        // The plugin is not used anymore by the audio engine and can be deactivated on the main
        // thread
        ActiveAndReadyToDeactivate,
    };

    bool isPluginActive() const;
    bool isPluginProcessing() const;
    bool isPluginSleeping() const;
    void setPluginState(PluginState state);
    PluginState _state = Inactive;

    bool _scheduleProcess = false;
    bool _scheduleDeactivate = false;

    bool _scheduleParamFlush = false;

    const char *_guiApi = nullptr;
    bool _isGuiCreated = false;
    bool _isGuiVisible = false;
    bool _isGuiFloating = false;

    bool _stateIsDirty = false;

    std::unordered_map<clap_id, bool> _isAdjustingParameter;

    struct EngineToAppParamQueueValue {
        void update(const EngineToAppParamQueueValue& v) noexcept {
            if (v.has_value) {
                has_value = true;
                value = v.value;
            }

            if (v.has_gesture) {
                has_gesture = true;
                is_begin = v.is_begin;
            }
        }

        bool has_value = false;
        bool has_gesture = false;
        bool is_begin = false;
        double value = 0;
    };

    struct AppToEngineParamQueueValue {
        void *cookie;
        double value;
    };

    clap::helpers::ReducingParamQueue<clap_id, AppToEngineParamQueueValue> _appToEngineValueQueue;
    clap::helpers::ReducingParamQueue<clap_id, AppToEngineParamQueueValue> _appToEngineModQueue;
    clap::helpers::ReducingParamQueue<clap_id, EngineToAppParamQueueValue> _engineToAppValueQueue;

    QTimer _idleTimer;

signals:
    void WindowSizeChange(int newWidth, int newHeight);
    void paramsChanged();
    void paramAdjusted(clap_id paramId);

public slots:
    void SetBufferSize(qint32 size) override;
    void SetSampleRate(float rate=44100.0) override;
    void EditorDestroyed();
    void OnEditorClosed();
    void OnShowEditor() override;
    void OnHideEditor() override;
    void UserAddPin(const ConnectionInfo &info) override;

    void LoadProgram(int progId) override;
    void SaveProgram() override;

    // void paramValueChanged();
};

}
#endif // CLAPPLUGIN_H
