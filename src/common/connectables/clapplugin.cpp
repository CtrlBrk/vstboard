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
#include "views/clappluginwindow.h"

#include <clap/helpers/host.hxx>
#include <clap/helpers/plugin-proxy.hxx>
#include <clap/helpers/reducing-param-queue.hxx>

#include <unordered_set>
// #include "claphost.h"

#include "commands/comaddpin.h"
#include "commands/comremovepin.h"


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
             ),
    editorWnd(0),
    _idleTimer(this)
{
    g_thread_type = ClapThreadType::MainThread;
    initThreadPool();
}

ClapPlugin::~ClapPlugin()
{
    Close();
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
    checkForMainThread();

    if (_isGuiCreated) {
        _plugin->guiDestroy();
        _isGuiCreated = false;
        _isGuiVisible = false;


    }

    SetSleep(true);

    if (_plugin.get())
    {
        _plugin->destroy();
        // _plugin->reset();
    }

    _pluginEntry->deinit();
    _pluginEntry = nullptr;

    delete editorWnd;
    _library.unload();

    checkForMainThread();
    terminateThreadPool();

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
    int cpt=0;
    for (auto it = _params.begin(); it != _params.end();) {
        Pin *p = listParameterPinIn->AddPin(cpt);
        p->SetClapId(it->first);

        //TODO : fix the double index of clapId and pinNumber
        it->second->pinNumber=cpt;

        // ClapPluginParam *param = it->second.get();
        // connect(param, &ClapPluginParam::valueChanged, this, &ClapPlugin::paramValueChanged);

        ++it;
        cpt++;
    }

    // int nbParam = _params.size();
    // for(int i=0;i<nbParam;i++) {
        // Pin * p = listParameterPinIn->AddPin(i);
    // }



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

    if(_plugin->canUseGui()) {
        CreateEditorWindow();
    }


    connect(&_idleTimer, &QTimer::timeout, this, &ClapPlugin::idle);
    _idleTimer.start(1000 / 30);

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

    bool hasEditor = _plugin->canUseGui();

    Pin *newPin = Object::CreatePin(info);
    if(newPin)
        return newPin;

    pinConstructArgs args(info);
    args.parent = this;
    args.bufferSize = myHost->GetBufferSize();
    args.doublePrecision = doublePrecision;


    if(info.type == PinType::Parameter) { // && info.direction == PinDirection::Input) {

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
            // if(!closed) {
                // args.name = _params[info.pinNumber]->info().name;
                // args.value = _params[info.pinNumber]->value();
            // }
            args.visible = !hasEditor;
            args.isRemoveable = hasEditor;
            args.nameCanChange = hasEditor;
            return PinFactory::MakePin(args);

        }
    }

    return 0;
}

// void ClapPlugin::paramValueChanged()
// {
//     ClapPluginParam* parm = static_cast<ClapPluginParam*>(sender());

//     if(!parm){
//         LOG("param not found")
//         return;
//     }

//     ParameterPin *pin = static_cast<ParameterPin*>(listParameterPinIn->listPins.value(parm->pinNumber,0));
//     if(!pin)
//         return;

//     pin->ChangeOutputValue(parm->value(),true);

// }

