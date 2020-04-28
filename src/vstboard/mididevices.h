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

#ifndef MIDIDEVICES_H
#define MIDIDEVICES_H

//#include "portmidi.h"
//#include "porttime.h"

#include "models/listmidiinterfacesmodel.h"
#include "connectables/mididevice.h"

class MainHostHost;
class MidiDevices : public QObject, public MsgHandler
{
Q_OBJECT
public:
    explicit MidiDevices(MainHostHost *myHost, MsgController *msgCtrl, int objId);
    ~MidiDevices();

//    ListMidiInterfacesModel* GetModel();

    void OpenDevice(Connectables::MidiDevice* objPtr);
    void CloseDevice(Connectables::MidiDevice* objPtr);

    static RtMidi::Api GetApiByName(const std::string &apiName);
    static int GetDevIdByName(const ObjectInfo &objInfo);

    void ReceiveMsg(const MsgObject &msg);
private:
    void OpenDevices();
    void BuildModel();
//    static void MidiReceive_poll(PtTimestamp timestamp, void *userData);
    QList< Connectables::MidiDevice* >listOpenedMidiDevices;

//    ListMidiInterfacesModel *model;
    bool pmOpened;

    QMutex mutexListMidi;
    MainHostHost *myHost;
    QList<qint32>listOpenedDevices;
};

#endif // MIDIDEVICES_H
