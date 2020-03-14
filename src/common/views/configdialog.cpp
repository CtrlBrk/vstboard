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

#include "configdialog.h"
#include "ui_configdialog.h"
#include "mainhost.h"
#include "viewconfigdialog.h"

ConfigDialog::ConfigDialog(Settings *settings, MainHost *myHost, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog),
    settings(settings),
    myHost(myHost)
{
    ui->setupUi(this);

//vst path
    ui->defaultVstPath->addItem( QIcon::fromTheme("document-open") , tr("System default"), "systemDefault");
    ui->defaultVstPath->addItem( QIcon::fromTheme("document-open") , tr("From last session"), "fromLastSession");
    ui->defaultVstPath->addItem(tr("Custom path"));

    connect( ui->defaultVstPath, SIGNAL(currentIndexChanged(int)),
             this, SLOT(onVstPathIndexChanged(int)));

    QString vstPath = settings->GetSetting("defaultVstPath", "fromLastSession").toString();
    if(vstPath == "systemDefault" || vstPath == "fromLastSession") {
        ui->defaultVstPath->setCurrentIndex( ui->defaultVstPath->findData( vstPath ) );
    } else {
        if(ui->defaultVstPath->findText(vstPath) == -1)
            ui->defaultVstPath->addItem(vstPath);
        ui->defaultVstPath->setCurrentIndex( ui->defaultVstPath->findText( vstPath ) );
    }

    onVstPathIndexChanged(ui->defaultVstPath->currentIndex());


//bank path
    ui->defaultBankPath->addItem( QIcon::fromTheme("document-open") , tr("From last session"), "fromLastSession");
    ui->defaultBankPath->addItem(tr("Custom path"));

    connect( ui->defaultBankPath, SIGNAL(currentIndexChanged(int)),
             this, SLOT(onBankPathIndexChanged(int)));

    QString bankPath = settings->GetSetting("defaultBankPath", "fromLastSession").toString();
    if(bankPath == "fromLastSession") {
        ui->defaultBankPath->setCurrentIndex( ui->defaultBankPath->findData( bankPath ) );
    } else {
        if(ui->defaultBankPath->findText(bankPath) == -1)
            ui->defaultBankPath->addItem(bankPath);
        ui->defaultBankPath->setCurrentIndex( ui->defaultBankPath->findText( bankPath ) );
    }

    onBankPathIndexChanged(ui->defaultBankPath->currentIndex());

//setup file
    ui->defaultSetup->addItem( QIcon::fromTheme("document-new") , tr("Empty setup"), "empty");
    ui->defaultSetup->addItem( QIcon::fromTheme("document-open") , tr("From last session"), "fromLastSession");
    ui->defaultSetup->addItem(tr("Custom setup file"));
    foreach(const QString &str, settings->GetSetting("recentSetupFiles").toStringList()) {
        ui->defaultSetup->addItem(str);
    }

    connect( ui->defaultSetup, SIGNAL(currentIndexChanged(int)),
             this, SLOT(onSetupIndexChanged(int)));

    QString setupFile = settings->GetSetting("defaultSetupFile","fromLastSession").toString();
    if(setupFile == "empty" || setupFile == "fromLastSession") {
        ui->defaultSetup->setCurrentIndex( ui->defaultSetup->findData( setupFile ) );
    } else {
        if(ui->defaultSetup->findText(setupFile) == -1)
            ui->defaultSetup->addItem(setupFile);
        ui->defaultSetup->setCurrentIndex( ui->defaultSetup->findText( setupFile ) );
    }

    onSetupIndexChanged(ui->defaultSetup->currentIndex());

//project file
    ui->defaultProject->addItem( QIcon::fromTheme("document-new") , tr("Empty project"), "empty");
    ui->defaultProject->addItem( QIcon::fromTheme("document-open") , tr("From last session"), "fromLastSession");
    ui->defaultProject->addItem(tr("Custom project file"));
    foreach(const QString &str, settings->GetSetting("recentProjectFiles").toStringList()) {
        ui->defaultProject->addItem(str);
    }

    connect( ui->defaultProject, SIGNAL(currentIndexChanged(int)),
             this, SLOT(onProjectIndexChanged(int)));

    QString projectFile = settings->GetSetting("defaultProjectFile","fromLastSession").toString();
    if(projectFile == "empty" || projectFile == "fromLastSession") {
        ui->defaultProject->setCurrentIndex( ui->defaultProject->findData( projectFile ) );
    } else {
        if(ui->defaultProject->findText(projectFile) == -1)
            ui->defaultProject->addItem(projectFile);
        ui->defaultProject->setCurrentIndex( ui->defaultProject->findText( projectFile ) );
    }

    onProjectIndexChanged(ui->defaultProject->currentIndex());

    int index=0;
    ui->bufferSize->setVisible(false);
    ui->labelbufferSize->setVisible(false);

//sample precision
    ui->samplePrecision->addItem("32 bits",false);
    ui->samplePrecision->addItem("64 bits",true);
    index=ui->samplePrecision->findData( defaultDoublePrecision(settings) );
    if(index==-1) {
        LOG("invalid sample precision"<<index)
        index=0;
    }
    ui->samplePrecision->setCurrentIndex(index);


//on unsaved setup
    ui->onUnsavedSetup->addItem(tr("Always ask"),Qt::PartiallyChecked);
    ui->onUnsavedSetup->addItem(tr("Save changes"),Qt::Checked);
    ui->onUnsavedSetup->addItem(tr("Discard changes"),Qt::Unchecked);

    int unsavedSetup = settings->GetSetting("onUnsavedSetup",Qt::PartiallyChecked).toInt();
    ui->onUnsavedSetup->setCurrentIndex( ui->onUnsavedSetup->findData(unsavedSetup) );

//on unsaved project
    ui->onUnsavedProject->addItem(tr("Always ask"),Qt::PartiallyChecked);
    ui->onUnsavedProject->addItem(tr("Save changes"),Qt::Checked);
    ui->onUnsavedProject->addItem(tr("Discard changes"),Qt::Unchecked);

    int unsavedProject = settings->GetSetting("onUnsavedProject",Qt::PartiallyChecked).toInt();
    ui->onUnsavedProject->setCurrentIndex( ui->onUnsavedProject->findData(unsavedProject) );

//engine
    int th = settings->GetSetting("NbThreads",0).toInt();
    if(th<1 && th>32)
        th=0;
    ui->nbThreads->setValue( th );

    if(myHost) {
        connect(ui->nbThreads, SIGNAL(valueChanged(int)),
            myHost, SLOT(ChangeNbThreads(int)));
    }

//hide/close editors
    ui->fastEditorOpenClose->setChecked( settings->GetSetting("fastEditorsOpenClose",true).toBool() );
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}


