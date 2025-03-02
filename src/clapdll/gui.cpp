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


Gui::Gui(MainHost* ctrl) :
    widget(0),
    myWindow(0)
{

#if defined(_M_X64) || defined(__amd64__)
    settings = new Settings("x64/plugin/",this);
#else
    settings = new Settings("x86/plugin/",this);
#endif

    myWindow = new MainWindowClap(ctrl,settings,0);
    myWindow->Init();
    connect( myWindow->viewConfig, SIGNAL(ColorChanged(ColorGroups::Enum,Colors::Enum,QColor)),
         this, SLOT(UpdateColor(ColorGroups::Enum,Colors::Enum,QColor)),
         Qt::UniqueConnection);
}

Gui::~Gui()
{
    if(myWindow) {
        myWindow->writeSettings();
        delete myWindow;
        myWindow=0;
    }
    if(widget) {
        //widget->deleteLater();
		delete widget;
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

void Gui::OnResizeHandleMove(const QPoint &pt)
{
    if(widget)
        widget->resize( pt.x(), pt.y() );

    if(myWindow)
        myWindow->resize(pt.x(), pt.y());

}


bool Gui::attached (void* parent)
{
    if(!myWindow)
        return kResultFalse;
#ifdef WIN32
    widget = new QWinWidget(static_cast<HWND>(parent));
#else
    widget = new QWidget();
#endif
    widget->setAutoFillBackground(false);
    widget->setObjectName("QWinWidget");
    myWindow->setParent(widget);
    myWindow->move(0,0);
    widget->move( 0, 0 );
    widget->resize( myWindow->size() );
    widget->setPalette( myWindow->palette() );

    // resizeH = new ResizeHandle(widget);
    // QPoint pos( widget->geometry().bottomRight() );
    // pos.rx()-=resizeH->width();
    // pos.ry()-=resizeH->height();
    // resizeH->move(pos);
    // resizeH->show();
    // connect(resizeH, SIGNAL(Moved(QPoint)),
            // this, SLOT(OnResizeHandleMove(QPoint)));

    widget->show();

    // Vst::EditorView::attachedToParent();
    return true;
}

bool Gui::removed ()
{
    return true;
}
/*

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
    return kResultTrue;
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

tresult PLUGIN_API Gui::setFrame (IPlugFrame* frame)
{
    plugFrame = frame;
    return kResultTrue;
}

*/

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
