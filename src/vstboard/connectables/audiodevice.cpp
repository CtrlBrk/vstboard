/**************************************************************************
#    Copyright 2010-2012 RaphaÃ«l FranÃ§ois
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

#include "connectables/audiodevice.h"
#include "connectables/object.h"
#include "connectables/audiodevicein.h"
#include "connectables/audiodeviceout.h"
#include "globals.h"
//#include "renderer/renderer.h"
#include "mainhosthost.h"
#include "audiodevices.h"

#include <qthread.h>

class I : public QThread
{
public:
        static void msleep(unsigned long msecs) {
                QThread::msleep(msecs);
        }
};

using namespace Connectables;

//QHash<int,QSharedPointer<AudioDevice> >AudioDevice::listAudioDevices;
QMutex AudioDevice::mutexCountInputDevicesReady;
int AudioDevice::countDevicesReady=0;
int AudioDevice::countInputDevices=0;

/*!
  \class Connectables::AudioDevice
  \brief an audio device using PortAudio. created by AudioDeviceIn or AudioDeviceOut
  */

/*!
  /param myHost the MainHost
  /param info ObjectInfo describing the device
  /param parent a parent QObject (unused ?)
  */
AudioDevice::AudioDevice(MainHostHost *myHost,const ObjectInfo &info,QObject *parent) :
    QObject(parent),
    sampleRate(44100.0f),
    bufferSize(4096),
    stream(0),
    objInfo(info),
    devIn(0),
    devOut(0),
    opened(false),
    myHost(myHost)
{
    memset(&devInfo, 0, sizeof(devInfo));

    devOutClosing=false;
    setObjectName(objInfo.name);

    connect(myHost,SIGNAL(SampleRateChanged(float)),
            this,SLOT(SetSampleRate(float)));

    connect(this,SIGNAL(InUseChanged(PaHostApiIndex,PaDeviceIndex,bool,PaTime,PaTime,double)),
            myHost->audioDevices,SLOT(OnToggleDeviceInUse(PaHostApiIndex,PaDeviceIndex,bool,PaTime,PaTime,double)));
}

AudioDevice::~AudioDevice()
{
    mutexDevicesInOut.lock();
    if(devIn) {
        devIn->SetParentDevice(0);
        devIn=0;
    }
    if(devOut) {
        devOut->SetParentDevice(0);
        devOut=0;
    }
    mutexDevicesInOut.unlock();

    LOG(objectName()<<"deleted");
}

/*!
  Close the device if not in use. Called after a timeout when the AudioDeviceIn or AudioDeviceOut are removed
  */
void AudioDevice::DeleteIfUnused()
{
    mutexOpenClose.lock();
    if(isClosing) {
        mutexOpenClose.unlock();
        return;
    }
    mutexOpenClose.unlock();

    bool del=false;
    mutexDevicesInOut.lock();
    if(!devIn && !devOut)
        del=true;
    mutexDevicesInOut.unlock();

    if(del) {
        SetSleep(true);
        myHost->audioDevices->RemoveDevice(this);
        delete this;
    }

}

/*!
  Set the input Object
  The device is closed after 2 seconds if it has no input or output object
  \param obj the AudioDeviceIn, null to unlink
  \return true on success
  */
bool AudioDevice::SetObjectInput(AudioDeviceIn *obj)
{
    QMutexLocker l(&mutexDevicesInOut);

    if(devIn && devIn == obj) {
        //it's the same object
        return true;
    }

    if(devIn && obj) {
        LOG("SetObjectInput already used");
        return false;
    }

    mutexCountInputDevicesReady.lock();
    if(obj) {
        countInputDevices++;
    } else {
        countInputDevices--;
    }
    mutexCountInputDevicesReady.unlock();

    devIn = obj;

    if(!obj) {
        QTimer::singleShot(2000,this,SLOT(DeleteIfUnused()));
    }

    return true;
}


/*!
  Set or remove the output Object.
  The device is closed after 2 seconds if it has no input or output object
  \param obj the AudioDeviceOut, null to unlink
  \return true on success
  */
bool AudioDevice::SetObjectOutput(AudioDeviceOut *obj)
{
    QMutexLocker l(&mutexDevicesInOut);

    if(devOut && devOut == obj) {
        //it's the same object
        return true;
    }

    devOutClosing=false;

    if(devOut && obj) {
        LOG("SetObjectOutput already used");
        return false;
    }

    devOut = obj;

    if(!obj) {
        QTimer::singleShot(2000,this,SLOT(DeleteIfUnused()));
        devOutClosing=true;
    }

    return true;
}