void ConfigDialog::onVstPathIndexChanged(int index)
{
    QString txt = ui->defaultVstPath->itemData(index).toString();
    if(txt == "systemDefault" || txt == "fromLastSession") {
        ui->defaultVstPath->setEditable(false);
    } else {
        ui->defaultVstPath->setEditable(true);
    }
}

void ConfigDialog::onBankPathIndexChanged(int index)
{
    QString txt = ui->defaultBankPath->itemData(index).toString();
    if(txt == "fromLastSession") {
        ui->defaultBankPath->setEditable(false);
    } else {
        ui->defaultBankPath->setEditable(true);
    }
}

void ConfigDialog::onSetupIndexChanged(int index)
{
    QString txt = ui->defaultSetup->itemData(index).toString();
    if( txt == "empty" || txt == "fromLastSession") {
        ui->defaultSetup->setEditable(false);
    } else {
        ui->defaultSetup->setEditable(true);
    }
}

void ConfigDialog::onProjectIndexChanged(int index)
{
    QString txt = ui->defaultProject->itemData(index).toString();
    if( txt == "empty" || txt == "fromLastSession") {
        ui->defaultProject->setEditable(false);
    } else {
        ui->defaultProject->setEditable(true);
    }
}


const QString ConfigDialog::defaultSetupFile(Settings *settings)
{
    QString file = settings->GetSetting("defaultSetupFile","fromLastSession").toString();
    if(file == "empty") {
        return "";
    }
    if(file == "fromLastSession") {
        file = settings->GetSetting("lastSetupFile","").toString();
    }
    return file;
}

