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

#ifndef MAINWINDOWCLAP_H
#define MAINWINDOWCLAP_H

#include "mainwindow.h"
#include "ui_mainwindow.h"

class MainWindowClap : public MainWindow {
    Q_OBJECT

public:
    MainWindowClap(MainHost* ctrl, Settings *settings, QWidget *parent = 0);
    void Init() override;
    void readSettings() override;
    void SendMsg(const MsgObject &msg) override;
    MainHost *controller;

private:
    void closeEvent(QCloseEvent *event) override;
    void BuildListTools() override;
    void resetSettings() override;

    QTranslator qtTranslator;
    QTranslator commonTranslator;
    QTranslator myappTranslator;

signals:
    void SendMsgSignal(const MsgObject &msg);

private slots:
    void on_actionConfig_triggered();

public slots:
    void ReceiveMsgSignal(const MsgObject &msg);

};

#endif // MAINWINDOWCLAP_H
