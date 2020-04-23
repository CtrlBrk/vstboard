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

#include "viewconfigdialog.h"
#include "ui_viewconfigdialog.h"

using namespace View;

/*!
  \class View::ViewConfigDialog
  \brief modify the application appearance
  */

/*!
  Constructor
  \param myHost pointer to the MainHost
  \param parent pointer to a parent widget
  */
ViewConfigDialog::ViewConfigDialog(MainHost *myHost, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ViewConfigDialog),
    myHost(myHost),
    conf(myHost->mainWindow->viewConfig),
    currentGrp(ColorGroups::ND),
    currentCol(Colors::ND),
	backupSaveInSetup(false),
    modified(false),
    updateInProgress(false)
{
    ui->setupUi(this);

    connect(conf,SIGNAL(NewSetupLoaded()),
            this,SLOT(InitDialog()));

    connect(ui->picker,SIGNAL(colorSelected(QColor)),
            this,SLOT(onPickerColorSelected(QColor)));
    connect(ui->picker_hue,SIGNAL(colorSelected(QColor)),
            this,SLOT(onPickerHueSelected(QColor)));

    connect(ui->RedSlider,SIGNAL(valueChanged(int)),
            ui->RedSpinBox,SLOT(setValue(int)));
    connect(ui->GreenSlider,SIGNAL(valueChanged(int)),
            ui->GreenSpinBox,SLOT(setValue(int)));
    connect(ui->BlueSlider,SIGNAL(valueChanged(int)),
            ui->BlueSpinBox,SLOT(setValue(int)));
    connect(ui->AlphaSlider,SIGNAL(valueChanged(int)),
            ui->AlphaSpinBox,SLOT(setValue(int)));

    connect(ui->RedSpinBox,SIGNAL(valueChanged(int)),
            ui->RedSlider,SLOT(setValue(int)));
    connect(ui->GreenSpinBox,SIGNAL(valueChanged(int)),
            ui->GreenSlider,SLOT(setValue(int)));
    connect(ui->BlueSpinBox,SIGNAL(valueChanged(int)),
            ui->BlueSlider,SLOT(setValue(int)));
    connect(ui->AlphaSpinBox,SIGNAL(valueChanged(int)),
            ui->AlphaSlider,SLOT(setValue(int)));

    InitDialog();
}

/*!
  Destructor
  */
ViewConfigDialog::~ViewConfigDialog()
{
    delete ui;
}

void ViewConfigDialog::InitDialog()
{
    ui->checkSavedInSetupFile->setChecked( conf->IsSavedInSetup() );
    InitLists();
    LoadPreset( conf->GetPresetName() );

    ui->fontProgFamily->addItem("Default");
    QFontDatabase database;
    foreach (const QString &family, database.families()) {
        ui->fontProgFamily->addItem(family);
    }
    ui->fontProgFamily->setCurrentIndex( ui->fontProgFamily->findText(myHost->settings->GetSetting("fontProgFamily","Default").toString()) );

    int s = myHost->settings->GetSetting("fontProgSize",10).toInt();
    if(s<=0) s=8;
    ui->fontProgSize->setValue( s );
    ui->fontProgBold->setChecked( myHost->settings->GetSetting("fontProgbold",false).toBool() );
    ui->fontProgItalic->setChecked( myHost->settings->GetSetting("fontProgItalic",false).toBool() );

    s = myHost->settings->GetSetting("fontProgStretch",100).toInt();
    if(s<=0) s=100;
    ui->fontProgStretch->setValue( s );
    UpdateProgramsFont();
}

void ViewConfigDialog::InitLists()
{
    ui->listPresets->clear();
    ui->listPresets->addItem( "Default" );

    viewConfigPresetList::const_iterator ip = conf->GetListOfPresets()->constBegin();
    while(ip!=conf->GetListOfPresets()->constEnd()) {
        if( ip.key()!="Default") {
            QListWidgetItem *item = new QListWidgetItem( ip.key() );
            item->setData( Qt::UserRole+1, ip.key() );
            item->setFlags (item->flags () | Qt::ItemIsEditable);
            ui->listPresets->addItem( item );
        }
        ++ip;
    }
}

