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

#ifndef MAINWINDOWHOST_H
#define MAINWINDOWHOST_H


#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "models/listaudiointerfacesmodel.h"
#include "models/listmidiinterfacesmodel.h"

class MainHostHost;
class MainWindowHost : public MainWindow {
    Q_OBJECT

public:
    MainWindowHost(Settings *settings, MainHostHost * myHost, QWidget *parent = 0);
    ~MainWindowHost();
    void Init();
    void readSettings();
    void SendMsg(const MsgObject &msg);

protected:
    void closeEvent(QCloseEvent *event);
    void resetSettings();

    ListAudioInterfacesModel *listAudioDevModel;
    ListMidiInterfacesModel *listMidiDevModel;

signals:
    void SendMsgSignal(const MsgObject &msg);

public slots:
    void Kill();
    void UpdateAudioDevices();
    void UpdateMidiDevices();
    void ReceiveMsgSignal(const MsgObject &msg);

private slots:
    void on_actionConfig_triggered();
    void on_actionRefresh_Midi_devices_triggered();
    void on_actionRefresh_Audio_devices_triggered();

};

#endif // MAINWINDOWHOST_H
