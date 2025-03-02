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

#ifndef GUI_H
#define GUI_H


#include <QObject>

#include "mainwindowclap.h"
// #include "../vstdll/resizehandle.h"
#ifdef WIN32
    #include <qwinwidget.h>
#endif


class Settings;

class Gui : public QObject
{
    Q_OBJECT


public:
    Gui(MainHost* ctrl);
    ~Gui();

    void ReceiveMsg(const MsgObject &msg);

	MainWindow* GetWindow() { return myWindow; };
    bool attached (void* parent=0);
    bool removed ();

    void fromJson(QJsonObject &json);
    void toJson(QJsonObject &json) const;

protected:
    Settings *settings;

#ifdef WIN32
    QWinWidget *widget;
#else
    QWidget *widget;
#endif

    MainWindowClap *myWindow;
    // ResizeHandle *resizeH;

signals:
    void update(float value);

public slots:
    void UpdateColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color);
    void OnResizeHandleMove(const QPoint &pt);

};

#endif // GUI_H
