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

#ifndef MAINWINDOWVST_H
#define MAINWINDOWVST_H

#include "mainwindow.h"
#include "../common/ui_mainwindow.h"

#ifdef _MSC_VER
#pragma warning( push, 1 )
#endif
#include "public.sdk/source/vst/vsteditcontroller.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

using namespace Steinberg;

class MainWindowVst : public MainWindow {
    Q_OBJECT

public:
    MainWindowVst(Vst::EditController *controller,Settings *settings, QWidget *parent = 0);
    ~MainWindowVst();
    void Init();
    void readSettings();
    void SendMsg(const MsgObject &msg);
    Vst::EditController *controller;

private:
    void closeEvent(QCloseEvent *event);
    void BuildListTools();
    void resetSettings();

    QTranslator qtTranslator;
    QTranslator commonTranslator;
    QTranslator myappTranslator;

private slots:
    void on_actionConfig_triggered();
};

#endif // MAINWINDOWVST_H
