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

#include "programlistwidget.h"
#include "mainhost.h"
#include "ui_programlistwidget.h"
//#include "models/programsmodel.h"
#include "models/groupsprogramsmodel.h"
#include "settings.h"

ProgramListWidget::ProgramListWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProgramListWidget),
    model(0)
{
    ui->setupUi(this);
}

ProgramListWidget::~ProgramListWidget()
{
    delete ui;
}

void ProgramListWidget::setFont(const QFont &f)
{
    ui->listProgs->setFont(f);
    ui->listGrps->setFont(f);
}

void ProgramListWidget::SetModel(GroupsProgramsModel *mod)
{
    if(!mod)
        return;

    model=mod;
    ui->listGrps->setModel(model);
    ui->listProgs->setModel(model);

    connect(ui->listGrps, SIGNAL(ShowProgList(QModelIndex)),
            ui->listProgs, SLOT(setRootIndex(QModelIndex)));

    //from model
    connect(model,SIGNAL(ProgAutosaveChanged(Qt::CheckState)),
            this,SLOT(OnProgAutoSaveChanged(Qt::CheckState)));
    connect(model,SIGNAL(GroupAutosaveChanged(Qt::CheckState)),
            this,SLOT(OnGroupAutoSaveChanged(Qt::CheckState)));

    //to model
    connect(this,SIGNAL(ProgAutoSave(Qt::CheckState)),
            model, SLOT(UserChangeProgAutosave(Qt::CheckState)));
    connect(this,SIGNAL(GroupAutoSave(Qt::CheckState)),
            model, SLOT(UserChangeGroupAutosave(Qt::CheckState)));
}

void ProgramListWidget::writeSettings(Settings *settings)
{
    settings->SetSetting("ProgramList/geometry", saveGeometry());
    settings->SetSetting("ProgramList/state", ui->splitter->saveState());
}

void ProgramListWidget::readSettings(Settings *settings)
{
    if(settings->SettingDefined("ProgramList/geometry")) {
        restoreGeometry(settings->GetSetting("ProgramList/geometry").toByteArray());
        ui->splitter->restoreState(settings->GetSetting("ProgramList/state").toByteArray());
    } else {
        resetSettings();
    }
}

void ProgramListWidget::resetSettings()
{
    int w = ui->splitter->width();
    QList<int>widths;
    widths << w << w;
    ui->splitter->setSizes(widths);
}

void ProgramListWidget::OnProgAutoSaveChanged(const Qt::CheckState state)
{
    ui->ProgAutosave->setCheckState( state );
}

void ProgramListWidget::OnGroupAutoSaveChanged(const Qt::CheckState state)
{
    ui->GroupAutosave->setCheckState(state);
}

void ProgramListWidget::on_GroupAutosave_stateChanged(int state)
{
    SetAutosaveToolTip( ui->GroupAutosave );
    emit GroupAutoSave( (Qt::CheckState)state );
}

void ProgramListWidget::on_ProgAutosave_stateChanged(int state)
{
    SetAutosaveToolTip( ui->ProgAutosave );
    emit ProgAutoSave( (Qt::CheckState)state );
}

void ProgramListWidget::SetAutosaveToolTip( QCheckBox *chkBox )
{
    switch( chkBox->checkState() ) {
        case Qt::Checked :
            chkBox->setToolTip(tr("Always save changes"));
            break;
        case Qt::Unchecked :
            chkBox->setToolTip(tr("Discard unsaved changes"));
            break;
        case Qt::PartiallyChecked :
            chkBox->setToolTip(tr("Prompt for unsaved changes"));
            break;
    }
}