/*!
  Reopen the device with the new sample rate
  \param rate the new rate
  */
void AudioDevice::SetSampleRate(float rate)
{
    mutexOpenClose.lock();
    if(opened) {
        mutexOpenClose.unlock();
        SetSleep(true);
        SetSleep(false);
    } else {
        mutexOpenClose.unlock();
    }
}


/*!
  Open the PortAudio stream, used by AudioDevice::Open
  \param sampleRate the desired sample rate
  \return true on success
  */
bool AudioDevice::OpenStream(double sampleRate)
{
//    if(Pa_GetDeviceInfo(objInfo.id)==0) {
//        errorMessage=tr("Device not found");
//        LOG(errorMessage)
//        return false;
//    }

    unsigned long framesPerBuffer = paFramesPerBufferUnspecified;

    PaStreamParameters *inputParameters = NULL;
    PaStreamParameters *outputParameters = NULL;
    PaStreamFlags flags = paClipOff; //paNoFlag;

    if(devInfo.maxInputChannels > 0) {

        inputParameters = new PaStreamParameters;
        ZeroMemory( inputParameters, sizeof( PaStreamParameters ) );
        inputParameters->channelCount = devInfo.maxInputChannels;
        inputParameters->device = objInfo.id;
        inputParameters->hostApiSpecificStreamInfo = NULL;
        inputParameters->sampleFormat = paFloat32 | paNonInterleaved;
        inputParameters->suggestedLatency = Pa_GetDeviceInfo(objInfo.id)->defaultLowInputLatency;

        switch(Pa_GetHostApiInfo( devInfo.hostApi )->type) {
            case paDirectSound :
                ZeroMemory( &directSoundStreamInfo, sizeof( PaWinDirectSoundStreamInfo) );
                directSoundStreamInfo.size = sizeof( PaWinDirectSoundStreamInfo);
                directSoundStreamInfo.hostApiType = paDirectSound;
                directSoundStreamInfo.version = 2;
                //directSoundStreamInfo.flags = paWinDirectSoundUseChannelMask;
                //directSoundStreamInfo.channelMask = PAWIN_SPEAKER_5POINT1; /* request 5.1 output format */
                inputParameters->hostApiSpecificStreamInfo = &directSoundStreamInfo;
                break;
            case paMME :
                ZeroMemory( &wmmeStreamInfo, sizeof(PaWinMmeStreamInfo) );
                wmmeStreamInfo.size = sizeof(PaWinMmeStreamInfo);
                wmmeStreamInfo.hostApiType = paMME;
                wmmeStreamInfo.version = 1;
                wmmeStreamInfo.flags = myHost->settings->GetSetting("api/wmme_flags", MME_DFAULT_FLAGS).toUInt();
                wmmeStreamInfo.framesPerBuffer = myHost->settings->GetSetting("api/wmme_bufferSize", MME_DEFAULT_BUFFER_SIZE).toUInt();
                wmmeStreamInfo.bufferCount = myHost->settings->GetSetting("api/wmme_bufferCount", MME_DEFAULT_BUFFER_COUNT).toUInt();
                inputParameters->hostApiSpecificStreamInfo = &wmmeStreamInfo;
                break;
            case paASIO :
                break;
            case paSoundManager :
                break;
            case paCoreAudio :
                break;
            case paOSS :
                break;
            case paALSA :
                break;
            case paAL :
                break;
            case paBeOS :
                break;
            case paWDMKS :
                break;
            case paJACK :
                break;
            case paWASAPI : {
                ZeroMemory( &wasapiStreamInfo, sizeof(PaWasapiStreamInfo) );
                wasapiStreamInfo.size = sizeof(PaWasapiStreamInfo);
                wasapiStreamInfo.hostApiType = paWASAPI;
                wasapiStreamInfo.version = 1;
                wasapiStreamInfo.flags = myHost->settings->GetSetting("api/wasapi_flags", WASAPI_DEFAULT_FLAGS).toUInt();
                inputParameters->hostApiSpecificStreamInfo = &wasapiStreamInfo;

                unsigned int lat = myHost->settings->GetSetting("api/wasapi_inLatency", WASAPI_DEFAULT_INLATENCY).toUInt();
                if(lat!=0)
                    inputParameters->suggestedLatency = (PaTime)lat/1000;
                break;
            }
            case paAudioScienceHPI :
                break;
            default:
                break;
        }
    }

    if(devInfo.maxOutputChannels > 0) {

        outputParameters = new PaStreamParameters;
        ZeroMemory( outputParameters, sizeof( PaStreamParameters ) );
        outputParameters->channelCount = devInfo.maxOutputChannels;
        outputParameters->device = objInfo.id;
        outputParameters->hostApiSpecificStreamInfo = NULL;
        outputParameters->sampleFormat = paFloat32 | paNonInterleaved;
        outputParameters->suggestedLatency = Pa_GetDeviceInfo(objInfo.id)->defaultLowOutputLatency ;

        switch(Pa_GetHostApiInfo( devInfo.hostApi )->type) {
            case paDirectSound :
                ZeroMemory( &directSoundStreamInfo, sizeof(PaWinDirectSoundStreamInfo) );
                directSoundStreamInfo.size = sizeof(PaWinDirectSoundStreamInfo);
                directSoundStreamInfo.hostApiType = paDirectSound;
                directSoundStreamInfo.version = 2;
                //directSoundStreamInfo.flags = paWinDirectSoundUseChannelMask;
                //directSoundStreamInfo.channelMask = PAWIN_SPEAKER_5POINT1; /* request 5.1 output format */
                outputParameters->hostApiSpecificStreamInfo = &directSoundStreamInfo;
                break;
            case paMME :
                ZeroMemory( &wmmeStreamInfo, sizeof(PaWinMmeStreamInfo) );
                wmmeStreamInfo.size = sizeof(PaWinMmeStreamInfo);
                wmmeStreamInfo.hostApiType = paMME;
                wmmeStreamInfo.version = 1;
                wmmeStreamInfo.flags = myHost->settings->GetSetting("api/wmme_flags", MME_DFAULT_FLAGS).toUInt();
                wmmeStreamInfo.framesPerBuffer = myHost->settings->GetSetting("api/wmme_bufferSize", MME_DEFAULT_BUFFER_SIZE).toUInt();
                wmmeStreamInfo.bufferCount = myHost->settings->GetSetting("api/wmme_bufferCount", MME_DEFAULT_BUFFER_COUNT).toUInt();
                outputParameters->hostApiSpecificStreamInfo = &wmmeStreamInfo;
                break;
            case paASIO :
                break;
            case paSoundManager :
                break;
            case paCoreAudio :
                break;
            case paOSS :
                break;
            case paALSA :
                break;
            case paAL :
                break;
            case paBeOS :
                break;
            case paWDMKS :
                break;
            case paJACK :
                break;
            case paWASAPI : {
                ZeroMemory( &wasapiStreamInfo, sizeof(PaWasapiStreamInfo) );
                wasapiStreamInfo.size = sizeof(PaWasapiStreamInfo);
                wasapiStreamInfo.hostApiType = paWASAPI;
                wasapiStreamInfo.version = 1;
                wasapiStreamInfo.flags = myHost->settings->GetSetting("api/wasapi_flags", WASAPI_DEFAULT_FLAGS).toUInt();
                outputParameters->hostApiSpecificStreamInfo = &wasapiStreamInfo;

                unsigned int lat = myHost->settings->GetSetting("api/wasapi_outLatency", WASAPI_DEFAULT_OUTLATENCY).toUInt();
                if(lat!=0)
                    outputParameters->suggestedLatency = (PaTime)lat/1000;
                break;
            }
            case paAudioScienceHPI :
                break;
            default :
                break;
        }
    }

    if(Pa_IsFormatSupported( inputParameters, outputParameters, sampleRate ) != paFormatIsSupported) {
        SetErrorMsg( tr("Stream format not supported") );
        if(inputParameters)
            delete inputParameters;
        if(outputParameters)
            delete outputParameters;
        return false;
    }

    PaError err = Pa_OpenStream(
            &stream,
            inputParameters,
            outputParameters,
            sampleRate,
            framesPerBuffer,
            flags,
            paCallback,
            (void *)this );

    if( err != paNoError ) {
        Pa_CloseStream(stream);
        SetErrorMsg( Pa_GetErrorText( err ) );
        if(inputParameters)
            delete inputParameters;
        if(outputParameters)
            delete outputParameters;
        return false;
    }

    LOG("Open"<<objectName())
    emit InUseChanged(objInfo.api, objInfo.id, true, inputParameters?inputParameters->suggestedLatency:0, outputParameters?outputParameters->suggestedLatency:0);

//    err = Pa_SetStreamFinishedCallback( stream, &paStreamFinished );
//    if( err != paNoError ) {
//        Pa_CloseStream(stream);
//        debug("AudioDevice::OpenStream Pa_SetStreamFinishedCallback %s",Pa_GetErrorText( err ))
//        errorMessage = Pa_GetErrorText( err );
//        if(inputParameters)
//            delete inputParameters;
//        if(outputParameters)
//            delete outputParameters;
//        return false;
//    }

//    const PaStreamInfo *inf = Pa_GetStreamInfo(&stream);
//    if(inf)
//        emit InUseChanged(objInfo.api,objInfo.id,true,inf->inputLatency,inf->outputLatency,inf->sampleRate);
//    else
//        emit InUseChanged(objInfo.api,objInfo.id,true);


    if(inputParameters)
        delete inputParameters;
    if(outputParameters)
        delete outputParameters;
    return true;
}

