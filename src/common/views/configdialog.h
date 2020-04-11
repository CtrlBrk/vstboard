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

#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

//#include "../precomp.h"
#include "settings.h"

namespace Ui {
    class ConfigDialog;
}
class MainHost;
class ConfigDialog : public QDialog {
    Q_OBJECT
public:
    ConfigDialog(Settings *settings, MainHost *myHost=0, QWidget *parent = 0);
    ~ConfigDialog();

    static const QString defaultSetupFile(Settings *settings);
    static const QString defaultProjectFile(Settings *settings);
    static const QString defaultVstPath(Settings *settings);
    static const QString defaultBankPath(Settings *settings);
    static void AddRecentSetupFile(const QString &file,Settings *settings);
    static void AddRecentProjectFile(const QString &file,Settings *settings);
    static void RemoveRecentSetupFile(const QString &file,Settings *settings);
    static void RemoveRecentProjectFile(const QString &file,Settings *settings);

    static const float defaultSampleRate(Settings *settings);
    static const int defaultBufferSize(Settings *settings);
    static const bool defaultDoublePrecision(Settings *settings);

protected:
    void changeEvent(QEvent *e);
    Ui::ConfigDialog *ui;
    Settings *settings;
    MainHost *myHost;

public slots:
    void accept();
    void reject();

private slots:
    void on_browseProject_clicked();
    void on_browseSetup_clicked();
    void on_browseVst_clicked();
    void onVstPathIndexChanged(int index);
    void onBankPathIndexChanged(int index);
    void onSetupIndexChanged(int index);
    void onProjectIndexChanged(int index);
    void on_browseBank_clicked();
};

#endif // CONFIGDIALOG_H
