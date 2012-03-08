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
#include "mmeconfigdialog.h"
#include "ui_mmeconfigdialog.h"
#include "mainhost.h"
#include "pa_win_wmme.h"

MmeConfigDialog::MmeConfigDialog( MainHost *myHost, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MmeConfigDialog),
    myHost(myHost)
{
    ui->setupUi(this);
    setWindowTitle(tr("MME Config"));

    unsigned int flags = myHost->settings->GetSetting("api/wmme_flags", MME_DFAULT_FLAGS).toUInt();
    unsigned int framesPerBuffer = myHost->settings->GetSetting("api/wmme_bufferSize", MME_DEFAULT_BUFFER_SIZE).toUInt();
    unsigned int bufferCount = myHost->settings->GetSetting("api/wmme_bufferCount", MME_DEFAULT_BUFFER_COUNT).toUInt();

    ui->UseLowLevelLatencyParameters->setChecked( flags & paWinMmeUseLowLevelLatencyParameters );
    ui->DontThrottleOverloadedProcessingThread->setChecked( flags & paWinMmeDontThrottleOverloadedProcessingThread );

    ui->framesPerBuffer->setValue( framesPerBuffer );
    ui->framesPerBuffer->setEnabled( ui->UseLowLevelLatencyParameters->isChecked() );
    ui->bufferCount->setValue( bufferCount );
    ui->bufferCount->setEnabled( ui->UseLowLevelLatencyParameters->isChecked() );
}

MmeConfigDialog::~MmeConfigDialog()
{
    delete ui;
}

void MmeConfigDialog::accept()
{
    unsigned int flags=0;
    if(ui->UseLowLevelLatencyParameters->isChecked())
        flags += paWinMmeUseLowLevelLatencyParameters;
    if(ui->DontThrottleOverloadedProcessingThread->isChecked())
        flags += paWinMmeDontThrottleOverloadedProcessingThread;

    unsigned int framesPerBuffer = ui->framesPerBuffer->value();
    unsigned int bufferCount = ui->bufferCount->value();

    myHost->settings->SetSetting("api/wmme_flags", flags);
    myHost->settings->SetSetting("api/wmme_bufferSize", framesPerBuffer);
    myHost->settings->SetSetting("api/wmme_bufferCount", bufferCount);

    QDialog::accept();
}