/*!
  Called when Ok is clicked
  */
void ViewConfigDialog::accept()
{
    SaveChanges();
    QDialog::accept();
}

/*!
  Called when Cancel is clicked. Asks the user if he wants to discard changes
  */
void ViewConfigDialog::reject()
{
    DiscardChanges();
    QDialog::reject();
}

bool ViewConfigDialog::UserWantsToUnloadPreset()
{
    if(!modified)
        return true;

    QMessageBox msg(QMessageBox::Question, tr("VstBoard Appearance"), tr("Discard changes ?"), QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, this);
    msg.exec();

    switch(msg.result()) {
        case QMessageBox::Cancel:
            return false;
        case QMessageBox::Discard:
            DiscardChanges();
            break;
        case QMessageBox::Save:
            SaveChanges();
            break;
    }

    return true;
}

void ViewConfigDialog::SaveChanges()
{
    myHost->settings->SetSetting("fontProgFamily", ui->fontProgFamily->itemText(ui->fontProgFamily->currentIndex()) );
    myHost->settings->SetSetting("fontProgSize", static_cast<int>(ui->fontProgSize->value()));
    myHost->settings->SetSetting("fontProgBold", static_cast<int>(ui->fontProgBold->isChecked()));
    myHost->settings->SetSetting("fontProgItalic", static_cast<int>(ui->fontProgItalic->isChecked()));
    myHost->settings->SetSetting("fontProgStretch", static_cast<int>(ui->fontProgStretch->value()));

    if(!modified)
        return;

    if(ui->checkSavedInSetupFile->isChecked())
        myHost->SetSetupDirtyFlag();
    else
        conf->SaveInRegistry();

    modified=false;
}

void ViewConfigDialog::DiscardChanges()
{
    myHost->mainWindow->LoadProgramsFont();
    conf->SetListGroups( backupColors );
    conf->SetSavedInSetup( backupSaveInSetup );
    modified=false;
}

void ViewConfigDialog::LoadPreset(const QString &presetName)
{
    conf->LoadPreset(presetName);
    backupColors = *conf->GetCurrentPreset();
    backupSaveInSetup = conf->IsSavedInSetup();

    ui->listPalettes->clear();
    viewConfigPreset::iterator i = conf->GetCurrentPreset()->begin();
    while(i!=conf->GetCurrentPreset()->end()) {

        if(i.key() == ColorGroups::Theme) {
            if(i.value().firstKey() == Colors::Dark) {
                ui->themeDark->setChecked( true );
            } else {
                ui->themeLight->setChecked( true );
            }
        } else {
            QListWidgetItem *item = new QListWidgetItem( conf->GetColorGroupName(i.key()) );
            item->setData(Qt::UserRole+1,i.key());
            ui->listPalettes->addItem( item );
        }
        ++i;
    }

    QList<QListWidgetItem*> listItems = ui->listPresets->findItems(conf->GetPresetName(),Qt::MatchExactly);
    if(!listItems.isEmpty())
        ui->listPresets->setCurrentItem( listItems.first() );

    ui->listRoles->clear();

    DisableSliders();
    modified=false;
}

/*!
  Update color sliders position according to the selected color
  */
void ViewConfigDialog::GetColorFromConf()
{
    if(!ui->groupColor->isEnabled())
        return;

    currentColor = conf->GetColor( currentGrp, currentCol );

    updateInProgress=true;
    UpdateSliders();
    ui->AlphaSpinBox->setValue(currentColor.alpha());
    ui->picker->setMainColor(currentColor);
    ui->picker_hue->setMainColor(currentColor);
    updateInProgress=false;
}

