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

#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H

#include <QWaitCondition>

//#include "precomp.h"
#include "connectables/object.h"
#include "connectables/objectinfo.h"
#include "portaudio.h"
#include "pa_win_wmme.h"
#include "pa_win_ds.h"
#include "pa_win_wasapi.h"

#ifdef CIRCULAR_BUFFER
    #include "circularbuffer.h"
#endif

class MainHostHost;
namespace Connectables {

    class AudioDeviceIn;
    class AudioDeviceOut;
    class AudioDevice : public QObject
    {
        Q_OBJECT
    public:
        AudioDevice(MainHostHost *myHost,const ObjectInfo &info, QObject *parent=0);
        ~AudioDevice();

        float GetCpuUsage();
        bool SetObjectInput(AudioDeviceIn *obj);
        bool SetObjectOutput(AudioDeviceOut *obj);
        bool SetSleep(bool sleeping);

        int GetNbInputs() const { return devInfo.maxInputChannels; }
        int GetNbOutputs() const { return devInfo.maxOutputChannels; }

        bool IsAnInstanceOf(const ObjectInfo &info);

    #ifdef CIRCULAR_BUFFER
        void PinsToRingBuffers();
    #endif

        /// current buffre size
        unsigned long bufferSize;
        float **currentInputBuffer;
        float **currentOutputBuffer;

        void GetErrMessage(QString &msg) {msg = errorMessage; }

    private:
        bool Open();
        bool Close();
        static int paCallback( const void *inputBuffer, void *outputBuffer,
                               unsigned long framesPerBuffer,
                               const PaStreamCallbackTimeInfo* timeInfo,
                               PaStreamCallbackFlags statusFlags,
                               void *userData );

        bool OpenStream(double sampleRate);

#ifdef CIRCULAR_BUFFER
        void CreateCircularBuffers();
        void DeleteCircularBuffers();

        bool DeviceToRingBuffers( const void *inputBuffer, unsigned long framesPerBuffer);
        void RingBuffersToPins();
        bool RingBuffersToDevice( void *outputBuffer, unsigned long framesPerBuffer);
#else
        //if not using ringbuffers
        bool DeviceToPinBuffers( const void *inputBuffer, unsigned long framesPerBuffer );
        bool PinBuffersToDevice( void *outputBuffer, unsigned long framesPerBuffer );
#endif

        void SetErrorMsg(const QString &msg);
        QString errorMessage;

        /// true if the device is currently closing
        bool isClosing;

        /// true if the AudioDeviceOut is closing
        bool devOutClosing;

        /// current sample rate
        float sampleRate;

        /// pointer to PortAudio stream
        PaStream *stream;

        /// PortAudio device informations
        PaDeviceInfo devInfo;

        /// objcet description
        ObjectInfo objInfo;

        /// pointer to the AudioDeviceIn, can be null
        AudioDeviceIn *devIn;

        /// pointer to the AudioDeviceOut, can be null
        AudioDeviceOut *devOut;

        /// true if the device is opened
        bool opened;

        /// mutex protecting open/close
        QMutex mutexOpenClose;

        /// mutex for input and output device
        QMutex mutexDevicesInOut;

        /// global audio devices mutex
        static QMutex mutexCountOpenedDevicesReady;

        /// windows mme stream options
        PaWinMmeStreamInfo wmmeStreamInfo;

        /// windows directsound stream options
        PaWinDirectSoundStreamInfo directSoundStreamInfo;

        /// windows wasapi stream options
        PaWasapiStreamInfo wasapiStreamInfo;

#ifdef CIRCULAR_BUFFER
        /// list of input ring buffers
        QList<CircularBuffer*>listCircularBuffersIn;

        /// list of output ring buffers
        QList<CircularBuffer*>listCircularBuffersOut;
#endif

        /// pointer to the MainHost
        MainHostHost *myHost;

        /// the number of opened devices
        static int countOpenedDevices;

        /// the number of devices ready to render. launch a rendering loop when all the devices are ready
        static int countDevicesReady;

        /// check if this device has been counted in the list of ready devices
        bool inputBufferReady;

        /// debug: stop output, fill buffer
        bool pause;

    signals:
        /*!
          emitted when the device is opened or closed, used by AudioDevices
          \param apiId api index
          \param devId device id
          \param inUse true if the device is in use
          */
        void InUseChanged(PaHostApiIndex apiId,PaDeviceIndex devId, bool inUse, PaTime inLatency=0, PaTime outLatency=0, double sampleRate=0);
        void DebugGraphUpdated(QVector<float> grph);

    public slots:
        void SetSampleRate(float rate=44100.0);
        void DeleteIfUnused();
        void PauseOutput(bool);
    };
}

#endif // AUDIODEVICE_H
