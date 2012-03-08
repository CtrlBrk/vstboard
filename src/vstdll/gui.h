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

#ifndef GUI_H
#define GUI_H


#include <QObject>

//#include "aeffeditor.h"
#include "mainwindowvst.h"
//#include "resizehandle.h"
#include <qwinwidget.h>
#include "pluginterfaces/gui/iplugview.h"
#include "public.sdk/source/vst/vsteditcontroller.h"

class Settings;
namespace Steinberg
{

class Gui : public QObject, public FObject, public IPlugView
{
    Q_OBJECT

//    AudioEffectX* effect;

public:
    Gui(Vst::EditController *ctrl);
    ~Gui();

    void ReceiveMsg(const MsgObject &msg);

//    bool getRect (ERect** rect);
//    void SetMainWindow(MainWindowVst *win);

    tresult PLUGIN_API isPlatformTypeSupported (FIDString type);
    tresult PLUGIN_API attached (void* parent, FIDString type);
    tresult PLUGIN_API removed ();

    tresult PLUGIN_API onWheel (float /*distance*/) { return kResultFalse; }
    tresult PLUGIN_API onKeyDown (char16 /*key*/, int16 /*keyMsg*/, int16 /*modifiers*/) { return kResultFalse; }
    tresult PLUGIN_API onKeyUp (char16 /*key*/, int16 /*keyMsg*/, int16 /*modifiers*/) { return kResultFalse; }
    tresult PLUGIN_API getSize (ViewRect* size);
    tresult PLUGIN_API onSize (ViewRect* newSize);

    tresult PLUGIN_API onFocus (TBool /*state*/) { return kResultFalse; }
    tresult PLUGIN_API setFrame (IPlugFrame* frame);

    tresult PLUGIN_API canResize () { return kResultTrue; }
    tresult PLUGIN_API checkSizeConstraint (ViewRect* /*rect*/) { return kResultTrue; }

    //---Interface------
    OBJ_METHODS (Gui, FObject)
    DEFINE_INTERFACES
        DEF_INTERFACE (IPlugView)
    END_DEFINE_INTERFACES (FObject)
    REFCOUNT_METHODS(FObject)

protected:
    Vst::EditController *ctrl;
//    bool hostCanSizeWindow;
    Settings *settings;
    QWinWidget *widget;
    MainWindowVst *myWindow;
//    ResizeHandle *resizeH;
//    QPoint widgetOffset;
    IPlugFrame* plugFrame;

signals:
    void update(float value);

public slots:
    void UpdateColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color);
//    void OnResizeHandleMove(const QPoint &pt);

};
}
#endif // GUI_H
