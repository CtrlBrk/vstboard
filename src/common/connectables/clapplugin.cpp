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

#include "mainwindow.h"
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

bool ClapPlugin::Close()
{
    checkForMainThread();

    if (!_library.isLoaded())
        return true;

    if (_isGuiCreated) {
        _plugin->guiDestroy();
        _isGuiCreated = false;
        _isGuiVisible = false;
    }

    setPluginState(ActiveAndReadyToDeactivate);
    deactivate();

    // SetSleep(true);

    if (_plugin.get())
    {
        _plugin->destroy();
        // _plugin->reset();
    }

    _pluginEntry->deinit();
    _pluginEntry = nullptr;

    delete editorWnd;

    terminateThreadPool();
    _library.unload();

    return true;
}

bool ClapPlugin::Open()
{
    QMutexLocker lock(&objMutex);

    checkForMainThread();

    // if (_library.isLoaded())
        // Close();

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

    if(objInfo.apiName.toInt()!=0) {
        //shell has been selected
        objInfo.id = objInfo.apiName.toInt();
    } else {

        auto count = _pluginFactory->get_plugin_count(_pluginFactory);
        if (objInfo.id > count) {
            qWarning() << "plugin index greater than count :" << count;
            return false;
        }

        if(count>1) {

            _MSGOBJ(msg,FixedObjId::shellselect);
            msg.prop[MsgObject::Id] = GetIndex();
            msg.prop[MsgObject::ObjInfo] = QVariant::fromValue(objInfo);
            for(int i=0; i<count; i++) {
                auto desc = _pluginFactory->get_plugin_descriptor(_pluginFactory, i);
                _MSGOBJ(plug,FixedObjId::ND);
                plug.prop[MsgObject::Name] = QString(desc->name);
                plug.prop[MsgObject::Id] = i;
                msg.children << plug;
            }

            msgCtrl->SendMsg(msg);
            return true;
        }
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

    const auto plugin = _pluginFactory->create_plugin(_pluginFactory, clapHost() , desc->id);

    if (!plugin) {
        qWarning() << "could not create the plugin with id: " << desc->id;
        return false;
    }

    _plugin = std::make_unique<PluginProxy>(*plugin, *this);

    if (!_plugin->init()) {
        qWarning() << "could not init the plugin with id: " << desc->id;
        return false;
    }

    paramsRescan(CLAP_PARAM_RESCAN_ALL);
    // scanQuickControls();
    // pluginLoadedChanged(true);


    //create all parameters pins
    int cpt=0;
    for (auto it = _params.begin(); it != _params.end();) {
        listParameterPinIn->AddPin(cpt,it->first);

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

        CreateEditorWindow();

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




    connect(&_idleTimer, &QTimer::timeout, this, &ClapPlugin::idle);
    _idleTimer.start(1000 / 30);

    closed=false;
    return true;
}

void ClapPlugin::ReceiveMsg(const MsgObject &msg)
{
    //reload a shell plugin with the selected id
    if(msg.prop.contains(MsgObject::ObjInfo) && objInfo.id==0) {
        objInfo = msg.prop[MsgObject::ObjInfo].value<ObjectInfo>();
        if(objInfo.id == 0) {
            myHost->undoStack.undo();
        } else {
            SetMsgEnabled(false);
            int lastProg = currentProgId;
            Object::LoadProgram(TEMP_PROGRAM);
            delete listPrograms.take(lastProg);
            Open();
            SetSleep(false);
            Object::LoadProgram(lastProg);

            SetMsgEnabled(true);
            UpdateView();
        }
        return;
    }

    Object::ReceiveMsg(msg);
}

void ClapPlugin::SetSleep(bool sleeping)
{
    if(closed) {
        return;
    }

    QMutexLocker lock(&objMutex);

    if(sleeping) {
        deactivate();
    } else {
        activate(sampleRate,bufferSize);

        if(_plugin->canUseLatency()) {
            SetInitDelay(_plugin->latencyGet());
        }
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

            auto it = _params.find(info.clapId);
            if (it == _params.end()) {
                LOG("parameter id out of range"<<info.clapId);
            } else {
                args.value = it->second->value();
                args.name = it->second->info().name;
            }
            args.clapId = it->first;
            args.visible = !hasEditor;
            args.isRemoveable = hasEditor;
            args.nameCanChange = false;//hasEditor;
            return PinFactory::MakePin(args);

        }
    }

    return 0;
}
/*
void ClapPlugin::paramValueChanged()
{
    ClapPluginParam* parm = static_cast<ClapPluginParam*>(sender());

    if(!parm){
        LOG("param not found")
        return;
    }

    ParameterPin *pin = static_cast<ParameterPin*>(listParameterPinIn->listPins.value(parm->pinNumber,0));
    if(!pin)
        return;

    pin->ChangeOutputValue(parm->value(),true);

}
*/
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

        auto &info = it->second->info();
        auto param = it->second.get();
        double newval = value * (info.max_value - info.min_value) + info.min_value;
        // LOG("parm value " << newval)
        setParamValueByHost(*param,newval);
    }
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
    checkForAudioThread();

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
    checkForAudioThread();

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


