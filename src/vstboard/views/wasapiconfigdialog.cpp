/**************************************************************************
#    Copyright 2010-2012 Raphaël François
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
#include "wasapiconfigdialog.h"
#include "ui_wasapiconfigdialog.h"
#include "mainhost.h"
#include "pa_win_wasapi.h"

WasapiConfigDialog::WasapiConfigDialog(MainHost *myHost, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WasapiConfigDialog),
    myHost(myHost)
{
    ui->setupUi(this);
    setWindowTitle(tr("WASAPI Config"));

    unsigned int flags = myHost->settings->GetSetting("api/wasapi_flags", WASAPI_DEFAULT_FLAGS).toUInt();
    ui->exclusiveMode->setChecked( flags & paWinWasapiExclusive);

    ui->inputLatency->setValue( myHost->settings->GetSetting("api/wasapi_inLatency", WASAPI_DEFAULT_INLATENCY).toUInt() );
    ui->outputLatency->setValue( myHost->settings->GetSetting("api/wasapi_outLatency", WASAPI_DEFAULT_OUTLATENCY).toUInt() );
}

WasapiConfigDialog::~WasapiConfigDialog()
{
    delete ui;
}


void WasapiConfigDialog::accept()
{
    unsigned int flags=0;
    if(ui->exclusiveMode->isChecked())
        flags += paWinWasapiExclusive;
    myHost->settings->SetSetting("api/wasapi_flags", flags);

    myHost->settings->SetSetting("api/wasapi_inLatency", ui->inputLatency->value());
    myHost->settings->SetSetting("api/wasapi_outLatency", ui->outputLatency->value());
    QDialog::accept();
}
