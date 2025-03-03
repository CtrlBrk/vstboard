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

#include "clapplugin.h"
#include "mainhost.h"

#include <clap/helpers/host.hxx>
#include <clap/helpers/plugin-proxy.hxx>
#include <clap/helpers/reducing-param-queue.hxx>

#include <unordered_set>
// #include "claphost.h"
using namespace Connectables;

template class clap::helpers::PluginProxy<PluginHost_MH, PluginHost_CL>;


enum class ClapThreadType {
    Unknown,
    MainThread,
    AudioThread,
    AudioThreadPool,
};


thread_local ClapThreadType g_thread_type = ClapThreadType::Unknown;


ClapPlugin::ClapPlugin(MainHost *myHost,int index, const ObjectInfo & info) :
    Object(myHost,index, info),
    BaseHost("Clap VstBoard Host",
             "CtrlBrk",
             "0.0.0",
             "https://"
             )
{
    g_thread_type = ClapThreadType::MainThread;
    initThreadPool();
}

ClapPlugin::~ClapPlugin()
{
    Close();

    checkForMainThread();
    terminateThreadPool();
}

void ClapPlugin::initThreadPool() {
    checkForMainThread();

    _threadPoolStop = false;
    _threadPoolTaskIndex = 0;
    auto N = QThread::idealThreadCount();
    _threadPool.resize(N);
    for (int i = 0; i < N; ++i) {
        _threadPool[i].reset(QThread::create(&ClapPlugin::threadPoolEntry, this));
        _threadPool[i]->start(QThread::HighestPriority);
    }
}

void ClapPlugin::terminateThreadPool() {
    checkForMainThread();

    _threadPoolStop = true;
    _threadPoolSemaphoreProd.release(_threadPool.size());
    for (auto &thr : _threadPool)
        if (thr)
            thr->wait();
}

void ClapPlugin::threadPoolEntry() {
    g_thread_type = ClapThreadType::AudioThreadPool;
    while (true) {
        _threadPoolSemaphoreProd.acquire();
        if (_threadPoolStop)
            return;

        int taskIndex = _threadPoolTaskIndex++;
        _plugin->threadPoolExec(taskIndex);
        _threadPoolSemaphoreDone.release();
    }
}


void ClapPlugin::requestCallback() noexcept { /*_scheduleMainThreadCallback = true;*/ }

void ClapPlugin::requestProcess() noexcept { /*_scheduleProcess = true;*/ }

void ClapPlugin::requestRestart() noexcept { /*_scheduleRestart = true;*/ }

bool ClapPlugin::threadCheckIsMainThread() const noexcept {
    return g_thread_type == ClapThreadType::MainThread;
}

bool ClapPlugin::threadCheckIsAudioThread() const noexcept {
    return g_thread_type == ClapThreadType::AudioThread;
}



bool ClapPlugin::Close()
{
    _library.unload();
    return true;
}