const QString ConfigDialog::defaultProjectFile(Settings *settings)
{
    QString file = settings->GetSetting("defaultProjectFile","fromLastSession").toString();
    if(file == "empty") {
        return "";
    }
    if(file == "fromLastSession") {
        file = settings->GetSetting("lastProjectFile","").toString();
    }
    return file;
}

const QString ConfigDialog::defaultVstPath(Settings *settings)
{
    QString vstPathType = settings->GetSetting("defaultVstPath","fromLastSession").toString();

    QSettings vstSettings("HKEY_LOCAL_MACHINE\\Software\\VST", QSettings::NativeFormat);
    QString defaultPath = vstSettings.value("VSTPluginsPath", "").toString();
    defaultPath.replace("\\","/");

    QString lastPath = settings->GetSetting("lastVstPath", "").toString();

    //if we want the path from the last session
    if(vstPathType == "fromLastSession") {

        //get the path from the last session
        if(!lastPath.isEmpty())
            return lastPath;

        //no last session, return the system default
        if(!defaultPath.isEmpty())
            return defaultPath;

        //no system default, return the home dir
        return QDir::homePath();
    }

    //if we want the system default vst path
    if(vstPathType == "systemDefault") {
        //get the system default
        if(!defaultPath.isEmpty())
            return defaultPath;

        //no system default, return the path from last session
        if(!lastPath.isEmpty())
            return lastPath;

        //no last session path, return the home dir
        return QDir::homePath();
    }

    //else return the custom path
    if(!vstPathType.isEmpty())
        return vstPathType;

    //no custom path, return the path from last session
    if(!lastPath.isEmpty())
        return lastPath;

    //no last session path, return the home dir
    return QDir::homePath();
}

const QString ConfigDialog::defaultBankPath(Settings * settings)
{
    QString bankPath = settings->GetSetting("defaultBankPath","fromLastSession").toString();

    if(bankPath == "fromLastSession") {
        bankPath = settings->GetSetting("lastBankPath","").toString();
    }

    if(!bankPath.isEmpty())
        return bankPath;

    //no last session path, return the home dir
    return QDir::homePath();
}

const float ConfigDialog::defaultSampleRate(Settings *settings)
{
    return settings->GetSetting("sampleRate",44100.0).toFloat();
}

const int ConfigDialog::defaultBufferSize(Settings *settings)
{
    return settings->GetSetting("bufferSize").toInt();
}

const bool ConfigDialog::defaultDoublePrecision(Settings *settings)
{
    return settings->GetSetting("doublePrecision",false).toBool();
}

void ConfigDialog::AddRecentSetupFile(const QString &file,Settings *settings)
{
    if(!file.isEmpty()) {
        QStringList lstFiles = settings->GetSetting("recentSetupFiles").toStringList();
        lstFiles.insert(0, file);
        lstFiles.removeDuplicates();
        while(lstFiles.size()>NB_RECENT_FILES) {
            lstFiles.removeLast();
        }
        settings->SetSetting("recentSetupFiles",lstFiles);
    }
    settings->SetSetting("lastSetupFile", file);
}

void ConfigDialog::AddRecentProjectFile(const QString &file,Settings *settings)
{
    if(!file.isEmpty()) {
        QStringList lstFiles = settings->GetSetting("recentProjectFiles").toStringList();
        lstFiles.insert(0, file);
        lstFiles.removeDuplicates();
        while(lstFiles.size()>NB_RECENT_FILES) {
            lstFiles.removeLast();
        }
        settings->SetSetting("recentProjectFiles",lstFiles);
    }
    settings->SetSetting("lastProjectFile", file);
}

void ConfigDialog::RemoveRecentSetupFile(const QString &file,Settings *settings)
{
    QStringList lstFiles = settings->GetSetting("recentSetupFiles").toStringList();
    lstFiles.removeAll(file);
    settings->SetSetting("recentSetupFiles",lstFiles);
}

