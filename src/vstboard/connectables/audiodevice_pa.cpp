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

#include "connectables/audiodevice_pa.h"
// #include "connectables/object.h"
#include "connectables/audiodevicein.h"
#include "connectables/audiodeviceout.h"
#include "globals.h"
//#include "renderer/renderer.h"
#include "mainhosthost.h"
#include "audiodevices_pa.h"

#include <qthread.h>

#include "mainwindow.h"

class I : public QThread
{
public:
        static void msleep(unsigned long msecs) {
                QThread::msleep(msecs);
        }
};

using namespace Connectables;

//QHash<int,QSharedPointer<AudioDevice> >AudioDevice::listAudioDevices;
QMutex AudioDevice::mutexCountOpenedDevicesReady;
int AudioDevice::countDevicesReady=0;
int AudioDevice::countOpenedDevices=0;

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
    bufferSize(4096),
    sampleRate(44100.0f),
    stream(0),
    objInfo(info),
    devIn(0),
    devOut(0),
    opened(false),
    myHost(myHost),
    pause(false)
{
   memset(&devInfo, 0, sizeof(devInfo));

    devOutClosing=false;
    setObjectName(objInfo.name);

    connect(myHost,SIGNAL(SampleRateChanged(float)),
            this,SLOT(SetSampleRate(float)));

    connect(this,SIGNAL(InUseChanged(PaHostApiIndex,PaDeviceIndex,bool,PaTime,PaTime,double)),
           myHost->audioDevices,SLOT(OnToggleDeviceInUse(PaHostApiIndex,PaDeviceIndex,bool,PaTime,PaTime,double)));
/*
    connect(this,SIGNAL(DebugGraphUpdated(QVector<float>)),
            myHost->GetMainWindow(),SLOT(UpdateDebugGraph(QVector<float>)));
    connect(myHost->GetMainWindow(),SIGNAL(PauseOutput(bool)),
            this,SLOT(PauseOutput(bool)));
*/
}


AudioDevice::~AudioDevice()
{
    QMutexLocker lio(&mutexDevicesInOut);
    if(devIn) {
        devIn->SetParentDevice(0);
        devIn=0;
    }
    if(devOut) {
        devOut->SetParentDevice(0);
        devOut=0;
    }
#ifdef DEBUG_DEVICES
    LOG(objectName()<<"deleted");
#endif
}


void AudioDevice::PauseOutput(bool p) {
    pause=p;
}

/*!
  Close the device if not in use. Called after a timeout when the AudioDeviceIn or AudioDeviceOut are removed
  */
void AudioDevice::DeleteIfUnused()
{
    {
        QMutexLocker lo(&mutexOpenClose);
        if(isClosing) {
            return;
        }
    }

    bool del=false;

    {
        QMutexLocker lio(&mutexDevicesInOut);
        if(!devIn && !devOut)
            del=true;
    }

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
#ifdef DEBUG_DEVICES
        LOG("SetObjectInput already used");
#endif
        return false;
    }

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
#ifdef DEBUG_DEVICES
        LOG("SetObjectOutput already used");
#endif
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
void AudioDevice::SetSampleRate(float /*rate*/)
{
    {
        QMutexLocker lo(&mutexOpenClose);
        if(!opened) return;
    }

    SetSleep(true);
    SetSleep(false);
}

/*!
  Open the PortAudio stream, used by AudioDevice::Open
  \param sampleRate the desired sample rate
  \return true on success
  */