QString ClapPlugin::GetParameterName(ConnectionInfo pinInfo)
{
    if(closed)
        return "";

    auto it = _params.find(pinInfo.clapId);
    if (it == _params.end()) {
        LOG("parameter id out of range"<<pinInfo.clapId);
        return "";
    }

    return it->second->info().name;


    // if(pinInfo.pinNumber < _params.size()){

        // ClapPluginParam para = _params[pinInfo.pinNumber];
        // clap_param_info inf = _params[pinInfo.pinNumber]->info();
        // QString na( inf.name );
        // return QString::fromLatin1( _params[pinInfo.pinNumber]->info().name );
        // args.value = _params[pinInfo.pinNumber]->value();
    // }
    // LOG("parameter id out of range"<<pinInfo.pinNumber);
    // return "";
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

void ClapPlugin::OnParameterChanged(ConnectionInfo pinInfo, float value)
{
    Object::OnParameterChanged(pinInfo,value);

    if(closed)
        return;

    if(pinInfo.direction == PinDirection::Input) {
        auto it = _params.find(pinInfo.clapId);
        if (it == _params.end()) {
            LOG("parameter id out of range"<<pinInfo.clapId);
            return ;
        }
        // it->second->setValue(value);

        clap_event_param_value ev;
        ev.header.time = 0;
        ev.header.type = CLAP_EVENT_PARAM_VALUE;
        ev.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
        ev.header.flags = 0;
        ev.header.size = sizeof(ev);
        ev.param_id = it->first;
        ev.cookie = nullptr;//_settings.shouldProvideCookie() ? value.cookie : nullptr;
        ev.port_index = 0;
        ev.key = -1;
        ev.channel = -1;
        ev.note_id = -1;

        ev.value = 0;

        if(it->second) {
            double newval = value * (it->second->info().max_value - it->second->info().min_value) + it->second->info().min_value;
            ev.value = newval; //it->second->value();
        } else {
            LOG("param not found")
        }


        _evIn.push(&ev.header);
    }
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


    handlePluginOutputEvents();

    _evIn.clear();
    _evOut.clear();

    _engineToAppValueQueue.producerDone();

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

void ClapPlugin::UserAddPin(const ConnectionInfo &info)
{
    if(info.type!=PinType::Parameter)
        return;

    if(info.direction!=PinDirection::Input)
        return;

    if(listParameterPinIn->listPins.contains(info.pinNumber))
        static_cast<ParameterPinIn*>(listParameterPinIn->listPins.value(info.pinNumber))->SetVisible(true);
    OnProgramDirty();
}

void ClapPlugin::ParamChangedFromPlugin(int pinNum,float val)
{

    ParameterPin *pin = static_cast<ParameterPin*>(listParameterPinIn->listPins.value(pinNum,0));
    if(!pin)
        return;

    switch(GetLearningMode()) {
    case LearningMode::unlearn :
        if(pin->GetVisible())
            myHost->undoStack.push( new ComRemovePin(myHost, pin->GetConnectionInfo()) );
        break;

    case LearningMode::learn :
        if(!pin->GetVisible())
            myHost->undoStack.push( new ComAddPin(myHost, pin->GetConnectionInfo()) );

    case LearningMode::off :
        pin->ChangeOutputValue(val,true);
    }
}

void ClapPlugin::idle() {
    checkForMainThread();

    // Try to send events to the audio engine
    // _appToEngineValueQueue.producerDone();
    // _appToEngineModQueue.producerDone();

    _engineToAppValueQueue.consume(
        [this](clap_id param_id, const EngineToAppParamQueueValue &value) {
            auto it = _params.find(param_id);
            if (it == _params.end()) {
                std::ostringstream msg;
                msg << "Plugin produced a CLAP_EVENT_PARAM_SET with an unknown param_id: " << param_id;
                throw std::invalid_argument(msg.str());
            }

            if (value.has_value)
                it->second->setValue(value.value);

            if (value.has_gesture)
                it->second->setIsAdjusting(value.is_begin);

            int pinNum = it->second->pinNumber;
            float val = (value.value - it->second->info().min_value) / (it->second->info().max_value - it->second->info().min_value);
            val = std::min(val,1.f);
            val = std::max(val,.0f);
            LOG(it->second->info().max_value << it->second->info().min_value << value.value << val)
            ParamChangedFromPlugin(pinNum,val);
            // emit paramAdjusted(param_id);
        });
    /*
    if (_scheduleParamFlush && !isPluginActive()) {
        paramFlushOnMainThread();
    }

    if (_scheduleMainThreadCallback) {
        _scheduleMainThreadCallback = false;
        _plugin->onMainThread();
    }

    if (_scheduleRestart) {
        deactivate();
        _scheduleRestart = false;
        activate(_engine._sampleRate, _engine._nframes);
    }
*/
}

void ClapPlugin::handlePluginOutputEvents() {
    for (uint32_t i = 0; i < _evOut.size(); ++i) {
        auto h = _evOut.get(i);
        switch (h->type) {
        case CLAP_EVENT_PARAM_GESTURE_BEGIN: {
            auto ev = reinterpret_cast<const clap_event_param_gesture *>(h);
            bool &isAdj = _isAdjustingParameter[ev->param_id];

            if (isAdj)
                throw std::logic_error("The plugin sent BEGIN_ADJUST twice");
            isAdj = true;

            EngineToAppParamQueueValue v;
            v.has_gesture = true;
            v.is_begin = true;
            _engineToAppValueQueue.setOrUpdate(ev->param_id, v);
            break;
        }

        case CLAP_EVENT_PARAM_GESTURE_END: {
            auto ev = reinterpret_cast<const clap_event_param_gesture *>(h);
            bool &isAdj = _isAdjustingParameter[ev->param_id];

            if (!isAdj)
                throw std::logic_error("The plugin sent END_ADJUST without a preceding BEGIN_ADJUST");
            isAdj = false;
            EngineToAppParamQueueValue v;
            v.has_gesture = true;
            v.is_begin = false;
            _engineToAppValueQueue.setOrUpdate(ev->param_id, v);
            break;
        }

        case CLAP_EVENT_PARAM_VALUE: {
            auto ev = reinterpret_cast<const clap_event_param_value *>(h);
            EngineToAppParamQueueValue v;
            v.has_value = true;
            v.value = ev->value;
            _engineToAppValueQueue.setOrUpdate(ev->param_id, v);
            break;
        }
        }
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

void ClapPlugin::CreateEditorWindow()
{
    //already done
    if(editorWnd)
        return;

    // editorWnd = new View::ClapPluginWindow(myHost->GetMainWindow());
    editorWnd = new View::ClapPluginWindow();
    editorWnd->setWindowTitle(objectName());

    connect(this,SIGNAL(HideEditorWindow()),
            editorWnd,SLOT(hide()),
            Qt::QueuedConnection);

    connect(editorWnd, SIGNAL(Hide()),
            this, SLOT(OnEditorClosed()));

    connect(editorWnd,SIGNAL(destroyed()),
            this,SLOT(EditorDestroyed()));

    connect(this,SIGNAL(WindowSizeChange(int,int)),
            editorWnd,SLOT(SetWindowSize(int,int)));

    setParentWindow(editorWnd->GetWinId());

}

void ClapPlugin::OnEditorClosed()
{
    ToggleEditor(false);
}

void ClapPlugin::EditorDestroyed()
{
    editorWnd = 0;
    listParameterPinIn->listPins.value(FixedPinNumber::editorVisible)->SetVisible(false);
}

static clap_window makeClapWindow(WId window) {
    clap_window w;
#if defined(Q_OS_LINUX)
    w.api = CLAP_WINDOW_API_X11;
    w.x11 = window;
#elif defined(Q_OS_MACX)
    w.api = CLAP_WINDOW_API_COCOA;
    w.cocoa = reinterpret_cast<clap_nsview>(window);
#elif defined(Q_OS_WIN)
    w.api = CLAP_WINDOW_API_WIN32;
    w.win32 = reinterpret_cast<clap_hwnd>(window);
#endif

    return w;
}

void ClapPlugin::setParentWindow(WId parentWindow) {
    checkForMainThread();

    if (!_plugin->canUseGui())
        return;

    if (_isGuiCreated) {
        _plugin->guiDestroy();
        _isGuiCreated = false;
        _isGuiVisible = false;
    }

    _guiApi = getCurrentClapGuiApi();

    _isGuiFloating = false;
    if (!_plugin->guiIsApiSupported(_guiApi, false)) {
        if (!_plugin->guiIsApiSupported(_guiApi, true)) {
            qWarning() << "could find a suitable gui api";
            return;
        }
        _isGuiFloating = true;
    }

    auto w = makeClapWindow(parentWindow);
    if (!_plugin->guiCreate(w.api, _isGuiFloating)) {
        qWarning() << "could not create the plugin gui";
        return;
    }

    _isGuiCreated = true;
    assert(_isGuiVisible == false);

    if (_isGuiFloating) {
        _plugin->guiSetTransient(&w);
        _plugin->guiSuggestTitle("using clap-host suggested title");
    } else {
        uint32_t width = 0;
        uint32_t height = 0;

        if (!_plugin->guiGetSize(&width, &height)) {
            qWarning() << "could not get the size of the plugin gui";
            _isGuiCreated = false;
            _plugin->guiDestroy();
            return;
        }

        emit WindowSizeChange(width,height);
        // Application::instance().mainWindow()->resizePluginView(width, height);

        if (!_plugin->guiSetParent(&w)) {
            qWarning() << "could embbed the plugin gui";
            _isGuiCreated = false;
            _plugin->guiDestroy();
            return;
        }
    }

    setPluginWindowVisibility(true);
}

void ClapPlugin::setPluginWindowVisibility(bool isVisible) {
    checkForMainThread();

    if (!_isGuiCreated)
        return;

    if (isVisible && !_isGuiVisible) {
        _plugin->guiShow();
        _isGuiVisible = true;
    } else if (!isVisible && _isGuiVisible) {
        _plugin->guiHide();
        _isGuiVisible = false;
    }
}


const char *ClapPlugin::getCurrentClapGuiApi() {
#if defined(Q_OS_LINUX)
    return CLAP_WINDOW_API_X11;
#elif defined(Q_OS_WIN)
    return CLAP_WINDOW_API_WIN32;
#elif defined(Q_OS_MACOS)
    return CLAP_WINDOW_API_COCOA;
#else
#   error "unsupported platform"
#endif
}

void ClapPlugin::OnShowEditor()
{
    if(!editorWnd)
        CreateEditorWindow();

    if(!editorWnd)
        return;

    if(editorWnd->isVisible())
        return;

    editorWnd->show();
    //    editorWnd->raise();
    connect(myHost->updateViewTimer,SIGNAL(timeout()),
            this,SLOT(EditIdle()));
    // editorWnd->LoadAttribs(currentViewAttr);
}

void ClapPlugin::OnHideEditor()
{
    if(!editorWnd)
        return;

    // editorWnd->SaveAttribs(currentViewAttr);

    if(myHost->settings->GetSetting("fastEditorsOpenClose",true).toBool()) {
        disconnect(myHost->updateViewTimer,SIGNAL(timeout()),
                   this,SLOT(EditIdle()));
        emit HideEditorWindow();
    } else {
        editorWnd->disconnect();
        // editorWnd->UnsetPlugin();
        disconnect(editorWnd);
        QTimer::singleShot(0,editorWnd,SLOT(close()));
        editorWnd=0;
        objMutex.lock();
        // EffEditClose();
        objMutex.unlock();
    }
}