bool ClapPlugin::Open()
{
    QMutexLocker lock(&objMutex);

    // checkForMainThread();

    if (_library.isLoaded())
        Close();

    _library.setFileName( objInfo.filename );
    _library.setLoadHints(QLibrary::ResolveAllSymbolsHint | QLibrary::DeepBindHint);
    if (!_library.load()) {
        QString err = _library.errorString();
        qWarning() << "Failed to load plugin '" << objInfo.filename << "': " << err;
        return false;
    }

    _pluginEntry = reinterpret_cast<const struct clap_plugin_entry *>(_library.resolve("clap_entry"));
    if (!_pluginEntry) {
        qWarning() << "Unable to resolve entry point 'clap_entry' in '" << objInfo.filename << "'";
        _library.unload();
        return false;
    }

    _pluginEntry->init(objInfo.filename.toStdString().c_str());

    _pluginFactory = static_cast<const clap_plugin_factory *>(_pluginEntry->get_factory(CLAP_PLUGIN_FACTORY_ID));

    auto count = _pluginFactory->get_plugin_count(_pluginFactory);
    if (objInfo.id > count) {
        qWarning() << "plugin index greater than count :" << count;
        return false;
    }

    auto desc = _pluginFactory->get_plugin_descriptor(_pluginFactory, objInfo.id);
    if (!desc) {
        qWarning() << "no plugin descriptor";
        return false;
    }

    if (!clap_version_is_compatible(desc->clap_version)) {
        qWarning() << "Incompatible clap version: Plugin is: " << desc->clap_version.major << "."
                   << desc->clap_version.minor << "." << desc->clap_version.revision << " Host is "
                   << CLAP_VERSION.major << "." << CLAP_VERSION.minor << "." << CLAP_VERSION.revision;
        return false;
    }

    // const auto plugin = _pluginFactory->create_plugin(_pluginFactory, myHost->clapHost->clapHost() , desc->id);
    const auto plugin = _pluginFactory->create_plugin(_pluginFactory, clapHost() , desc->id);

    if (!plugin) {
        qWarning() << "could not create the plugin with id: " << desc->id;
        return false;
    }

    // _plugin = std::make_unique<PluginProxy>(*plugin, *myHost->clapHost);
    _plugin = std::make_unique<PluginProxy>(*plugin, *this);

    if (!_plugin->init()) {
        qWarning() << "could not init the plugin with id: " << desc->id;
        return false;
    }

    paramsRescan(CLAP_PARAM_RESCAN_ALL);
    // scanParams();
    // scanQuickControls();

    // pluginLoadedChanged(true);


    //create all parameters pins
    int nbParam = _params.size();
    for(int i=0;i<nbParam;i++) {
        listParameterPinIn->AddPin(i);
    }

    if (_plugin->canUseGui()) {
        //editor pin
        listParameterPinIn->AddPin(FixedPinNumber::editorVisible);

        //learning pin
        listIsLearning << "off";
        listIsLearning << "learn";
        listIsLearning << "unlearn";
        listParameterPinIn->AddPin(FixedPinNumber::learningMode);
    }

    bufferSize = myHost->GetBufferSize();
    sampleRate = myHost->GetSampleRate();

    uint32_t nbIn = _plugin->audioPortsCount(true);
    clap_audio_port_info_t info;
    for(int i=0;i< nbIn; i++) {
        _plugin->audioPortsGet(i,true,&info);
        listAudioPinIn->ChangeNumberOfPins(info.channel_count);
    }

    uint32_t nbOut = _plugin->audioPortsCount(false);
    for(int i=0;i< nbOut; i++) {
        _plugin->audioPortsGet(i,false,&info);
        listAudioPinOut->ChangeNumberOfPins(info.channel_count);
    }

    listMidiPinIn->AddPin(0);
    listMidiPinOut->AddPin(0);

    closed=false;
    return true;
}


void ClapPlugin::SetSleep(bool sleeping)
{
    if(closed) {
        return;
    }

    QMutexLocker lock(&objMutex);

    if(sleeping) {
        // _plugin->stopProcessing();
        _plugin->deactivate();
        _scheduleDeactivate=false;
    } else {
        if(!_plugin->activate(sampleRate,bufferSize,bufferSize)) {
            LOG("err activ")
            return;
        }
        _scheduleProcess=true;
    }

    Object::SetSleep(sleeping);
}

void ClapPlugin::SetBufferSize(qint32 size)
{
    if(closed)
        return;

    if(size==bufferSize)
        return;

    bool wasSleeping = GetSleep();
    if(!wasSleeping)
        SetSleep(true);

    Object::SetBufferSize(size);


    bufferSize = size;
    _process.frames_count = bufferSize;

    if(!wasSleeping)
        SetSleep(false);
}


void ClapPlugin::SetSampleRate(float rate)
{
    sampleRate = rate;

    if(closed)
        return;

    bool wasSleeping = GetSleep();
    SetSleep(true);

    SetSleep(wasSleeping);
}