void ViewConfigDialog::UpdateSliders()
{
    ui->RedSpinBox->setValue(currentColor.red());
    ui->GreenSpinBox->setValue(currentColor.green());
    ui->BlueSpinBox->setValue(currentColor.blue());
}

/*!
  Disable sliders, when no color selected
  */
void ViewConfigDialog::DisableSliders()
{
    currentCol = Colors::ND;
    ui->groupColor->setEnabled(false);
    ui->RedSlider->setValue(0);
    ui->GreenSlider->setValue(0);
    ui->BlueSlider->setValue(0);
    ui->AlphaSlider->setValue(0);
}

/*!
  Send the modified color to the config
  */
void ViewConfigDialog::SaveColor()
{
    if(currentGrp==ColorGroups::ND || currentCol==Colors::ND)
        return;
    modified=true;
    conf->SetColor(currentGrp,currentCol, currentColor);
}

void View::ViewConfigDialog::on_listPresets_clicked(const QModelIndex &index)
{
    if(!UserWantsToUnloadPreset()) {
        QListWidgetItem *item = ui->listPresets->findItems(conf->GetPresetName(),Qt::MatchExactly).first();
        if(item)
            ui->listPresets->setCurrentItem( item );

        return;
    }
    LoadPreset( index.data().toString() );
}

/*!
  A group was clicked, update the list of colors
  \param item the selected group item
  */
void View::ViewConfigDialog::on_listPalettes_itemClicked(QListWidgetItem* item)
{
    currentGrp = (ColorGroups::Enum)item->data(Qt::UserRole+1).toInt();

    ui->listRoles->clear();

    if(!conf->GetCurrentPreset()->contains(currentGrp))
        return;

    QMap<Colors::Enum,QColor> grp = conf->GetCurrentPreset()->value( currentGrp );

    bool colorExistsInGroup=false;
    int cpt=0;
    QMap<Colors::Enum,QColor>::iterator i = grp.begin();
    while(i!=grp.end()) {

        ui->listRoles->addItem( conf->GetColorName( i.key() ) );
        ui->listRoles->item(cpt)->setData(Qt::UserRole+1,i.key());

        if(i.key() == currentCol) {
            colorExistsInGroup=true;
            ui->listRoles->setCurrentRow(cpt);
        }

        cpt++;
        ++i;
    }

    if(colorExistsInGroup) {
        ui->groupColor->setEnabled(true);
        GetColorFromConf();
    } else {
        DisableSliders();
    }
}

/*!
  The selection changed, update the color sliders
  \param item the selected color item
  */
void View::ViewConfigDialog::on_listRoles_itemClicked(QListWidgetItem* item)
{
    currentCol = (Colors::Enum)item->data(Qt::UserRole+1).toInt();
    ui->groupColor->setEnabled(true);
    GetColorFromConf();
}

/*!
  SaveInSetup was clicked, load the colors from the registry or the setup file
  */
void View::ViewConfigDialog::on_checkSavedInSetupFile_clicked(bool checked)
{
    if(!UserWantsToUnloadPreset()) {
        ui->checkSavedInSetupFile->setChecked(!checked);
        return;
    }

    conf->SetSavedInSetup( checked );
    InitLists();
    LoadPreset("Default");
}


void View::ViewConfigDialog::on_RedSpinBox_valueChanged(int value)
{
    if(!StartUpdate())
        return;
    ui->picker->setRed(value);
    ui->picker_hue->setRed(value);
    EndUpdate();
}

void View::ViewConfigDialog::on_GreenSpinBox_valueChanged(int value)
{
    if(!StartUpdate())
        return;
    ui->picker->setGreen(value);
    ui->picker_hue->setGreen(value);
    EndUpdate();
}

void View::ViewConfigDialog::on_BlueSpinBox_valueChanged(int value)
{
    if(!StartUpdate())
        return;
    ui->picker->setBlue(value);
    ui->picker_hue->setBlue(value);
    EndUpdate();
}