void ClapPlugin::processCC(int sampleOffset, int channel, int cc, int value) {
    checkForAudioThread();

    clap_event_midi ev;
    ev.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
    ev.header.type = CLAP_EVENT_MIDI;
    ev.header.time = sampleOffset;
    ev.header.flags = 0;
    ev.header.size = sizeof(ev);
    ev.port_index = 0;
    ev.data[0] = 0xB0 | channel;
    ev.data[1] = cc;
    ev.data[2] = value;

    _evIn.push(&ev.header);
}

void ClapPlugin::generatePluginInputEvents() {
    _appToEngineValueQueue.consume(
        [this](clap_id param_id, const AppToEngineParamQueueValue &value) {
            clap_event_param_value ev;
            ev.header.time = 0;
            ev.header.type = CLAP_EVENT_PARAM_VALUE;
            ev.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
            ev.header.flags = 0;
            ev.header.size = sizeof(ev);
            ev.param_id = param_id;
            ev.cookie = value.cookie; // _settings.shouldProvideCookie() ? value.cookie : nullptr;
            ev.port_index = 0;
            ev.key = -1;
            ev.channel = -1;
            ev.note_id = -1;
            ev.value = value.value;
            _evIn.push(&ev.header);
        });

    _appToEngineModQueue.consume([this](clap_id param_id, const AppToEngineParamQueueValue &value) {
        clap_event_param_mod ev;
        ev.header.time = 0;
        ev.header.type = CLAP_EVENT_PARAM_MOD;
        ev.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
        ev.header.flags = 0;
        ev.header.size = sizeof(ev);
        ev.param_id = param_id;
        ev.cookie = value.cookie; // _settings.shouldProvideCookie() ? value.cookie : nullptr;
        ev.port_index = 0;
        ev.key = -1;
        ev.channel = -1;
        ev.note_id = -1;
        ev.amount = value.value;
        _evIn.push(&ev.header);
    });
}