/*!
  Open the device
  \return true on success
  */
bool AudioDevice::Open()
{
    //already opened
    mutexOpenClose.lock();
    if(opened) {
        mutexOpenClose.unlock();
        LOG("already opened");
        return true;
    }
    isClosing=false;
    mutexOpenClose.unlock();

    SetErrorMsg("");

    //get portaudio infos
    if(!myHost->audioDevices->FindPortAudioDevice(objInfo, &devInfo)) {
        SetErrorMsg( tr("Device not found") );
        return false;
    }

    //try to open at the host rate
    double sampleRate = myHost->GetSampleRate();
    if(!OpenStream(sampleRate)) {

        //if it fails, try to open with the default rate
        sampleRate = devInfo.defaultSampleRate;
        if(!OpenStream(sampleRate)) {
            SetErrorMsg( tr("Sample format not supported") );
            return false;
        }

        //opened with the default rate, change the host rate
        //MainHost::Get()->SetSampleRate(sampleRate);
    }

    //start the stream
    PaError err = Pa_StartStream( stream );

    //failed to open the stream
    if( err != paNoError ) {

        if(stream)
        {
            Pa_StopStream(stream);
            Pa_CloseStream(stream);
            stream = 0;
        }

        SetErrorMsg( Pa_GetErrorText( err ) );
        return false;
    }

#ifdef CIRCULAR_BUFFER
    CreateCircularBuffers();
#endif

    mutexOpenClose.lock();
    opened=true;
    mutexOpenClose.unlock();
    return true;
}

