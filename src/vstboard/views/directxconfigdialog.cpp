#include "directxconfigdialog.h"
#include "ui_directxconfigdialog.h"
#include "mainhost.h"
// #include "pa_win_ds.h"

DirectxConfigDialog::DirectxConfigDialog(MainHost *myHost, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DirectxConfigDialog),
    myHost(myHost)
{
    ui->setupUi(this);
    setWindowTitle(tr("DirectSound Config"));
	/*
    unsigned int flags = myHost->settings->GetSetting("api/dx_flags", DIRECTX_DFAULT_FLAGS).toUInt();
    unsigned int framesPerBuffer = myHost->settings->GetSetting("api/dx_bufferSize", DIRECTX_DEFAULT_BUFFER_SIZE).toUInt();

    ui->UseLowLevelLatencyParameters->setChecked( flags & paWinDirectSoundUseLowLevelLatencyParameters );
    ui->framesPerBuffer->setValue( framesPerBuffer );
    ui->framesPerBuffer->setEnabled( ui->UseLowLevelLatencyParameters->isChecked() );
	*/
}

DirectxConfigDialog::~DirectxConfigDialog()
{
    delete ui;
}

void DirectxConfigDialog::accept()
{
	/*
    unsigned int flags=0;
    if(ui->UseLowLevelLatencyParameters->isChecked())
        flags += paWinDirectSoundUseLowLevelLatencyParameters;

    unsigned int framesPerBuffer = ui->framesPerBuffer->value();

    myHost->settings->SetSetting("api/dx_flags", flags);
    myHost->settings->SetSetting("api/dx_bufferSize", framesPerBuffer);
	*/
    QDialog::accept();
}
