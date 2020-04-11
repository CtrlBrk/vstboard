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
#include "configdialoghost.h"
#include "ui_configdialog.h"
#include "mainhost.h"

ConfigDialogHost::ConfigDialogHost(Settings *settings,MainHost *myHost, QWidget *parent) :
    ConfigDialog(settings,myHost, parent)
{
    QList<float> rates {
    22050,
    24000,
    32000,
    44100,
    48000,
    88200,
    96000,
    192000,
    };

    for (int i = 0; i < rates.size(); ++i) {
        ui->sampleRate->addItem(
                    QString::number(rates.at(i)/1000,'f',1) + " kHz",
                        rates.at(i)
                    );
    }
//    ui->sampleRate->addItem("44.1 kHz",44100);
//    ui->sampleRate->addItem("48 kHz",48000);
//    ui->sampleRate->addItem("88.2 kHz",88200);
//    ui->sampleRate->addItem("96 kHz",96000);

    int index=ui->sampleRate->findData( myHost->GetSampleRate() );
    if(index==-1) {
        LOG("invalid sample rate"<<index);
        return;
    }
    ui->sampleRate->setCurrentIndex(index);

//buffer size
//    ui->bufferSize->addItem("Auto",0);
//    index = ui->bufferSize->findData( (unsigned int)myHost->GetBufferSize() );
//    if(index==-1) {
//        debug("ConfigDialog invalid buffer size")
//        index=0;
//    }
//    ui->bufferSize->setCurrentIndex(index);
}

void ConfigDialogHost::accept()
{
    float rate = ui->sampleRate->itemData(ui->sampleRate->currentIndex()).toFloat();
    if(rate!=defaultSampleRate(settings)) {
        myHost->SetSampleRate( rate );
    }
    myHost->settings->SetSetting("sampleRate", rate);

//    int buffer = ui->bufferSize->itemData(ui->bufferSize->currentIndex()).toInt();
//    settings.setValue("bufferSize", buffer);

    ConfigDialog::accept();
}