void View::ViewConfigDialog::on_AlphaSpinBox_valueChanged(int value)
{
    if(!StartUpdate())
        return;
    ui->picker->setAlpha(value);
    ui->picker_hue->setAlpha(value);
    EndUpdate();
}

void ViewConfigDialog::onPickerColorSelected(const QColor &color)
{
    StartUpdate();
    currentColor=color;
    UpdateSliders();
    ui->picker_hue->setMainColor(currentColor);
    EndUpdate();
}

void ViewConfigDialog::onPickerHueSelected(const QColor &color)
{
    StartUpdate();
    currentColor=color;
    UpdateSliders();
    ui->AlphaSpinBox->setValue(ui->picker_hue->getAlpha()*255);
    ui->picker->setMainColor(currentColor);
    EndUpdate();
}

bool ViewConfigDialog::StartUpdate()
{
    if(updateInProgress)
        return false;
    updateInProgress=true;
    return true;
}

void ViewConfigDialog::EndUpdate()
{
    currentColor=ui->picker->getSelectedColor();
    SaveColor();
    updateInProgress=false;
}

void View::ViewConfigDialog::on_addPreset_clicked()
{
    if(!UserWantsToUnloadPreset())
        return;

    QString name(tr("New preset"));

    QListWidgetItem * item = ui->listPresets->currentItem();
    if(item) {
        name=item->text();
        conf->CopyPreset(name,name);
    } else {
        conf->AddPreset( name );
    }
    InitLists();
    LoadPreset(name);

    if(ui->checkSavedInSetupFile->isChecked())
        myHost->SetSetupDirtyFlag();
}

void View::ViewConfigDialog::on_delPreset_clicked()
{
    QListWidgetItem * item = ui->listPresets->currentItem();
    if(!item)
        return;
    if(item->text() == "Default")
        return;

    conf->RemovePreset( item->text() );
    InitLists();
    LoadPreset("Default");

    if(ui->checkSavedInSetupFile->isChecked())
        myHost->SetSetupDirtyFlag();
}

void View::ViewConfigDialog::on_listPresets_itemChanged(QListWidgetItem *item)
{
    QString oldName = item->data(Qt::UserRole+1).toString();
    QString newName = item->text();
    conf->RenamePreset( oldName, newName );

    InitLists();
    LoadPreset(newName);

    if(ui->checkSavedInSetupFile->isChecked())
        myHost->SetSetupDirtyFlag();
}

void ViewConfigDialog::UpdateProgramsFont()
{
    int b = QFont::Normal;
    if(ui->fontProgBold->isChecked())
        b = QFont::Bold;

    QFont f(
        ui->fontProgFamily->itemText(ui->fontProgFamily->currentIndex()),
        ui->fontProgSize->value(),
        b,
        ui->fontProgItalic->isChecked()
            );
    if(ui->fontProgSize->value()<=0 || ui->fontProgStretch->value()<=0)
        return;
    f.setPointSize(ui->fontProgSize->value());
    f.setStretch(ui->fontProgStretch->value());
    myHost->mainWindow->SetProgramsFont(f);
}

void View::ViewConfigDialog::on_fontProgItalic_clicked(bool /*checked*/)
{
    UpdateProgramsFont();
}

void View::ViewConfigDialog::on_fontProgStretch_valueChanged(double /*arg1*/)
{
    UpdateProgramsFont();
}

void View::ViewConfigDialog::on_fontProgSize_valueChanged(double /*arg1*/)
{
    UpdateProgramsFont();
}

void View::ViewConfigDialog::on_fontProgFamily_currentIndexChanged(const QString &/*arg1*/)
{
    UpdateProgramsFont();
}

void View::ViewConfigDialog::on_fontProgBold_clicked(bool /*checked*/)
{
    UpdateProgramsFont();
}

void View::ViewConfigDialog::on_themeLight_toggled(bool checked)
{
    modified=true;
    conf->SetTheme(checked ? Colors::Light : Colors::Dark);
}

