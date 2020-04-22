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
#include "gui.h"
#include <QHBoxLayout>
#include "settings.h"
#include "projectfile/jsonwriter.h"
#include "projectfile/jsonreader.h"

namespace Steinberg
{
Gui::Gui(Vst::EditController *ctrl, ViewRect *size) :
    Vst::EditorView(ctrl,size),
    widget(0),
    myWindow(0),
    resizeH(0),
    plugFrame(0)
{


    //reaper needs an offset.. can't find a good solution
//    char str[64];
//    effect->getHostProductString(str);
//    if(!strcmp(str,"REAPER")) {
//        widgetOffset.setY(27);
//    }

//    hostCanSizeWindow = (bool)effect->canHostDo("sizeWindow");
//    if(!hostCanSizeWindow)
//        qDebug()<<"host can't resize window";



#if defined(_M_X64) || defined(__amd64__)
    settings = new Settings("x64/plugin/",this);
#else
    settings = new Settings("x86/plugin/",this);
#endif

    myWindow = new MainWindowVst(ctrl,settings,0);
    myWindow->Init();
    connect( myWindow->viewConfig, SIGNAL(ColorChanged(ColorGroups::Enum,Colors::Enum,QColor)),
         this, SLOT(UpdateColor(ColorGroups::Enum,Colors::Enum,QColor)),
         Qt::UniqueConnection);


}

Gui::~Gui()
{
    if(myWindow) {
        myWindow->writeSettings();
//        myWindow->deleteLater();
//        myWindow->setParent(0);
//        delete myWindow;
//        myWindow=0;
    }
    if(widget) {
        widget->deleteLater();
        widget=0;
    }
}

void Gui::UpdateColor(ColorGroups::Enum groupId, Colors::Enum /*colorId*/, const QColor & /*color*/)
{
    if(groupId!=ColorGroups::Window)
        return;

    if(widget)
        widget->setPalette( myWindow->palette() );
}

void Gui::ReceiveMsg(const MsgObject &msg)
{
    if(myWindow)
        myWindow->ReceiveMsg(msg);
}

//void Gui::SetMainWindow(MainWindowVst *win)
//{
//    if(win==myWindow)
//        return;

//    myWindow = win;
//    if(!myWindow)
//        return;

//    connect( myWindow->viewConfig, SIGNAL(ColorChanged(ColorGroups::Enum,Colors::Enum,QColor)),
//             this, SLOT(UpdateColor(ColorGroups::Enum,Colors::Enum,QColor)),
//             Qt::UniqueConnection);
//}

void Gui::OnResizeHandleMove(const QPoint &pt)
{
    widget->resize( pt.x(), pt.y() );
//    widget->move(widgetOffset);

    if(myWindow)
        myWindow->resize(pt.x(), pt.y());

////    if(effect)
////        effect->sizeWindow(pt.x(), pt.y());

//    rectangle.right = pt.x();
//    rectangle.bottom = pt.y();

    if(!plugFrame)
        return;

    ViewRect size(0,0,pt.x(),pt.y());
    plugFrame->resizeView(this,&size);
}

tresult PLUGIN_API Gui::isPlatformTypeSupported (FIDString type)
{
#if WINDOWS
        if (strcmp (type, kPlatformTypeHWND) == 0)
                return kResultTrue;

#elif MAC
        #if MAC_CARBON
        if (strcmp (type, kPlatformTypeHIView) == 0)
                return kResultTrue;
        #endif

        #if MAC_COCOA
        if (strcmp (type, kPlatformTypeNSView) == 0)
                return kResultTrue;
        #endif
#endif

        return kInvalidArgument;
}

tresult PLUGIN_API Gui::attached (void* parent, FIDString /*type*/)
{
    if(!myWindow)
        return kResultFalse;

    widget = new QWinWidget(static_cast<HWND>(parent));
    widget->setAutoFillBackground(false);
    widget->setObjectName("QWinWidget");
    myWindow->setParent(widget);
    myWindow->move(0,0);
    widget->move( 0, 0 );
    widget->resize( myWindow->size() );
    widget->setPalette( myWindow->palette() );

//    AEffEditor::open(ptr);

    resizeH = new ResizeHandle(widget);
    QPoint pos( widget->geometry().bottomRight() );
    pos.rx()-=resizeH->width();
    pos.ry()-=resizeH->height();
    resizeH->move(pos);
    resizeH->show();

    connect(resizeH, SIGNAL(Moved(QPoint)),
            this, SLOT(OnResizeHandleMove(QPoint)));

    widget->show();

    Vst::EditorView::attachedToParent();
    return kResultOk;
}

tresult PLUGIN_API Gui::removed ()
{
//    if(myWindow) {
//        myWindow->writeSettings();
//        myWindow->setParent(0);
//        myWindow->deleteLater();
//        delete myWindow;
//        myWindow=0;
//    }
//    if(widget) {
//        widget->setParent(0);
//        widget->deleteLater();
//        delete widget;
//        widget=0;
//    }
//	myWindow = 0;
    Vst::EditorView::removedFromParent();
    return kResultOk;
}

tresult PLUGIN_API Gui::getSize (ViewRect* size)
{
    if (!size)
        return kInvalidArgument;

    if(widget) {
        size->left=0;
        size->top=0;
        size->right=widget->width();
        size->bottom=widget->height();
        return kResultTrue;
    }

    size->right = 600;
    size->bottom = 400;
    return kResultFalse;
}

tresult PLUGIN_API Gui::onSize (ViewRect* newSize)
{
    if (!newSize)
        return kInvalidArgument;

    if(myWindow) {
        myWindow->resize(newSize->getWidth(),newSize->getHeight());
    }
    if(widget) {
        widget->resize(newSize->getWidth(),newSize->getHeight());
//        widget->move(widgetOffset);

        if(resizeH) {
            QPoint pos( widget->geometry().bottomRight() );
            pos.rx()-=resizeH->width();
            pos.ry()-=resizeH->height();
            resizeH->move(pos);
        }
    }

    return kResultTrue;
}

//bool Gui::getRect (ERect** rect)
//{
//    if(!widget || !rect)
//        return false;

//    *rect = &rectangle;
//    return true;
//}

tresult PLUGIN_API Gui::setFrame (IPlugFrame* frame)
{
    plugFrame = frame;
    return kResultTrue;
}



}

void Gui::fromJson(QJsonObject &json)
{
	if(myWindow)
		JsonReader::readProjectView(json, myWindow, 0);
}

void Gui::toJson(QJsonObject &json) const
{
	if(myWindow)
		json = JsonWriter::writeProjectView(myWindow, true, true);
}