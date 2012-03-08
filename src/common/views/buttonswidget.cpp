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

#include "buttonswidget.h"
#include "ui_buttonswidget.h"

ButtonsWidget::ButtonsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ButtonsWidget)
{
    ui->setupUi(this);
}

ButtonsWidget::~ButtonsWidget()
{
    delete ui;
}

void ButtonsWidget::SetData(int mb)
{
    ui->lmb->setChecked(mb & Qt::LeftButton);
    ui->mmb->setChecked(mb & Qt::MiddleButton);
    ui->rmb->setChecked(mb & Qt::RightButton);
    ui->mb4->setChecked(mb & Qt::XButton1);
    ui->mb5->setChecked(mb & Qt::XButton2);
}

int ButtonsWidget::GetData()
{
    int ret=0;
    if(ui->lmb->isChecked()) ret+=Qt::LeftButton;
    if(ui->mmb->isChecked()) ret+=Qt::MiddleButton;
    if(ui->rmb->isChecked()) ret+=Qt::RightButton;
    if(ui->mb4->isChecked()) ret+=Qt::XButton1;
    if(ui->mb5->isChecked()) ret+=Qt::XButton2;
    return ret;
}