Pin* ClapPlugin::CreatePin(const ConnectionInfo &info)
{

    bool hasEditor = false;// _plugin->canUseGui();

    Pin *newPin = Object::CreatePin(info);
    if(newPin)
        return newPin;

    pinConstructArgs args(info);
    args.parent = this;
    args.bufferSize = myHost->GetBufferSize();
    args.doublePrecision = doublePrecision;


    if(info.type == PinType::Parameter && info.direction == PinDirection::Input) {

        ParameterPin *pin=0;

        switch(info.pinNumber) {
        // case FixedPinNumber::vstProgNumber :
        //     args.name = tr("prog");
        //     args.listValues = &listValues;
        //     args.defaultVariantValue = 0;
        //     return PinFactory::MakePin(args);

        case FixedPinNumber::learningMode :
            if(!hasEditor && !IsInError())
                return 0;
            args.name = tr("Learn");
            args.listValues = &listIsLearning;
            args.defaultVariantValue = "off";
            return PinFactory::MakePin(args);

        // case FixedPinNumber::bypass :
        //     args.name = tr("Bypass");
        //     args.listValues = &listBypass;
        //     args.defaultVariantValue = "On";
        //     return PinFactory::MakePin(args);

        default :
            if(!closed) {
                args.name = QString::fromLatin1( _params[info.pinNumber]->info().name );
                args.value = _params[info.pinNumber]->value();
            }
            args.visible = !hasEditor;
            return PinFactory::MakePin(args);

            return pin;
        }
    }

    return 0;
}

double ClapPlugin::getParamValue(const clap_param_info &info) {
    // checkForMainThread();

    if (!_plugin->canUseParams())
        return 0;

    double value;
    if (_plugin->paramsGetValue(info.id, &value))
        return value;

    std::ostringstream msg;
    msg << "failed to get the param value, id: " << info.id << ", name: " << info.name
        << ", module: " << info.module;
    throw std::logic_error(msg.str());
}