void ConfigDialog::RemoveRecentProjectFile(const QString &file,Settings *settings)
{
    QStringList lstFiles = settings->GetSetting("recentProjectFiles").toStringList();
    lstFiles.removeAll(file);
    settings->SetSetting("recentProjectFiles",lstFiles);
}

void ConfigDialog::accept()
{
    bool needRestart=false;

//vst directory
    QString vstDir = ui->defaultVstPath->itemData( ui->defaultVstPath->currentIndex() ).toString();
    if(vstDir != "systemDefault" && vstDir != "fromLastSession") {
        vstDir = ui->defaultVstPath->currentText();
        QDir dir;
        if(vstDir.isEmpty() || !dir.exists(vstDir)) {
            QMessageBox msg(QMessageBox::Critical,
                            tr("Error"),
                            tr("Vst path is not a valid directory"),
                            QMessageBox::Ok,
                            this);
            msg.exec();
            ui->defaultVstPath->setFocus();
            return;
        }
    }

//bank directory
    QString bankDir = ui->defaultBankPath->itemData( ui->defaultBankPath->currentIndex() ).toString();
    if(bankDir != "fromLastSession") {
        bankDir = ui->defaultBankPath->currentText();
        QDir dir;
        if(bankDir.isEmpty() || !dir.exists(bankDir)) {
            QMessageBox msg(QMessageBox::Critical,
                            tr("Error"),
                            tr("Bank path is not a valid directory"),
                            QMessageBox::Ok,
                            this);
            msg.exec();
            ui->defaultBankPath->setFocus();
            return;
        }
    }

//setup file
    QString setupFile =  ui->defaultSetup->itemData( ui->defaultSetup->currentIndex() ).toString();
    if(setupFile != "empty" && setupFile != "fromLastSession") {
        setupFile = ui->defaultSetup->currentText();
        QFile file;
        if(!file.exists(setupFile) || !setupFile.endsWith(SETUP_FILE_EXTENSION)) {
            QMessageBox msg(QMessageBox::Critical,
                            tr("Error"),
                            tr("Setup file is not valid"),
                            QMessageBox::Ok,
                            this);
            msg.exec();
            ui->defaultSetup->setFocus();
            return;
        }
    }

//project file
    QString projectFile =  ui->defaultProject->itemData( ui->defaultProject->currentIndex() ).toString();
    if(projectFile != "empty" && projectFile != "fromLastSession") {
        projectFile = ui->defaultProject->currentText();
        QFile file;
        if(!file.exists(projectFile) || !projectFile.endsWith(PROJECT_FILE_EXTENSION)) {
            QMessageBox msg(QMessageBox::Critical,
                            tr("Error"),
                            tr("Project file is not valid"),
                            QMessageBox::Ok,
                            this);
            msg.exec();
            ui->defaultProject->setFocus();
            return;
        }
    }

//default files
    settings->SetSetting("defaultVstPath", vstDir );
    settings->SetSetting("defaultBankPath", bankDir );
    settings->SetSetting("defaultSetupFile", setupFile );
    settings->SetSetting("defaultProjectFile", projectFile );

    bool precision = ui->samplePrecision->itemData(ui->samplePrecision->currentIndex()).toBool();
    if(defaultDoublePrecision(settings)!=precision)
        needRestart=true;
    settings->SetSetting("doublePrecision",precision);

//on unsaved changes
    settings->SetSetting("onUnsavedSetup", ui->onUnsavedSetup->itemData( ui->onUnsavedSetup->currentIndex() ).toInt() );
    settings->SetSetting("onUnsavedProject", ui->onUnsavedProject->itemData( ui->onUnsavedProject->currentIndex() ).toInt() );

//engine
    int oldNbThreads = settings->GetSetting("NbThreads",0).toInt();
    int newNbThreads = ui->nbThreads->value();
    if( newNbThreads != oldNbThreads ) {
        settings->SetSetting("NbThreads", newNbThreads);
    }
//hide/close editors
    settings->SetSetting("fastEditorsOpenClose", ui->fastEditorOpenClose->isChecked());

    QDialog::accept();

    if(needRestart) {
        QMessageBox msg;
        msg.setText(tr("You must restart VstBoard for the changes to take effect"));
        msg.setIcon(QMessageBox::Information);
        msg.exec();
    }
}