/*!
  Close the PortAudio stream, used by AudioDevice::Close and AudioDevice::SetSleep
  \return true on success
  */
bool AudioDevice::Close()
{
    mutexOpenClose.lock();
    if(isClosing) {
        mutexOpenClose.unlock();
        return false;
    }
    isClosing=true;
    opened=false;
    mutexOpenClose.unlock();

    LOG("Close"<<objectName())
    emit InUseChanged( objInfo.api,objInfo.id,false);

    if(stream)
    {
        Pa_StopStream(stream);
        int count=0;
        while(Pa_IsStreamActive(stream) && count<200) {
            I::msleep(10);
            count++;
        }
        if(count>0) {
            LOG("AudioDevice::Close"<<objectName()<<"wait:"<<count);
        }

        Pa_CloseStream(stream);
        stream = 0;
    }
#ifdef CIRCULAR_BUFFER
    DeleteCircularBuffers();
#endif
    return true;
}

#ifdef CIRCULAR_BUFFER
void AudioDevice::CreateCircularBuffers()
{
    for(int i=0; i<devInfo.maxInputChannels; i++ )
        listCircularBuffersIn << new CircularBuffer();
    for(int i=0; i<devInfo.maxOutputChannels; i++ )
        listCircularBuffersOut << new CircularBuffer();
}

/*!
  Delete ring buffers, used by AudioDevice::CloseStream
  */
void AudioDevice::DeleteCircularBuffers()
{
    foreach(CircularBuffer *buf, listCircularBuffersIn)
        delete buf;
    listCircularBuffersIn.clear();

    foreach(CircularBuffer *buf, listCircularBuffersOut)
        delete buf;
    listCircularBuffersOut.clear();

}
#endif

/*!
  Set the sleep state
  \param sleeping the new state
  */
