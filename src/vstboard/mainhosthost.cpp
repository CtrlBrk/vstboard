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
#include "mainhosthost.h"
#ifdef RTAUDIO
#include "connectables/mididevice_rm.h"
#include "connectables/audiodevice_rt.h"
#endif
#ifdef PORTAUDIO
#include "connectables/mididevice_pm.h"
#include "connectables/audiodevice_pa.h"
#endif
#include "connectables/objectfactoryhost.h"
#include "mainwindow.h"

#include "connectables/clapplugin.h"

MainHostHost::MainHostHost(Settings *settings, QObject *parent) :
    MainHost(settings,parent),
	audioDevices(0),
	midiDevices(0)
{
	objFactory = new Connectables::ObjectFactoryHost(this);
}

MainHostHost::~MainHostHost()
{
    Close();
}

void MainHostHost::Init()
{
    MainHost::Init();
    midiDevices = new MidiDevices(this,this,FixedObjId::midiDevices);
    audioDevices = new AudioDevices(this,this,FixedObjId::audioDevices);
}

void MainHostHost::Close()
{
    //must be set to zero, devices will check this on close
    delete audioDevices;
    delete midiDevices;
    audioDevices=0;
    midiDevices=0;

    MainHost::Close();
}

void MainHostHost::Render()
{
    #ifdef VSTSDK
#ifdef VST2PLUGIN
        vstHost->UpdateTimeInfo(timeFromStart.elapsed(), GetBufferSize(), GetSampleRate());
#endif
        vst3Host->UpdateTime(GetBufferSize(), GetSampleRate());
    #endif

    Connectables::ClapPlugin::UpdateTransport(GetBufferSize(), GetSampleRate());

    MainHost::Render();
}

void MainHostHost::SendMsg(const MsgObject &msg)
{
    emit SendMsgSignal(msg);
//    mainWindow->ReceiveMsg(msg);
}

void MainHostHost::ReceiveMsgSignal(const MsgObject &msg)
{
    ReceiveMsg(msg);
}