void ConfigDialog::reject()
{
    //reset nb threads
    if(myHost) {
        int th = settings->GetSetting("NbThreads",0).toInt();
        myHost->ChangeNbThreads(th);
    }

    QDialog::reject();
}

void ConfigDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void ConfigDialog::on_browseVst_clicked()
{
    QFileDialog fileDlg( this, tr("Default Vst path"), defaultBankPath(settings), tr("Directory (*.)") );
    fileDlg.setFileMode(QFileDialog::Directory);

    QString filename;
    if (fileDlg.exec())
        filename = fileDlg.selectedFiles().value(0);

    if(filename.isEmpty())
        return;

    if(ui->defaultVstPath->findText(filename) == -1)
        ui->defaultVstPath->addItem(filename);
    ui->defaultVstPath->setCurrentIndex( ui->defaultVstPath->findText(filename) );
    settings->SetSetting("defaultVstPath", filename );
}

void ConfigDialog::on_browseSetup_clicked()
{
    QFileDialog fileDlg( this, tr("Default setup file"), defaultSetupFile(settings), tr("Setup file (*.%1)").arg(SETUP_FILE_EXTENSION) );
    fileDlg.setFileMode(QFileDialog::ExistingFile);
    fileDlg.setDefaultSuffix(SETUP_FILE_EXTENSION);

    QString filename;
    if (fileDlg.exec())
        filename = fileDlg.selectedFiles().value(0);

    if(filename.isEmpty())
        return;

    if(!filename.endsWith(SETUP_FILE_EXTENSION)) {
        QMessageBox msgBox;
        msgBox.setText(tr("Wrong file extension."));
        msgBox.exec();
        return;
    }

    AddRecentSetupFile(filename,settings);

    int i = ui->defaultSetup->findText(filename);
    if(i == -1) {
        ui->defaultSetup->addItem(filename);
        i = ui->defaultSetup->findText(filename);
    }
    ui->defaultSetup->setCurrentIndex( i );
    settings->SetSetting("defaultSetupFile", filename );
}

void ConfigDialog::on_browseProject_clicked()
{
    QFileDialog fileDlg( this, tr("Default project file"), defaultSetupFile(settings), tr("Project file (*.%1)").arg(PROJECT_FILE_EXTENSION) );
    fileDlg.setFileMode(QFileDialog::ExistingFile);
    fileDlg.setDefaultSuffix(PROJECT_FILE_EXTENSION);

    QString filename;
    if (fileDlg.exec())
        filename = fileDlg.selectedFiles().value(0);

    if(filename.isEmpty())
        return;

    if(!filename.endsWith(PROJECT_FILE_EXTENSION)) {
        QMessageBox msgBox;
        msgBox.setText(tr("Wrong file extension."));
        msgBox.exec();
        return;
    }

    AddRecentProjectFile(filename,settings);

    int i = ui->defaultProject->findText(filename);
    if(i == -1) {
        ui->defaultProject->addItem(filename);
        i = ui->defaultProject->findText(filename);
    }
    ui->defaultProject->setCurrentIndex( i );
    settings->SetSetting("defaultProjectFile", filename );
}

void ConfigDialog::on_browseBank_clicked()
{
    QFileDialog fileDlg( this, tr("Default Bank path"), defaultBankPath(settings), tr("Directory (*.)") );
    fileDlg.setFileMode(QFileDialog::Directory);

    QString filename;
    if (fileDlg.exec())
        filename = fileDlg.selectedFiles().value(0);

    if(filename.isEmpty())
        return;

    if(ui->defaultBankPath->findText(filename) == -1)
        ui->defaultBankPath->addItem(filename);
    ui->defaultBankPath->setCurrentIndex( ui->defaultBankPath->findText(filename) );
    settings->SetSetting("defaultBankPath", filename );
}

