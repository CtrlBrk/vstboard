/**************************************************************************
#    Copyright 2010-2025 Raphaël François
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

#include "audiodevices_pa.h"
#include "globals.h"
#include "connectables/objectinfo.h"
#include "mainhosthost.h"
#include "pa_asio.h"
#ifdef WIN32
#include "views/mmeconfigdialog.h"
#include "views/wasapiconfigdialog.h"
#include "views/directxconfigdialog.h"
#endif
#include "connectables/audiodevicein.h"
#include "connectables/audiodeviceout.h"
#include "mainwindow.h"


FakeTimer::FakeTimer(MainHostHost *myHost) :
    QThread(myHost),
    myHost(myHost),
    stop(false)
{
    setObjectName("FakeTimer");
    myHost->SetBufferSize((myHost->GetSampleRate()*FAKE_RENDER_TIMER_MS)/1000);
    start(QThread::TimeCriticalPriority);
}

FakeTimer::~FakeTimer()
{
    LOG("stop thread"<<objectName()<<(Qt::HANDLE)currentThreadId());
    stop=true;
    while(isRunning())
        wait(10);
    LOG("thread stopped"<<objectName()<<(Qt::HANDLE)currentThreadId());
}

void FakeTimer::run()
{

    LOG("start thread"<<objectName()<<(Qt::HANDLE)currentThreadId());
    while(!stop) {
        msleep(FAKE_RENDER_TIMER_MS);
        myHost->Render();
    }
    stop=false;
}

/*!
  \class AudioDevices
  \brief manage the list of audio devices provided by PortAudio
  */

/*!
  \param myHost pointer to the MainHost
  */
AudioDevices::AudioDevices(MainHostHost *myHost, MsgController *msgCtrl, int objId) :
    QObject(myHost),
    MsgHandler(msgCtrl, objId),
//    model(0),
//    countActiveDevices(0),
    fakeRenderTimer(0),
    closing(false),
    paOpened(false),
    myHost(myHost)
{
    fakeRenderTimer = new FakeTimer(myHost);
    timerRefreshDevices.setSingleShot(true);
    timerRefreshDevices.setInterval(5000);

    connect(&timerRefreshDevices, SIGNAL(timeout()),
            this, SLOT(TryToOpenDevice()));
    OpenDevices();
}

/*!
  Destructor, close all the opened devices
  */
AudioDevices::~AudioDevices()
{
    CloseDevices(true);

	{
		QMutexLocker d(&mutexDevices);
		foreach(Connectables::AudioDevice *dev, listAudioDevices)
			delete dev; //dev->DeleteIfUnused();
		listAudioDevices.clear();
	}

    if(fakeRenderTimer)
        delete fakeRenderTimer;
}

void AudioDevices::CloseDevices(bool close)
{
	{
		QMutexLocker l(&mutexClosing);
		if (close) {
			closing = true;
		}
	}

    QMutexLocker d(&mutexDevices);
    foreach(Connectables::AudioDevice *ad, listAudioDevices) {
        ad->SetSleep(true);
    }

    if(paOpened) {
        PaError err=Pa_Terminate();
        if(err!=paNoError) {
            LOG("Pa_Terminate"<<Pa_GetErrorText( err ));
        }
        paOpened=false;
    }
}

void AudioDevices::SleepAll()
{
    QMutexLocker d(&mutexDevices);
   foreach(Connectables::AudioDevice *ad, listAudioDevices) {
       ad->SetSleep(true);
   }
}

