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

#include "globals.h"
#include "version.h"
#include "splash.h"
#include "ui_splash.h"

Splash::Splash(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Splash)
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(this);
    ui->labelApp->setText(APP_NAME);
    ui->labelVersion->setText( APP_VERSION_STR );

    setWindowTitle(APP_NAME);

    setFixedSize(size());

}

Splash::~Splash()
{
    QSettings settings;
    if(ui->checkHideIt->isChecked()) {
        settings.setValue("splashHide", APP_VERSION_STR);
    }
    delete ui;
}