void AudioDevice::SetSleep(bool sleeping)
{
    if(!sleeping)
        Open();

//    mutexDevicesInOut.lock();
//    if(devIn)
//        devIn->SetSleep( (!opened || sleeping) );
//    if(devOut)
//        devOut->SetSleep( (!opened || sleeping) );
//    mutexDevicesInOut.unlock();

    if(sleeping)
        Close();
}

/*!
  Get the cpu usage returned by PortAudio
  \return cpu usage
  */
float AudioDevice::GetCpuUsage()
{
   return Pa_GetStreamCpuLoad(stream);
}

#ifdef CIRCULAR_BUFFER
bool AudioDevice::DeviceToRingBuffers( const void *inputBuffer, unsigned long framesPerBuffer)
{

    mutexOpenClose.lock();
    if(isClosing) {
        mutexOpenClose.unlock();
        return false;
    }
    mutexOpenClose.unlock();

    unsigned long hostBuffSize = myHost->GetBufferSize();
    if(framesPerBuffer > hostBuffSize) {
       myHost->SetBufferSize(framesPerBuffer);
       hostBuffSize = framesPerBuffer;
    }

    mutexDevicesInOut.lock();
    if(!devIn) {
        mutexDevicesInOut.unlock();
        return true;
    }
    mutexDevicesInOut.unlock();

    bool readyToRender=true;

    //fill circular buffer with device audio
    int cpt=0;
    foreach(CircularBuffer *buf, listCircularBuffersIn) {
        if(buf->buffSize<myHost->GetBufferSize()*2)
            buf->SetSize(myHost->GetBufferSize()*2);

        buf->Put( ((float **) inputBuffer)[cpt], framesPerBuffer );
        if(buf->filledSize < hostBuffSize )
            readyToRender=false;
        cpt++;
    }

    //if we filled enough buffer
    if(readyToRender)
        RingBuffersToPins();

    return true;
}

void AudioDevice::RingBuffersToPins()
{
    mutexDevicesInOut.lock();
    if(devIn)
        devIn->SetBufferFromRingBuffer(listCircularBuffersIn);
    mutexDevicesInOut.unlock();

    if(!inputBufferReady) {
        inputBufferReady=true;

        QMutexLocker locker(&mutexCountInputDevicesReady);
        countDevicesReady++;
    }
}

void AudioDevice::PinsToRingBuffers()
{
    mutexOpenClose.lock();
    if(isClosing) {
        mutexOpenClose.unlock();
        return;
    }
    mutexOpenClose.unlock();

    mutexDevicesInOut.lock();
    if(devOut)
        devOut->SetRingBufferFromPins(listCircularBuffersOut);
    mutexDevicesInOut.unlock();

    inputBufferReady=false;
}

bool AudioDevice::RingBuffersToDevice( void *outputBuffer, unsigned long framesPerBuffer)
{
    mutexOpenClose.lock();
    if(isClosing) {
        mutexOpenClose.unlock();
        return false;
    }
    mutexOpenClose.unlock();

    mutexDevicesInOut.lock();
    if(devOutClosing) {
        //the device was removed : clear the output buffer one time
        devOutClosing=false;
        mutexDevicesInOut.unlock();

        int cpt=0;
        foreach(CircularBuffer *buf, listCircularBuffersOut) {
            //empty the circular buffer, in case we reopen this device
            buf->Clear();
            //send a blank buffer to the device
            ZeroMemory( ((float **) outputBuffer)[cpt], sizeof(float)*framesPerBuffer );
            cpt++;
        }
        return true;

    }
    mutexDevicesInOut.unlock();

    //send circular buffer to device if there's enough data
    int cpt=0;
    foreach(CircularBuffer *buf, listCircularBuffersOut) {
        if(buf->filledSize>=framesPerBuffer)
            buf->Get( ((float **) outputBuffer)[cpt], framesPerBuffer );
        cpt++;
    }

    return true;
}
#else
bool AudioDevice::DeviceToPinBuffers( const void *inputBuffer, unsigned long framesPerBuffer )
{
    unsigned long hostBuffSize = myHost->GetBufferSize();
    if(framesPerBuffer > hostBuffSize) {
       myHost->SetBufferSize(framesPerBuffer);
       hostBuffSize = framesPerBuffer;
    }

//    mutexDevicesInOut.lock();
    if(!devIn) {
//        mutexDevicesInOut.unlock();
        return true;
    }

    if(!devIn)
        return true;
    PinsList *lst = devIn->GetListAudioPinOut();
    if(!lst)
        return true;

    for(int cpt=0; cpt<lst->nbPins(); cpt++) {
        AudioBuffer *pinBuf = lst->GetBuffer(cpt);
        if(pinBuf) {
            pinBuf->SetBufferContent( ((float **) inputBuffer)[cpt], framesPerBuffer);
        }
    }

//    mutexDevicesInOut.unlock();

    return true;
}