void AudioDevices::OpenDevices()
{

    QStringList disabled = myHost->settings->GetSetting("disabledAudioApis").toStringList();
    QList<int>disabledApis;
    foreach(const QString &str, disabled) {
        disabledApis << str.toInt();
    }

    PaError paRet =Pa_Initialize();
    if(paRet!=paNoError) {
        QMessageBox msgBox;
        msgBox.setText(tr("Unable to initialize audio engine : %1").arg( Pa_GetErrorText(paRet) ));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }
    paOpened=true;

    int numDevices;
    numDevices = Pa_GetDeviceCount();
    if( numDevices < 0 )
    {
        printf( "ERROR: Pa_CountDevices returned 0x%x\n", numDevices );
    }

    {
        QMutexLocker l(&mutexClosing);
        closing = true;
    }

    {
        QMutexLocker d(&mutexDevices);
        foreach(Connectables::AudioDevice *ad, listAudioDevices) {
            ad->SetSleep(false);
        }
    }

    {
        QMutexLocker l(&mutexClosing);
        closing = false;
    }

    //rebuild all audio in&out objects
    foreach(QSharedPointer<Connectables::Object>obj, myHost->objFactory->GetListObjects()) {
        if(!obj)
            continue;

        ObjectInfo info( obj->info() );
        if(info.objType == ObjType::AudioInterfaceIn || info.objType == ObjType::AudioInterfaceOut) {
            obj->Open();
        }
    }

    if(MsgEnabled()) {
        BuildModel();
//        myHost->UpdateView();
    }
}

void AudioDevices::ReceiveMsg(const MsgObject &msg)
{
    SetMsgEnabled(true);

    if(msg.prop.contains(MsgObject::GetUpdate)) {
        BuildModel();
        return;
    }

    if(msg.prop.contains(MsgObject::Rescan)) {
        CloseDevices();
        OpenDevices();
        return;
    }

    if(msg.prop.contains(MsgObject::Setup)) {
        ObjectInfo info = msg.prop[MsgObject::Setup].value<ObjectInfo>();
        ConfigDevice(info);
        return;
    }

    if(msg.prop.contains(MsgObject::State)) {
        if( msg.prop[MsgObject::State].toInt() == -1 ) {
            //reset apis
            myHost->settings->SetSetting("disabledAudioApis",QStringList());
        } else {
            //disable an api
            QStringList disabled = myHost->settings->GetSetting("disabledAudioApis").toStringList();
            disabled << msg.prop[MsgObject::State].toString();
            myHost->settings->SetSetting("disabledAudioApis",disabled);
        }

        CloseDevices();
        OpenDevices();
        return;
    }
}

/*!
  Create or update the list of devices
  */
void AudioDevices::BuildModel()
{
    QStringList disabled = myHost->settings->GetSetting("disabledAudioApis").toStringList();
    QList<int>disabledApis;
    foreach(const QString &str, disabled) {
        disabledApis << str.toInt();
    }

    MSGOBJ();
    msg.prop[MsgObject::Update]=1;

    for (int i = 0; i < Pa_GetHostApiCount(); ++i) {
        const PaHostApiInfo *apiInfo = Pa_GetHostApiInfo(i);
        //        MsgObject msgApi((int)apiInfo->type);
        _MSGOBJ(msgApi,(int)apiInfo->type);
        msgApi.prop[MsgObject::Name]=apiInfo->name;

        //an api can contain multiple devices with the same name
        QString lastName;
        int cptDuplicateNames=0;

        for (int j=0; j<apiInfo->deviceCount; j++) {
            PaDeviceIndex devIndex = Pa_HostApiDeviceIndexToDeviceIndex(i, j);
            const PaDeviceInfo *devInfo = Pa_GetDeviceInfo( devIndex );

            QString devName(devInfo->name);
            //remove " x64" from device name so we can share files with 32bit version
            devName.remove(QRegularExpression("( )?x64"));

            //count duplicate names
            if(lastName == devName) {
                cptDuplicateNames++;
            } else {
                cptDuplicateNames=0;
                lastName = devName;
            }

            ObjectInfo obj;
            obj.nodeType = NodeType::object;
            obj.objType = ObjType::AudioInterface;
            obj.id = devIndex;
            obj.name = devName;
            obj.api = apiInfo->type;
            obj.duplicateNamesCounter = cptDuplicateNames;
            obj.inputs = devInfo->maxInputChannels;
            obj.outputs = devInfo->maxOutputChannels;

            MsgObject msgDevice;
            msgDevice.prop[MsgObject::Name]=devName;
            msgDevice.prop[MsgObject::ObjInfo]=QVariant::fromValue(obj);
            msgDevice.prop[MsgObject::State]=(bool)listOpenedDevices.contains(obj.api*1000+obj.id);
            msgApi.children << msgDevice;
        }
        msg.children << msgApi;
    }

    msgCtrl->SendMsg(msg);
}