bool AudioDevice::OpenStream(double sampleRate)
{
    unsigned long framesPerBuffer = paFramesPerBufferUnspecified;

    PaStreamParameters *inputParameters = NULL;
    PaStreamParameters *outputParameters = NULL;
    PaStreamFlags flags = paClipOff; //paNoFlag;

    if(devInfo.maxInputChannels > 0) {

        inputParameters = new PaStreamParameters;
#ifdef WIN32
        ZeroMemory( inputParameters, sizeof( PaStreamParameters ) );
#else
        memset(inputParameters,0,sizeof( PaStreamParameters ) );
#endif

        inputParameters->channelCount = devInfo.maxInputChannels;
        inputParameters->device = objInfo.id;
        inputParameters->hostApiSpecificStreamInfo = NULL;
        inputParameters->sampleFormat = paFloat32 | paNonInterleaved;
        inputParameters->suggestedLatency = Pa_GetDeviceInfo(objInfo.id)->defaultLowInputLatency;

        switch(Pa_GetHostApiInfo( devInfo.hostApi )->type) {
        case paDirectSound :
#ifdef WIN32
            ZeroMemory( &directSoundStreamInfo, sizeof( PaWinDirectSoundStreamInfo) );
            directSoundStreamInfo.size = sizeof( PaWinDirectSoundStreamInfo);
            directSoundStreamInfo.hostApiType = paDirectSound;
            directSoundStreamInfo.version = 2;
            //directSoundStreamInfo.flags = paWinDirectSoundUseChannelMask;
            //directSoundStreamInfo.channelMask = PAWIN_SPEAKER_5POINT1; /* request 5.1 output format */
            directSoundStreamInfo.flags = paWinDirectSoundUseLowLevelLatencyParameters;
            directSoundStreamInfo.framesPerBuffer = myHost->settings->GetSetting("api/dx_bufferSize", DIRECTX_DEFAULT_BUFFER_SIZE).toUInt();
            inputParameters->hostApiSpecificStreamInfo = &directSoundStreamInfo;
            myHost->SetBufferSize(directSoundStreamInfo.framesPerBuffer);
#endif
            break;
        case paMME :
#ifdef WIN32
            ZeroMemory( &wmmeStreamInfo, sizeof(PaWinMmeStreamInfo) );
            wmmeStreamInfo.size = sizeof(PaWinMmeStreamInfo);
            wmmeStreamInfo.hostApiType = paMME;
            wmmeStreamInfo.version = 1;
            wmmeStreamInfo.flags = myHost->settings->GetSetting("api/wmme_flags", MME_DFAULT_FLAGS).toUInt();
            wmmeStreamInfo.framesPerBuffer = myHost->settings->GetSetting("api/wmme_bufferSize", MME_DEFAULT_BUFFER_SIZE).toUInt();
            wmmeStreamInfo.bufferCount = myHost->settings->GetSetting("api/wmme_bufferCount", MME_DEFAULT_BUFFER_COUNT).toUInt();
            inputParameters->hostApiSpecificStreamInfo = &wmmeStreamInfo;
            myHost->SetBufferSize(wmmeStreamInfo.framesPerBuffer);
#endif
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
#ifdef WIN32
            ZeroMemory( &wasapiStreamInfo, sizeof(PaWasapiStreamInfo) );
            wasapiStreamInfo.size = sizeof(PaWasapiStreamInfo);
            wasapiStreamInfo.hostApiType = paWASAPI;
            wasapiStreamInfo.version = 1;
            wasapiStreamInfo.flags = myHost->settings->GetSetting("api/wasapi_flags", WASAPI_DEFAULT_FLAGS).toUInt();
            inputParameters->hostApiSpecificStreamInfo = &wasapiStreamInfo;

            unsigned int lat = myHost->settings->GetSetting("api/wasapi_inLatency", WASAPI_DEFAULT_INLATENCY).toUInt();
            if(lat!=0)
                inputParameters->suggestedLatency = (PaTime)lat/1000;
#endif
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
#ifdef WIN32
        ZeroMemory( outputParameters, sizeof( PaStreamParameters ) );
#else
        memset(outputParameters,0,sizeof( PaStreamParameters ) );
#endif
        outputParameters->channelCount = devInfo.maxOutputChannels;
        outputParameters->device = objInfo.id;
        outputParameters->hostApiSpecificStreamInfo = NULL;
        outputParameters->sampleFormat = paFloat32 | paNonInterleaved;
        outputParameters->suggestedLatency = Pa_GetDeviceInfo(objInfo.id)->defaultLowOutputLatency ;

        switch(Pa_GetHostApiInfo( devInfo.hostApi )->type) {
        case paDirectSound :
#ifdef WIN32
            ZeroMemory( &directSoundStreamInfo, sizeof(PaWinDirectSoundStreamInfo) );
            directSoundStreamInfo.size = sizeof(PaWinDirectSoundStreamInfo);
            directSoundStreamInfo.hostApiType = paDirectSound;
            directSoundStreamInfo.version = 2;
            //directSoundStreamInfo.flags = paWinDirectSoundUseChannelMask;
            //directSoundStreamInfo.channelMask = PAWIN_SPEAKER_5POINT1; /* request 5.1 output format */
            //directSoundStreamInfo.flags = paWinDirectSoundUseLowLevelLatencyParameters;
            directSoundStreamInfo.flags = myHost->settings->GetSetting("api/dx_flags", DIRECTX_DEFAULT_BUFFER_SIZE).toUInt();
            directSoundStreamInfo.framesPerBuffer = myHost->settings->GetSetting("api/dx_bufferSize", DIRECTX_DEFAULT_BUFFER_SIZE).toUInt();
            outputParameters->hostApiSpecificStreamInfo = &directSoundStreamInfo;
#endif
            break;
        case paMME :
#ifdef WIN32
            ZeroMemory( &wmmeStreamInfo, sizeof(PaWinMmeStreamInfo) );
            wmmeStreamInfo.size = sizeof(PaWinMmeStreamInfo);
            wmmeStreamInfo.hostApiType = paMME;
            wmmeStreamInfo.version = 1;
            wmmeStreamInfo.flags = myHost->settings->GetSetting("api/wmme_flags", MME_DFAULT_FLAGS).toUInt();
            wmmeStreamInfo.framesPerBuffer = myHost->settings->GetSetting("api/wmme_bufferSize", MME_DEFAULT_BUFFER_SIZE).toUInt();
            wmmeStreamInfo.bufferCount = myHost->settings->GetSetting("api/wmme_bufferCount", MME_DEFAULT_BUFFER_COUNT).toUInt();
            outputParameters->hostApiSpecificStreamInfo = &wmmeStreamInfo;
#endif
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
#ifdef WIN32
            ZeroMemory( &wasapiStreamInfo, sizeof(PaWasapiStreamInfo) );
            wasapiStreamInfo.size = sizeof(PaWasapiStreamInfo);
            wasapiStreamInfo.hostApiType = paWASAPI;
            wasapiStreamInfo.version = 1;
            wasapiStreamInfo.flags = myHost->settings->GetSetting("api/wasapi_flags", WASAPI_DEFAULT_FLAGS).toUInt();
            outputParameters->hostApiSpecificStreamInfo = &wasapiStreamInfo;

            unsigned int lat = myHost->settings->GetSetting("api/wasapi_outLatency", WASAPI_DEFAULT_OUTLATENCY).toUInt();
            if(lat!=0)
                outputParameters->suggestedLatency = (PaTime)lat/1000;
#endif
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

#ifdef DEBUG_DEVICES
    LOG("Open"<<objectName())
#endif
    emit InUseChanged(objInfo.api, objInfo.id, true, inputParameters?inputParameters->suggestedLatency:0, outputParameters?outputParameters->suggestedLatency:0);

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
    {
        QMutexLocker lo(&mutexOpenClose);
        if(opened) {
#ifdef DEBUG_DEVICES
            LOG("already opened");
#endif
            return true;
        }
        isClosing=false;
    }

    {
        QMutexLocker l(&mutexCountOpenedDevicesReady);
        countOpenedDevices++;
    }

    SetErrorMsg("");

    //get portaudio infos
    if(!myHost->audioDevices->FindPortAudioDevice(objInfo, &devInfo)) {
        SetErrorMsg( tr("Device not found") );
        return false;
    }


#ifdef CIRCULAR_BUFFER
    CreateCircularBuffers();
#endif

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



    {
        QMutexLocker lo(&mutexOpenClose);
        opened=true;
    }
    return true;
}
/*!
  Close the PortAudio stream, used by AudioDevice::Close and AudioDevice::SetSleep
  \return true on success
  */
bool AudioDevice::Close()
{
    {
        QMutexLocker lo(&mutexOpenClose);
        if(isClosing) return false;

        isClosing=true;
        opened=false;
    }
    
    {
        QMutexLocker l(&mutexCountOpenedDevicesReady);
        countOpenedDevices--;
    }

    emit InUseChanged( objInfo.api,objInfo.id,false);


#ifdef DEBUG_DEVICES
    LOG("Close"<<objectName())
#endif

    if(stream)
    {
        Pa_StopStream(stream);
        int count=0;
        while(Pa_IsStreamActive(stream) && count<200) {
            I::msleep(10);
            count++;
        }
        if(count>0) {
#ifdef DEBUG_DEVICES
            LOG("AudioDevice::Close"<<objectName()<<"wait:"<<count);
#endif
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
   for(int i=0; i<devInfo.maxInputChannels; i++ ) {
       listCircularBuffersIn << new CircularBuffer();
   }
   for(int i=0; i<devInfo.maxOutputChannels; i++ ) {
       listCircularBuffersOut << new CircularBuffer();
   }

//    const ulong bsize=4000;
//    float buff[bsize];
//    memset(buff,.0f,bsize);
//    foreach(CircularBuffer *buf, listCircularBuffersOut) {
//        if(buf) {
//            buf->Put( buff, bsize );
//        }
//    }
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
bool AudioDevice::SetSleep(bool sleeping)
{
    if(!sleeping)
        return Open();

//    mutexDevicesInOut.lock();
//    if(devIn)
//        devIn->SetSleep( (!opened || sleeping) );
//    if(devOut)
//        devOut->SetSleep( (!opened || sleeping) );
//    mutexDevicesInOut.unlock();

    else
        return Close();
}

/*!
  Get the cpu usage returned by PortAudio
  \return cpu usage
  */
float AudioDevice::GetCpuUsage()
{
  return Pa_GetStreamCpuLoad(stream);
   return .0f;
}

#ifdef CIRCULAR_BUFFER
bool AudioDevice::DeviceToRingBuffers( const void *inputBuffer, qint32 framesPerBuffer)
{
    //if no input, render when the host buffer is full
    static qint32 fakeFilledSize=0;

    {
        QMutexLocker lo(&mutexOpenClose);
        if(isClosing) return false;
    }

    qint32 hostBuffSize = myHost->GetBufferSize();
    if(framesPerBuffer > hostBuffSize) {
#ifdef DEBUG_DEVICES
        LOG(QString("ajusting host buffer %1->%2")
            .arg(hostBuffSize)
            .arg(framesPerBuffer));
#endif
       myHost->SetBufferSize(framesPerBuffer);
       hostBuffSize = framesPerBuffer;
    }

    {
        QMutexLocker lio(&mutexDevicesInOut);
        if(!devIn) {
            fakeFilledSize+=framesPerBuffer;
            if(fakeFilledSize>=hostBuffSize) {
                inputBufferReady=true;
                fakeFilledSize=0;
            } else {
                inputBufferReady=false;
            }
            return true;
        }
    }

    inputBufferReady=true;

    //fill circular buffer with device audio
     int cpt=0;
    //float* inData = (float *) inputBuffer;
    float* inData;
    foreach(CircularBuffer *buf, listCircularBuffersIn) {
        inData = ((float **) inputBuffer)[cpt];

        if(buf->buffSize<myHost->GetBufferSize()*2)
            buf->SetSize(myHost->GetBufferSize()*2);

        buf->Put( inData, framesPerBuffer );
        if(buf->filledSize < hostBuffSize ) {
            inputBufferReady=false;
        } /*else {
#ifdef DEBUG_DEVICES
            LOG(QString("not enough data host buffer %1/%2")
#endif
                .arg(buf->filledSize)
                .arg(hostBuffSize));
        }*/
        //inData+=framesPerBuffer;
        cpt++;
    }

    //if we filled enough buffer
    if(inputBufferReady)
        RingBuffersToPins();

    return true;
}

void AudioDevice::RingBuffersToPins()
{
    {
        QMutexLocker lio(&mutexDevicesInOut);
        if(devIn)
            devIn->SetBufferFromRingBuffer(listCircularBuffersIn);
    }

//    if(!inputBufferReady) {
//        inputBufferReady=true;

//        QMutexLocker locker(&mutexCountInputDevicesReady);
//        countDevicesReady++;
//    }
}

void AudioDevice::PinsToRingBuffers()
{
    {
        QMutexLocker lo(&mutexOpenClose);
        if(isClosing) return;
    }

    {
        QMutexLocker lio(&mutexDevicesInOut);
        if(devOut)
            devOut->SetRingBufferFromPins(listCircularBuffersOut);
    }

}

bool AudioDevice::RingBuffersToDevice( void *outputBuffer, qint32 framesPerBuffer)
{
    {
        QMutexLocker lo(&mutexOpenClose);
        if(isClosing) return false;
    }

    {
        QMutexLocker lio(&mutexDevicesInOut);
        if(devOutClosing) {
            //the device was removed : clear the output buffer one time
            devOutClosing=false;

            int cpt=0;
            // float *outData = (float *) outputBuffer;
            float *outData;
            foreach(CircularBuffer *buf, listCircularBuffersOut) {
                outData = ((float **) outputBuffer)[cpt];
                //empty the circular buffer, in case we reopen this device
                buf->Clear();
                //send a blank buffer to the device
#ifdef WIN32
        ZeroMemory( outData, sizeof(float)*framesPerBuffer );
#else
        memset(outData,0,sizeof(float)*framesPerBuffer  );
#endif
                cpt++;
                // outData += framesPerBuffer;
            }
            return true;
        }
    }

    //send circular buffer to device if there's enough data
    // float *outData = (float *) outputBuffer;
    int cpt=0;
    float *outData;
    foreach(CircularBuffer *buf, listCircularBuffersOut) {
        outData = ((float **) outputBuffer)[cpt];
        if(!pause && buf->filledSize>=framesPerBuffer) {
#ifdef DEBUG_DEVICES
//            LOG(QString("buffer->device %1<-%2/%3 (host:%4)")
//                .arg(framesPerBuffer)
//                .arg(buf->filledSize)
//                .arg(buf->buffSize)
//                .arg(myHost->GetBufferSize())
//                );
#endif
            buf->Get( outData, framesPerBuffer );

        } else {
#ifdef WIN32
            ZeroMemory( outData, sizeof(float)*framesPerBuffer );
#else
            memset(outData,0,sizeof(float)*framesPerBuffer);
#endif
#ifdef DEBUG_DEVICES
            LOG(QString("buffer->device %1<-%2 | not enough data")
                .arg(framesPerBuffer)
                .arg(buf->filledSize));
#endif
            return true;
        }
        //outData += framesPerBuffer;
        cpt++;
    }

    /*if(outputBuffer) {
           QVector<float> grph;
           grph.reserve(framesPerBuffer * sizeof(float));
           std::copy(((float **) outputBuffer)[0], ((float **) outputBuffer)[0] + framesPerBuffer, std::back_inserter(grph));
           emit DebugGraphUpdated(grph);
       }*/

    return true;
}
#else
bool AudioDevice::DeviceToPinBuffers( const void *inputBuffer, qint32 framesPerBuffer )
{
//    qint32 hostBuffSize = myHost->GetBufferSize();
//    if(framesPerBuffer > hostBuffSize) {
//       myHost->SetBufferSize(framesPerBuffer);
//       hostBuffSize = framesPerBuffer;
//    }

    myHost->SetBufferSize(framesPerBuffer);

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

bool AudioDevice::PinBuffersToDevice( void *outputBuffer, qint32 framesPerBuffer )
{
//    qint32 hostBuffSize = myHost->GetBufferSize();
//    if(framesPerBuffer > hostBuffSize) {
//       myHost->SetBufferSize(framesPerBuffer);
//       hostBuffSize = framesPerBuffer;
//    }



    myHost->SetBufferSize(framesPerBuffer);

	if (devOutClosing || !devOut) {
		devOutClosing = false;

        for (int i = 0; i < devInfo.maxOutputChannels; i++) {
        // for (int i = 0; i < objInfo.outputs; i++) {
            for (qint32 j = 0; j < framesPerBuffer; j++) {
				((float **)outputBuffer)[i][j] = .0f;
			}
		}
		return true;
	}

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
#ifdef DEBUG_DEVICES
//    LOG(QString("%1 %2")
//        .arg( device->devInfo.name )
//        .arg( framesPerBuffer )
//        );
#endif

#ifdef CIRCULAR_BUFFER

   //put input buffer in rongbuffer
   if(!device->DeviceToRingBuffers(inputBuffer, framesPerBuffer))
       return paComplete;

   //check if pins buffers are ready
   {
       QMutexLocker l(&mutexCountOpenedDevicesReady);
       if(device->inputBufferReady) {
           countDevicesReady++;
       } else {
#ifdef DEBUG_DEVICES
//            LOG(QString("%1 %2")
//                .arg( device->devInfo.name )
//                .arg("buffer not full")
//                );
#endif
//            return paContinue;
       }
   }

   //all devices are ready : render
   mutexCountOpenedDevicesReady.lock();
#ifdef DEBUG_DEVICES
//    LOG(QString("%1 ready: %2/%3")
//        .arg( device->devInfo.name )
//        .arg( countDevicesReady )
//        .arg( countOpenedDevices )
//        );
#endif
   if(!device->RingBuffersToDevice(outputBuffer, framesPerBuffer))
       return paComplete;

   if(countDevicesReady>=countOpenedDevices) {
       countDevicesReady=0;
#ifdef DEBUG_DEVICES
//        LOG(QString("%1 %2")
//            .arg( device->devInfo.name )
//            .arg("RENDER")
//            );
#endif
       mutexCountOpenedDevicesReady.unlock();

       device->myHost->Render();
       device->myHost->audioDevices->PutPinsBuffersInRingBuffers();
   } else {
       mutexCountOpenedDevicesReady.unlock();
       return paContinue;
   }

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
#ifdef DEBUG_DEVICES
//        LOG("buffer not ready")
#endif
//        return paContinue;
//    }
//    device->SetOutputBufferReady(false);


#endif

//    mutexCountOpenedDevicesReady.lock();
//    countDevicesReady++;
//    mutexCountOpenedDevicesReady.unlock();

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
    if(errorMessage == msg)
        return;

#ifndef QT_NO_DEBUG
    if(!msg.isEmpty()) {
#ifdef DEBUG_DEVICES
        LOG(msg)
#endif
    }
#endif

    errorMessage = msg;

    if(devIn)
        devIn->SetErrorMessage(msg);
    if(devOut)
        devOut->SetErrorMessage(msg);
}