void ClapPlugin::paramsRescan(uint32_t flags) noexcept {
    // checkForMainThread();

    if (!_plugin->canUseParams())
        return;

    // 1. it is forbidden to use CLAP_PARAM_RESCAN_ALL if the plugin is active
    // if (isPluginActive() && (flags & CLAP_PARAM_RESCAN_ALL)) {
    //     throw std::logic_error(
    //         "clap_host_params.recan(CLAP_PARAM_RESCAN_ALL) was called while the plugin is active!");
    //     return;
    // }

    // 2. scan the params.
    auto count = _plugin->paramsCount();
    std::unordered_set<clap_id> paramIds(count * 2);

    for (int32_t i = 0; i < count; ++i) {
        clap_param_info info;
        if (!_plugin->paramsGetInfo(i, &info))
            throw std::logic_error("clap_plugin_params.get_info did return false!");

        if (info.id == CLAP_INVALID_ID) {
            std::ostringstream msg;
            msg << "clap_plugin_params.get_info() reported a parameter with id = CLAP_INVALID_ID"
                << std::endl
                << " 2. name: " << info.name << ", module: " << info.module << std::endl;
            throw std::logic_error(msg.str());
        }

        auto it = _params.find(info.id);

        // check that the parameter is not declared twice
        if (paramIds.count(info.id) > 0) {
            Q_ASSERT(it != _params.end());

            std::ostringstream msg;
            msg << "the parameter with id: " << info.id << " was declared twice." << std::endl
                << " 1. name: " << it->second->info().name << ", module: " << it->second->info().module
                << std::endl
                << " 2. name: " << info.name << ", module: " << info.module << std::endl;
            throw std::logic_error(msg.str());
        }
        paramIds.insert(info.id);

        if (it == _params.end()) {
            if (!(flags & CLAP_PARAM_RESCAN_ALL)) {
                std::ostringstream msg;
                msg << "a new parameter was declared, but the flag CLAP_PARAM_RESCAN_ALL was not "
                       "specified; id: "
                    << info.id << ", name: " << info.name << ", module: " << info.module << std::endl;
                throw std::logic_error(msg.str());
            }

            double value = getParamValue(info);
            auto param = std::make_unique<ClapPluginParam>(*this, info, value);
            checkValidParamValue(*param, value);
            _params.insert_or_assign(info.id, std::move(param));
        } else {
            // update param info
            if (!it->second->isInfoEqualTo(info)) {
                if (!clapParamsRescanMayInfoChange(flags)) {
                    std::ostringstream msg;
                    msg << "a parameter's info did change, but the flag CLAP_PARAM_RESCAN_INFO "
                           "was not specified; id: "
                        << info.id << ", name: " << info.name << ", module: " << info.module
                        << std::endl;
                    throw std::logic_error(msg.str());
                }

                if (!(flags & CLAP_PARAM_RESCAN_ALL) &&
                    !it->second->isInfoCriticallyDifferentTo(info)) {
                    std::ostringstream msg;
                    msg << "a parameter's info has critical changes, but the flag CLAP_PARAM_RESCAN_ALL "
                           "was not specified; id: "
                        << info.id << ", name: " << info.name << ", module: " << info.module
                        << std::endl;
                    throw std::logic_error(msg.str());
                }

                it->second->setInfo(info);
            }

            double value = getParamValue(info);
            if (it->second->value() != value) {
                if (!clapParamsRescanMayValueChange(flags)) {
                    std::ostringstream msg;
                    msg << "a parameter's value did change but, but the flag CLAP_PARAM_RESCAN_VALUES "
                           "was not specified; id: "
                        << info.id << ", name: " << info.name << ", module: " << info.module
                        << std::endl;
                    throw std::logic_error(msg.str());
                }

                // update param value
                checkValidParamValue(*it->second, value);
                it->second->setValue(value);
                it->second->setModulation(value);
            }
        }
    }

    // remove parameters which are gone
    for (auto it = _params.begin(); it != _params.end();) {
        if (paramIds.find(it->first) != paramIds.end())
            ++it;
        else {
            if (!(flags & CLAP_PARAM_RESCAN_ALL)) {
                std::ostringstream msg;
                auto &info = it->second->info();
                msg << "a parameter was removed, but the flag CLAP_PARAM_RESCAN_ALL was not "
                       "specified; id: "
                    << info.id << ", name: " << info.name << ", module: " << info.module << std::endl;
                throw std::logic_error(msg.str());
            }
            it = _params.erase(it);
        }
    }

    //signal
    if (flags & CLAP_PARAM_RESCAN_ALL)
        paramsChanged();
}


void ClapPlugin::checkValidParamValue(const ClapPluginParam &param, double value) {
    // checkForMainThread();

    if (!param.isValueValid(value)) {
        std::ostringstream msg;
        msg << "Invalid value for param. ";
        param.printInfo(msg);
        msg << "; value: " << value;
        // std::cerr << msg.str() << std::endl;
        throw std::invalid_argument(msg.str());
    }
}

void ClapPlugin::checkForMainThread() {
    if (g_thread_type != ClapThreadType::MainThread) [[unlikely]] {
        qFatal() << "Requires Main Thread!";
        std::terminate();
    }
}

void ClapPlugin::checkForAudioThread() {
    if (g_thread_type != ClapThreadType::AudioThread) {
        qFatal() << "Requires Audio Thread!";
        std::terminate();
    }
}

void ClapPlugin::processNoteOn(int sampleOffset, int channel, int key, int velocity) {
    // checkForAudioThread();

    clap_event_note ev;
    ev.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
    ev.header.type = CLAP_EVENT_NOTE_ON;
    ev.header.time = sampleOffset;
    ev.header.flags = 0;
    ev.header.size = sizeof(ev);
    ev.port_index = 0;
    ev.key = key;
    ev.channel = channel;
    ev.note_id = -1;
    ev.velocity = velocity / 127.0;

    _evIn.push(&ev.header);
}