/*!
  Called when a device is added or removed from a panel
  \param objInfo object description
  \param opened true if opened, false if closed
  */
void AudioDevices::OnToggleDeviceInUse(PaHostApiIndex apiId, PaDeviceIndex devId, bool inUse, PaTime /*inLatency*/, PaTime /*outLatency*/, double /*sampleRate*/)
{
    bool oldState = listOpenedDevices.contains(apiId*1000+devId);
    if(oldState == inUse)
        return;

    if(inUse) {
        listOpenedDevices << apiId*1000+devId;
    } else {
        listOpenedDevices.removeAll(apiId*1000+devId);
    }

    //the renderer is normally launched when all the audio devices are ready,
    //if there is no audio device we have to run a timer
    if(!listOpenedDevices.isEmpty()) {
       // myHost->SetBufferSize(1);
        if(fakeRenderTimer) {
            delete fakeRenderTimer;
            fakeRenderTimer=0;
        }
    } else {
        myHost->SetBufferSizeMs(FAKE_RENDER_TIMER_MS);
        if(!fakeRenderTimer && !closing) {
            fakeRenderTimer = new FakeTimer(myHost);

            if(!listAudioDevices.isEmpty() && !timerRefreshDevices.isActive()) {
                //we need some devices but none can be initilized : try again
                timerRefreshDevices.start();
            }
        }
    }

    MSGOBJ();
    msg.prop[MsgObject::State]=inUse;
    msg.prop[MsgObject::Group]=apiId;
    msg.prop[MsgObject::Id]=devId;
    msgCtrl->SendMsg(msg);
}

Connectables::AudioDevice * AudioDevices::AddDevice(ObjectInfo &objInfo)
{
    {
        //search in opened devices
        QMutexLocker l(&mutexDevices);
        foreach(Connectables::AudioDevice *d, listAudioDevices) {
            if(d->IsAnInstanceOf(objInfo)) {
                return d;
            }
        }
    }

    //create a device
    Connectables::AudioDevice *dev = new Connectables::AudioDevice(myHost,objInfo);

	{
		QMutexLocker d(&mutexDevices);
		listAudioDevices << dev;
	}

    if(!dev->SetSleep(false)) {
        //retry later
        if(!timerRefreshDevices.isActive())
            timerRefreshDevices.start();
    }

    return dev;
}

void AudioDevices::RemoveDevice(Connectables::AudioDevice *dev)
{
	QMutexLocker d(&mutexDevices);
    listAudioDevices.removeAll(dev);
}

#ifdef CIRCULAR_BUFFER
void AudioDevices::PutPinsBuffersInRingBuffers()
{
	QMutexLocker l(&mutexClosing);
    if(closing) {
        return;
    }

	{
		QMutexLocker d(&mutexDevices);
		foreach(Connectables::AudioDevice *dev, listAudioDevices) {
			dev->PinsToRingBuffers();
		}
	}

}
#endif

/*!
  Try to find a device in the list returned by PortAudio
  \param[in] objInfo the ObjectInfo we're looking for
  \param[out] devInfo the PaDeviceInfo of the object found
  \return true if found
  */
