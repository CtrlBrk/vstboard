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

#ifndef PROGRAMLISTWIDGET_H
#define PROGRAMLISTWIDGET_H

#include "globals.h"
#include "mainhost.h"

namespace Ui {
    class ProgramListWidget;
}

class GroupsProgramsModel;
class Settings;
class ProgramListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ProgramListWidget(QWidget *parent = 0);
    ~ProgramListWidget();

    void SetModel(GroupsProgramsModel *model);
    void writeSettings(Settings *settings);
    void readSettings(Settings *settings);
    void resetSettings();

    void setFont(const QFont &);

private:
    Ui::ProgramListWidget *ui;
    GroupsProgramsModel *model;
    void SetAutosaveToolTip( QCheckBox *chkBox );

signals:
    void ProgAutoSave(const Qt::CheckState state);
    void GroupAutoSave(const Qt::CheckState state);

public slots:
    void OnProgAutoSaveChanged(const Qt::CheckState state);
    void OnGroupAutoSaveChanged(const Qt::CheckState state);

private slots:
    void on_GroupAutosave_stateChanged(int state);
    void on_ProgAutosave_stateChanged(int state);
};

#endif // PROGRAMLISTWIDGET_H