void ClapPlugin::Render()
{
    // checkForAudioThread();
    g_thread_type = ClapThreadType::AudioThread; //fake it for now

    if (!_plugin.get())
        return;

    // Can't process a plugin that is not active
    if (!isPluginActive())
        return;

    // Do we want to deactivate the plugin?
    if (_scheduleDeactivate) {
        _scheduleDeactivate = false;
        if (_state == ActiveAndProcessing)
            _plugin->stopProcessing();
        setPluginState(ActiveAndReadyToDeactivate);
        return;
    }

    // We can't process a plugin which failed to start processing
    if (_state == ActiveWithError)
        return;

    QMutexLocker lock(&objMutex);

    if(closed) // || GetSleep())
        return;

    _process.transport = nullptr;

    _process.in_events = _evIn.clapInputEvents();
    _process.out_events = _evOut.clapOutputEvents();


    _evOut.clear();
    generatePluginInputEvents();


    if (isPluginSleeping()) {
        if (!_scheduleProcess && _evIn.empty())
            // The plugin is sleeping, there is no request to wake it up and there are no events to
            // process
            return;

        _scheduleProcess = false;
        if (!_plugin->startProcessing()) {
            // the plugin failed to start processing
            setPluginState(ActiveWithError);
            return;
        }

        setPluginState(ActiveAndProcessing);
    }

    float **tmpBufOut = new float*[listAudioPinOut->listPins.size()];

    if(listAudioPinOut->listPins.isEmpty()) {
        _audioOut.data32 = nullptr;
        _audioOut.data64 = nullptr;
        _audioOut.channel_count = 0;
        _audioOut.constant_mask = 0;
        _audioOut.latency = 0;
        _process.audio_outputs_count = 0;
    } else {
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
        _process.audio_outputs_count = 1;
    }

    float **tmpBufIn;
    if(listAudioPinIn->listPins.isEmpty()) {
        //no input, don't know what we're supposed to do...
        tmpBufIn = tmpBufOut;

        _audioIn.data32 = nullptr;
        _audioIn.data64 = nullptr;
        _audioIn.channel_count = 0;
        _audioIn.constant_mask = 0;
        _audioIn.latency = 0;
        _process.audio_inputs_count = 0;
    } else {
        tmpBufIn = new float*[listAudioPinIn->listPins.size()];

        int cpt=0;
        foreach(Pin* pin,listAudioPinIn->listPins) {
            tmpBufIn[cpt] = (float*)static_cast<AudioPin*>(pin)->GetBuffer()->ConsumeStack();
            cpt++;
        }

        _audioIn.data32 = tmpBufIn;
        _audioIn.data64 = nullptr;
        _audioIn.channel_count = cpt;
        _audioIn.constant_mask = 0;
        _audioIn.latency = 0;
        _process.audio_inputs_count = 1;
    }

    _process.audio_inputs = &_audioIn;
    _process.audio_outputs = &_audioOut;
    _process.frames_count = bufferSize;
    _process.steady_time = 0;

    int32_t status = CLAP_PROCESS_SLEEP;
    if (isPluginProcessing())
        status = _plugin->process(&_process);

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

    g_thread_type = ClapThreadType::Unknown;
}


