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

#ifndef AUDIODEVICES_H
#define AUDIODEVICES_H

#define FAKE_RENDER_TIMER_MS 5

//#include "precomp.h"
#include "portaudio.h"
#include "models/listaudiointerfacesmodel.h"
#include "connectables/objectinfo.h"
#include "connectables/audiodevice.h"

class MainHostHost;

class FakeTimer : public QThread
{
public:
    FakeTimer(MainHostHost *myHost);
    ~FakeTimer();
    void run();
    void CleanupBuffers();

private:
    MainHostHost *myHost;
    bool stop;
};


class AudioDevices : public QObject, public MsgHandler
{
    Q_OBJECT
public:
    explicit AudioDevices(MainHostHost *myHost, MsgController *msgCtrl, int objId);
    ~AudioDevices();
//    ListAudioInterfacesModel * GetModel();
    Connectables::AudioDevice * AddDevice(ObjectInfo &objInfo);
    void RemoveDevice(Connectables::AudioDevice *dev);

#ifdef CIRCULAR_BUFFER
    void PutPinsBuffersInRingBuffers();
#endif

    /// timer to launch the rendering loop when no audio devices are opened
    FakeTimer *fakeRenderTimer;

    /// model index of the asio devices, used by the view to expand this branch only
    QPersistentModelIndex AsioIndex;

    bool FindPortAudioDevice(ObjectInfo &objInfo, PaDeviceInfo *dInfo);
    void ReceiveMsg(const MsgObject &msg);

private:
    void CloseDevices(bool close=false);
    void OpenDevices();
    void BuildModel();

    bool closing;

    /// list of opened AudioDevice
    QList<Connectables::AudioDevice* >listAudioDevices;

    /// model pointer
//    ListAudioInterfacesModel *model;
    bool paOpened;

    /// number of opened devices
//    int countActiveDevices;

    /// pointer to the MainHost
    MainHostHost *myHost;

    QMutex mutexDevices;

    QMutex mutexClosing;
    QList<qint32>listOpenedDevices;

    QTimer timerRefreshDevices;

public slots:
    void OnToggleDeviceInUse(PaHostApiIndex apiId, PaDeviceIndex devId, bool inUse, PaTime inLatency=0, PaTime outLatency=0, double sampleRate=0);
    void ConfigDevice(const ObjectInfo &info);

private slots:
    void TryToOpenDevice();
};

#endif // AUDIODEVICES_H