bool AudioDevices::FindPortAudioDevice(ObjectInfo &objInfo, PaDeviceInfo *dInfo)
{
    int cptDuplicateNames=0;

    PaDeviceIndex foundSameName=-1;
    PaDeviceIndex foundSameNameId=-1;
    PaDeviceIndex foundSameNamePins=-1;
    PaDeviceIndex foundSameNamePinsId=-1;

    PaHostApiTypeId apiType = (PaHostApiTypeId)objInfo.api;
    PaHostApiIndex apiIndex = Pa_HostApiTypeIdToHostApiIndex( apiType );
    const PaHostApiInfo *apiInfo = Pa_GetHostApiInfo( apiIndex );

    if(!apiInfo)
        return false;

    for (int i=0; i<apiInfo->deviceCount; i++) {
        PaDeviceIndex devIndex = Pa_HostApiDeviceIndexToDeviceIndex( apiIndex, i);
        const PaDeviceInfo *info = Pa_GetDeviceInfo( devIndex );

        QString devName(info->name);
        //remove " x64" from device name so we can share files with 32bit version
        devName.remove(QRegularExpression("( )?x64"));

        if(devName == objInfo.name) {

            if(info->maxInputChannels == objInfo.inputs
                && info->maxOutputChannels == objInfo.outputs) {
                if(objInfo.duplicateNamesCounter == cptDuplicateNames) {
                    foundSameNamePinsId = devIndex;
                } else {
                    foundSameNamePins = devIndex;
                }
            } else {
                if(objInfo.duplicateNamesCounter == cptDuplicateNames) {
                    foundSameNameId = devIndex;
                } else {
                    foundSameName = devIndex;
                }
            }

            cptDuplicateNames++;
        }
    }


    PaDeviceIndex deviceNumber=-1;

    if(foundSameNamePinsId!=-1)
        deviceNumber = foundSameNamePinsId;
    else if(foundSameNameId!=-1)
        deviceNumber = foundSameNameId;
    else if(foundSameNamePins!=-1)
        deviceNumber = foundSameNamePins;
    else if(foundSameName!=-1)
        deviceNumber = foundSameName;
    else {
        LOG("device not found"<<objInfo.apiName<<objInfo.name);
        return false;
    }

    if(dInfo) {
        const PaDeviceInfo *i = Pa_GetDeviceInfo(deviceNumber);
        if(!i) {
            LOG("error in GetDeviceInfo");
            return false;
        }
        *dInfo = *i;
    }
    objInfo.id = deviceNumber;
    return true;
}

void AudioDevices::ConfigDevice(const ObjectInfo &info)
{
   PaDeviceIndex configDevId = (PaDeviceIndex)info.id;
   PaHostApiTypeId configApiIndex = (PaHostApiTypeId)info.api;

   //stop scanning for devices while config is opened
   if(timerRefreshDevices.isActive()) {
       timerRefreshDevices.stop();
   }

   switch(configApiIndex) {
       case paASIO: {

           SleepAll();

           PaError err = paNoError;
#if WIN32
           err = PaAsio_ShowControlPanel( configDevId, (void*)myHost->GetMainWindow() );
#endif
#ifdef __APPLE__
           err = PaAsio_ShowControlPanel( configDevId, (void*)0 );
#endif

           if( err != paNoError ) {
               QMessageBox msg(QMessageBox::Warning,
                               tr("Error"),
                               Pa_GetErrorText( err ),
                               QMessageBox::Ok,
                               myHost->GetMainWindow());
               msg.exec();
           }
           break;
       }

       case paMME: {
#ifdef WIN32
           MmeConfigDialog dlg( myHost, myHost->GetMainWindow() );
           dlg.exec();
#endif
           break;
       }

       case paWASAPI: {
#ifdef WIN32
           WasapiConfigDialog dlg( myHost, myHost->GetMainWindow() );
           dlg.exec();
#endif
           break;
       }

       case paDirectSound: {
#ifdef WIN32
           DirectxConfigDialog dlg( myHost, myHost->GetMainWindow() );
           dlg.exec();
#endif
           break;
       }

       default: {
           QMessageBox msg(QMessageBox::Information,
               tr("No config"),
               tr("No config dialog for this device"),
               QMessageBox::Ok,
               myHost->GetMainWindow());
           msg.exec();
           break;
       }
   }

   //restart devices
   SleepAll();
   OpenDevices();
   //continue scanning for new devices
   if(!timerRefreshDevices.isActive()) {
       timerRefreshDevices.start();
   }
}

//restart portaudio until a device is found
void AudioDevices::TryToOpenDevice()
{
    //the fake render loop is not running anymore, leave
   if(!fakeRenderTimer || listAudioDevices.isEmpty() || closing)
       return;

   if(paOpened)
       CloseDevices();
   OpenDevices();

   //the fake render loop is still running, retry
   if(fakeRenderTimer && listAudioDevices.isEmpty() && !timerRefreshDevices.isActive() && !closing) {
       CloseDevices();
       timerRefreshDevices.start();
   }
}