void ClapPlugin::setPluginState(PluginState state) {
    switch (state) {
    case Inactive:
        Q_ASSERT(_state == ActiveAndReadyToDeactivate);
        break;

    case InactiveWithError:
        Q_ASSERT(_state == Inactive);
        break;

    case ActiveAndSleeping:
        Q_ASSERT(_state == Inactive || _state == ActiveAndProcessing);
        break;

    case ActiveAndProcessing:
        Q_ASSERT(_state == ActiveAndSleeping);
        break;

    case ActiveWithError:
        Q_ASSERT(_state == ActiveAndProcessing);
        break;

    case ActiveAndReadyToDeactivate:
        Q_ASSERT(_state == ActiveAndProcessing || _state == ActiveAndSleeping ||
                 _state == ActiveWithError);
        break;

    default:
        std::terminate();
    }

    _state = state;
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

void ClapPlugin::setParamValueByHost(ClapPluginParam &param, double value) {
    checkForMainThread();

    param.setValue(value);

    _appToEngineValueQueue.set(param.info().id, {param.info().cookie, value});
    _appToEngineValueQueue.producerDone();
    paramsRequestFlush();
}

void ClapPlugin::setParamModulationByHost(ClapPluginParam &param, double value) {
    checkForMainThread();

    param.setModulation(value);

    _appToEngineModQueue.set(param.info().id, {param.info().cookie, value});
    _appToEngineModQueue.producerDone();
    paramsRequestFlush();
}

bool ClapPlugin::isPluginActive() const {
    switch (_state) {
    case Inactive:
    case InactiveWithError:
        return false;
    default:
        return true;
    }
}

bool ClapPlugin::isPluginProcessing() const { return _state == ActiveAndProcessing; }

bool ClapPlugin::isPluginSleeping() const { return _state == ActiveAndSleeping; }


void ClapPlugin::activate(int32_t sample_rate, int32_t blockSize) {
    checkForMainThread();

    if (!_plugin.get())
        return;

    assert(!isPluginActive());
    if (!_plugin->activate(sample_rate, blockSize, blockSize)) {
        setPluginState(InactiveWithError);
        return;
    }

    _scheduleProcess = true;
    setPluginState(ActiveAndSleeping);
}

void ClapPlugin::deactivate() {
    checkForMainThread();

    if (!isPluginActive())
        return;

    while (isPluginProcessing() || isPluginSleeping()) {
        _scheduleDeactivate = true;
        QThread::msleep(10);
    }
    _scheduleDeactivate = false;

    _plugin->deactivate();
    setPluginState(Inactive);
}

void ClapPlugin::paramFlushOnMainThread() {
    checkForMainThread();

    assert(!isPluginActive());

    _scheduleParamFlush = false;

    _evIn.clear();
    _evOut.clear();

    generatePluginInputEvents();

    if (_plugin->canUseParams())
        _plugin->paramsFlush(_evIn.clapInputEvents(), _evOut.clapOutputEvents());
    handlePluginOutputEvents();

    _evOut.clear();
    _engineToAppValueQueue.producerDone();
}

void ClapPlugin::idle() {
    checkForMainThread();

    // Try to send events to the audio engine
    _appToEngineValueQueue.producerDone();
    _appToEngineModQueue.producerDone();

    _engineToAppValueQueue.consume(
        [this](clap_id param_id, const EngineToAppParamQueueValue &value) {
            auto it = _params.find(param_id);
            if (it == _params.end()) {
                std::ostringstream msg;
                msg << "Plugin produced a CLAP_EVENT_PARAM_SET with an unknown param_id: " << param_id;
                throw std::invalid_argument(msg.str());
            }

            if (value.has_value) {
                it->second->setValue(value.value);

                int pinNum = it->second->pinNumber;
                float val = (value.value - it->second->info().min_value) / (it->second->info().max_value - it->second->info().min_value);
                val = std::min(val,1.f);
                val = std::max(val,.0f);
                ParamChangedFromPlugin(pinNum,val);
            }

            if (value.has_gesture)
                it->second->setIsAdjusting(value.is_begin);

            // emit paramAdjusted(param_id);
        });

    if (_scheduleParamFlush && !isPluginActive()) {
        paramFlushOnMainThread();
    }
    /*

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
    g_thread_type = ClapThreadType::AudioThread;

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
    case MidiConst::ctrl:
        processCC(0,chan,d1,d2);
        break;
    }
}

void ClapPlugin::CreateEditorWindow()
{
    //already done
    if(editorWnd)
        return;

    editorWnd = new View::ClapPluginWindow( static_cast<QMainWindow*>(myHost->GetMainWindow()) );
    //editorWnd = new View::ClapPluginWindow();
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


// clap_host_gui
//=====================

void ClapPlugin::guiResizeHintsChanged() noexcept {
    // TODO
}

bool ClapPlugin::guiRequestResize(uint32_t width, uint32_t height) noexcept {
    if(!editorWnd)
        return false;

    editorWnd->SetWindowSize(width,height);

    // QMetaObject::invokeMethod(
    //     Application::instance().mainWindow(),
    //     [width, height] { Application::instance().mainWindow()->resizePluginView(width, height); },
    //     Qt::QueuedConnection);

    return true;
}


bool ClapPlugin::guiRequestShow() noexcept {
    if(!editorWnd)
        return false;

    editorWnd->show();

    // QMetaObject::invokeMethod(
    //     Application::instance().mainWindow(),
    //     [] { Application::instance().mainWindow()->showPluginWindow(); },
    //     Qt::QueuedConnection);

    return true;
}

bool ClapPlugin::guiRequestHide() noexcept {
    if(!editorWnd)
        return false;

    editorWnd->hide();

    // QMetaObject::invokeMethod(
    //     Application::instance().mainWindow(),
    //     [] { Application::instance().mainWindow()->hidePluginWindow(); },
    //     Qt::QueuedConnection);

    return true;
}

void ClapPlugin::guiClosed(bool wasDestroyed) noexcept { checkForMainThread(); }



// clap_host_params
//=====================


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


void ClapPlugin::paramsClear(clap_id paramId, clap_param_clear_flags flags) noexcept
{
    checkForMainThread();
}


void ClapPlugin::paramsRequestFlush() noexcept {
    if (!isPluginActive() && threadCheckIsMainThread()) {
        // Perform the flush immediately
        paramFlushOnMainThread();
        return;
    }

    _scheduleParamFlush = true;
    return;
}

// clap_host_state
//================


void ClapPlugin::stateMarkDirty() noexcept {
    checkForMainThread();


    if (!_plugin->canUseState())
        throw std::logic_error("Plugin called clap_host_state.set_dirty() but the host does not "
                               "provide a complete clap_plugin_state interface.");

    _stateIsDirty = true;
}


// clap_host_thread_check
// ======================

bool ClapPlugin::threadCheckIsMainThread() const noexcept {
    return g_thread_type == ClapThreadType::MainThread;
}

bool ClapPlugin::threadCheckIsAudioThread() const noexcept {
    return g_thread_type == ClapThreadType::AudioThread;
}


// clap_host_thread_pool
//========================

bool ClapPlugin::threadPoolRequestExec(uint32_t num_tasks) noexcept {
    checkForAudioThread();

    if (!_plugin->canUseThreadPool())
        throw std::logic_error("Called request_exec() without providing clap_plugin_thread_pool to "
                               "execute the job.");

    Q_ASSERT(!_threadPoolStop);
    Q_ASSERT(!_threadPool.empty());

    if (num_tasks == 0)
        return true;

    if (num_tasks == 1) {
        _plugin->threadPoolExec(0);
        return true;
    }

    _threadPoolTaskIndex = 0;
    _threadPoolSemaphoreProd.release(num_tasks);
    _threadPoolSemaphoreDone.acquire(num_tasks);
    return true;
}

void ClapPlugin::LoadProgram(int progId)
{
    Object::LoadProgram(progId);

    if(_plugin && _plugin->canUseState()) {

        QByteArray ba = currentProgram->listOtherValues.value(0, QByteArray()).toByteArray();

        clap_istream istream;
        istream.ctx = &ba;
        istream.read = [] (const clap_istream *stream, void *buffer, uint64_t size) -> int64_t {
            QByteArray *_ba = (QByteArray *)stream->ctx;
            // buffer = _ba->data();
            size = std::min(size,(uint64_t)_ba->size());
            QByteArray b = _ba->first( size );
            _ba->remove(0,size);
            memcpy_s(buffer,size,b.data(),size);
            return b.size();
        };

        if(ba.size()>0) {
            _plugin->stateLoad(&istream);
        }
    }

    currentProgram->Load(listParameterPinIn,listParameterPinOut);
}

void ClapPlugin::SaveProgram()
{
    if(!currentProgram || !currentProgram->IsDirty())
        return;

    if(_plugin && _plugin->canUseState()) {
        QByteArray ba;

        clap_ostream ostream;
        ostream.ctx = &ba;
        ostream.write = [] (const clap_ostream *stream, const void *buffer, uint64_t size) -> int64_t {
            QByteArray *_ba = (QByteArray *)stream->ctx;
            _ba->append( QByteArray::fromRawData((char*)buffer, size) );
            return size;
        };

        _plugin->stateSave(&ostream);

        currentProgram->listOtherValues.insert(0,ba);
    }

    paramsRescan(CLAP_PARAM_RESCAN_ALL);
    for (auto it = _params.begin(); it != _params.end();) {
        int pinNum = it->second->pinNumber;
        ParameterPinIn *pin = static_cast<ParameterPinIn*>(listParameterPinIn->GetPin(pinNum,false));
        pin->ChangeValue((float)it->second->value());
        ++it;
    }
    Object::SaveProgram();
}


void ClapPlugin::fromJson(QJsonObject &json)
{
    // Object::fromJson(json);

    // if(savedChunk) {
    //     delete savedChunk;
    //     savedChunk=0;
    //     savedChunkSize=0;
    // }

    // if(json.contains("chunk")) {
    //     QByteArray ba = QByteArray::fromHex( json["chunk"].toString().toUtf8() );

    //     savedChunkSize = ba.size();
    //     savedChunk = new char[savedChunkSize];
    //     std::memcpy(savedChunk,ba.constData(),savedChunkSize);

    //     if(pEffect && (pEffect->flags & effFlagsProgramChunks)) {
    //         EffSetChunk(savedChunk ,savedChunkSize);
    //     }
    // }

}

void ClapPlugin::toJson(QJsonObject &json)
{
    // QByteArray ba;

    // clap_ostream ostream;
    // ostream.ctx = &ba,
    // ostream.write = [] (const clap_ostream *stream, const void *buffer, uint64_t size) -> int64_t {
    //     QByteArray *_ba = (QByteArray *)stream->ctx;
    //     _ba->append( QByteArray::fromRawData((char*)buffer, size) );
    //     return size;
    // };

    // _plugin->stateSave(&ostream);
    // json["chunk"] = QString(ba.toHex());

}