void ClapPlugin::processNoteOff(int sampleOffset, int channel, int key, int velocity) {
    // checkForAudioThread();

    clap_event_note ev;
    ev.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
    ev.header.type = CLAP_EVENT_NOTE_OFF;
    ev.header.time = sampleOffset;
    ev.header.flags = 0;
    ev.header.size = sizeof(ev);
    ev.port_index = 0;
    ev.key = key;
    ev.channel = channel;
    ev.note_id = -1;
    ev.velocity = velocity / 127.0;

    _evIn.push(&ev.header);
}

void ClapPlugin::Render()
{
    QMutexLocker lock(&objMutex);

    // checkForAudioThread();
    g_thread_type = ClapThreadType::AudioThread; //fake it for now

    if (!_plugin.get())
        return;

    if(closed) // || GetSleep())
        return;

    _process.transport = nullptr;

    _process.in_events = _evIn.clapInputEvents();
    _process.out_events = _evOut.clapOutputEvents();

    if (_scheduleDeactivate) {
        _scheduleDeactivate = false;
        _plugin->stopProcessing();
        return;
    }

    if(_scheduleProcess) {
        _scheduleProcess=false;
        if (!_plugin->startProcessing()) {
            LOG("err start")
            return;
        }
    }


    float **tmpBufOut = new float*[listAudioPinOut->listPins.size()];

    int cpt=0;
    foreach(Pin* pin,listAudioPinOut->listPins) {
        if (pin) {
            AudioPin *audioPin = static_cast<AudioPin*>(pin);
            tmpBufOut[cpt] = (float*)audioPin->GetBuffer()->GetPointerWillBeFilled();
        }
        else {
            LOG("no out pin");
        }
        cpt++;
    }
    _audioOut.data32 = tmpBufOut;
    _audioOut.data64 = nullptr;
    _audioOut.channel_count = cpt;
    _audioOut.constant_mask = 0;
    _audioOut.latency = 0;

    float **tmpBufIn;
    if(listAudioPinIn->listPins.isEmpty()) {
        //no input, don't know what we're supposed to do...
        tmpBufIn = tmpBufOut;
    } else {
        tmpBufIn = new float*[listAudioPinIn->listPins.size()];

        cpt=0;
        foreach(Pin* pin,listAudioPinIn->listPins) {
            tmpBufIn[cpt] = (float*)static_cast<AudioPin*>(pin)->GetBuffer()->ConsumeStack();
            cpt++;
        }
    }
    _audioIn.data32 = tmpBufIn;
    _audioIn.data64 = nullptr;
    _audioIn.channel_count = cpt;
    _audioIn.constant_mask = 0;
    _audioIn.latency = 0;


    _process.audio_inputs = &_audioIn;
    _process.audio_inputs_count = 1;
    _process.audio_outputs = &_audioOut;
    _process.audio_outputs_count = 1;

    _process.frames_count = bufferSize;
    _process.steady_time = 0;

    // if (isPluginProcessing())
        // status =
        _plugin->process(&_process);

    _evIn.clear();
    _evOut.clear();

    if(tmpBufOut!=tmpBufIn)
        delete[] tmpBufIn;

    delete[] tmpBufOut;

    //send result
    //=========================
    foreach(Pin* pin,listAudioPinOut->listPins) {
        static_cast<AudioPin*>(pin)->GetBuffer()->ConsumeStack();
        static_cast<AudioPin*>(pin)->SendAudioBuffer();
    }
}


void ClapPlugin::MidiMsgFromInput(long msg)
{
    QMutexLocker lock(&objMutex);

    int status = msg & 0xF0; // MidiStatus(msg);

    int chan = msg & 0x0F;
    int d1 = MidiData1(msg);
    int d2 = MidiData2(msg);

    switch(status) {
    case MidiConst::noteOn :
        if(d2==0) {
            processNoteOff(0,chan,d1,d2);
        } else {
            processNoteOn(0,chan,d1,d2);
        }
        break;
    case MidiConst::noteOff :
        processNoteOff(0,chan,d1,d2);
        break;
    }
}
