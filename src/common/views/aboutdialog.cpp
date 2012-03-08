/**************************************************************************
#    Copyright 2010-2012 Raphael Francois
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

#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
//    QString ver(APP_VERSION_MAJOR);

    ui->setupUi(this);
    ui->labelApp->setText(APP_NAME);
    ui->labelVersion->setText( QString("%1.%2.%3").arg(APP_VERSION_MAJOR).arg(APP_VERSION_MINOR).arg(APP_VERSION_BUILD) );
//    ui->labelBuild->setText(ver.section("-",2,2));

    setWindowTitle(QString(tr("About %1")).arg(APP_NAME));

    setFixedSize(size());
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
