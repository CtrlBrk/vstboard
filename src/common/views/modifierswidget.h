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

#ifndef MODIFIERSWIDGET_H
#define MODIFIERSWIDGET_H

#include <QWidget>

namespace Ui {
    class ModifiersWidget;
}

class ModifiersWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ModifiersWidget(QWidget *parent = 0);
    ~ModifiersWidget();
    void SetData(int mb);
    int GetData();
private:
    Ui::ModifiersWidget *ui;
};

#endif // MODIFIERSWIDGET_H