bool AudioDevice::PinBuffersToDevice( void *outputBuffer, unsigned long framesPerBuffer )
{
//    mutexDevicesInOut.lock();
    if(devOutClosing) {
        devOutClosing=false;
//        mutexDevicesInOut.unlock();
        return true;
    }
//    mutexDevicesInOut.unlock();

    if(!devOut)
        return true;
    PinsList *lst = devOut->GetListAudioPinIn();
    if(!lst)
        return true;

    for(int cpt=0; cpt<lst->nbPins(); cpt++) {
        AudioBuffer *pinBuf = lst->GetBuffer(cpt);
        if(pinBuf) {
            pinBuf->ConsumeStack();
            pinBuf->DumpToBuffer( ((float **) outputBuffer)[cpt], framesPerBuffer);
            pinBuf->ResetStackCounter();
        }
    }

    return true;
}
#endif

/*!
  PortAudio callback
  put the audio provided by PortAudio in ring buffers
  starts a rendering loop when we have enough data
  send the resulting buffers back to PortAudio
  */
int AudioDevice::paCallback( const void *inputBuffer, void *outputBuffer,
                                 unsigned long framesPerBuffer,
                                 const PaStreamCallbackTimeInfo* /*timeInfo*/,
                                 PaStreamCallbackFlags /*statusFlags*/,
                                 void *userData )
{
    AudioDevice* device = (AudioDevice*)userData;

#ifdef CIRCULAR_BUFFER
    if(!device->DeviceToRingBuffers(inputBuffer, framesPerBuffer))
        return paComplete;

    //all devices are ready : render
    mutexCountInputDevicesReady.lock();
    if(countDevicesReady>=countInputDevices) {
        countDevicesReady=0;
        mutexCountInputDevicesReady.unlock();

        device->myHost->Render();
        device->myHost->audioDevices->PutPinsBuffersInRingBuffers();
    } else {
        mutexCountInputDevicesReady.unlock();
    }

    if(!device->RingBuffersToDevice(outputBuffer, framesPerBuffer))
        return paComplete;
#else
    QMutexLocker l(&device->mutexDevicesInOut);
    device->mutexOpenClose.lock();
    if(device->isClosing) {
        device->mutexOpenClose.unlock();
        return paComplete;
    }
    device->mutexOpenClose.unlock();


////    if(device->devOut) {
//        device->myHost->SetCurrentBuffers((float **)inputBuffer, (float **)outputBuffer, framesPerBuffer);
//    }

//    QMutexLocker l(&device->mutexOutReady);

//    device->currentInputBuffer=(float**)inputBuffer;
//    device->currentOutputBuffer=(float**)outputBuffer;
//    device->bufferSize=framesPerBuffer;



//    if(device->devIn)
//        device->devIn->NewRenderLoop2();



    if(!device->DeviceToPinBuffers(inputBuffer,framesPerBuffer))
        return paComplete;


    device->myHost->Render();
    if(!device->PinBuffersToDevice(outputBuffer,framesPerBuffer))
        return paComplete;
//    if(device->devOut)
//        device->devOut->NewRenderLoop2();

//    device->currentInputBuffer=0;
//    device->currentOutputBuffer=0;
//    if(device->devOut)
//        device->condBufferOutReady.wait(l.mutex(), 100);

//    if(!device->OutBufferIsReady()) {
//        LOG("buffer not ready")
//        return paContinue;
//    }
//    device->SetOutputBufferReady(false);


#endif
    return paContinue;
}

bool AudioDevice::IsAnInstanceOf(const ObjectInfo &info)
{
    if(objInfo.api != info.api)
        return false;

    if(objInfo.name != info.name)
        return false;

    if(objInfo.duplicateNamesCounter != info.duplicateNamesCounter)
        return false;

    return true;
}

void AudioDevice::SetErrorMsg(const QString &msg)
{
#ifndef QT_NO_DEBUG
    if(!msg.isEmpty()) {
        LOG(msg)
    }
#endif

    errorMessage = msg;

    if(devIn)
        devIn->SetErrorMessage(msg);
    if(devOut)
        devOut->SetErrorMessage(msg);
}
