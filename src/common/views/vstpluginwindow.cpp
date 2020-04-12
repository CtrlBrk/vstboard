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

#ifdef VSTSDK

#include "vstpluginwindow.h"
#include "ui_vstpluginwindow.h"
#include "connectables/vstplugin.h"
#include "connectables/vst3plugin.h"
#include "mainhost.h"

using namespace View;

VstPluginWindow::VstPluginWindow(QWidget *parent) :
    QFrame(parent),
    IPlugFrame(),
    plugin(0),
    ui(new Ui::VstPluginWindow),
    canResize(false)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setWindowFlags(Qt::Tool);
    ui->setupUi(this);
}

VstPluginWindow::~VstPluginWindow()
{
    delete ui;
}

//void VstPluginWindow::changeEvent(QEvent *e)
//{
//    QFrame::changeEvent(e);
//    switch (e->type()) {
//    case QEvent::LanguageChange:
//        ui->retranslateUi(this);
//        break;
//    default:
//        break;
//    }
//}

WId VstPluginWindow::GetWinId()
{
    return ui->scrollAreaWidgetContents->winId();
}

void  VstPluginWindow::UnsetPlugin()
{
    this->plugin = 0;
}

bool VstPluginWindow::SetPlugin(Connectables::Vst3Plugin *plug)
{
    plugin = plug;
    canResize = (plug->pView->canResize() == kResultTrue);

	ViewRect rec(0, 0, 600, 600);
	if (plug->pView->checkSizeConstraint(&rec) == kResultTrue) {
		SetWindowSize(rec.getWidth(), rec.getHeight());

		//tresult r = plug->pView->onSize(&rec);
		//LOG(r);
	}

    return true;
}

bool VstPluginWindow::SetPlugin(Connectables::VstPlugin *plugin)
{
    bool windowOk = false;
    this->plugin = plugin;

    if(!plugin)
        return false;

    setWindowFlags(Qt::Tool);

    long res;
    ERect *rect = 0;

    //try to get rect (some plugins wants it before editopen)
    res = plugin->EffEditGetRect(&rect);
    if(res == 1L)
        windowOk=true;

    //try to open the window
//    plugin->objMutex.lock();
    res = plugin->EffEditOpen((void *)ui->scrollAreaWidgetContents->winId());
//    plugin->objMutex.unlock();

    if(res == 1L)
        windowOk=true;

    //try to get rect again
    res = plugin->EffEditGetRect(&rect);
    if(res == 1L)
        windowOk=true;

    if(!windowOk)
        return false;

    SetWindowSize(rect->right, rect->bottom);

    return true;
}

void VstPluginWindow::LoadAttribs(const ObjectContainerAttribs &attr)
{
    if(attr.editorSize == QSize(0,0))
        return;

    resize( attr.editorSize );
    move( attr.editorPosition );
    ui->scrollArea->verticalScrollBar()->setValue( attr.editorVScroll );
    ui->scrollArea->horizontalScrollBar()->setValue( attr.editorHScroll );
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void VstPluginWindow::SaveAttribs(ObjectContainerAttribs &attr)
{
    attr.editorSize = size();
    attr.editorPosition = pos();
    attr.editorVScroll = ui->scrollArea->verticalScrollBar()->value();
    attr.editorHScroll = ui->scrollArea->horizontalScrollBar()->value();
}

void VstPluginWindow::closeEvent( QCloseEvent * event )
{
    if(!plugin) {
        event->accept();
        return;
    }

    hide();
    emit Hide();
    event->ignore();
}

void VstPluginWindow::SetWindowSize(int newWidth, int newHeight)
{
    if(!canResize) {
        ui->scrollAreaWidgetContents->setFixedSize(newWidth,newHeight);
        setMaximumSize(newWidth,newHeight);
    }
    resize(newWidth,newHeight);
}

void VstPluginWindow::showEvent ( QShowEvent * /*event*/ )
{
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    int w = ui->scrollAreaWidgetContents->width();
    int h = ui->scrollAreaWidgetContents->height();
    if(!canResize) {
        setMaximumSize(w,h);
    }
}

void VstPluginWindow::resizeEvent ( QResizeEvent * event )
{
    Connectables::Vst3Plugin* p = qobject_cast<Connectables::Vst3Plugin*>(plugin);
    if(p) {
        if(canResize) {
            ViewRect r(0,0,event->size().width(),event->size().height());
            if(p->pView->checkSizeConstraint(&r) == kResultTrue) {
                p->pView->onSize(&r);
                return;
            }
        }
    }

    int maxH=ui->scrollAreaWidgetContents->height();
    int maxW=ui->scrollAreaWidgetContents->width();

    if( event->size().width() >= maxW &&
        event->size().height() >= maxH &&
        ui->scrollArea->horizontalScrollBarPolicy()!=Qt::ScrollBarAlwaysOff) {

        ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setMaximumSize(maxW,maxH);
    } else
        if(ui->scrollArea->horizontalScrollBarPolicy()!=Qt::ScrollBarAsNeeded) {

        ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        maxW+=ui->scrollArea->verticalScrollBar()->width();
        maxH+=ui->scrollArea->horizontalScrollBar()->height();
        setMaximumSize(maxW,maxH);
    }
}


//const QPixmap VstPluginWindow::GetScreenshot()
//{
//    QPixmap pix = QPixmap::grabWindow(ui->scrollAreaWidgetContents->winId() ,0,0,size().width(),size().height());//45);
//    QPainter p(&pix);
//    p.fillRect(pix.rect(), QColor(255,255,255,150));
//    return pix;
//}

tresult PLUGIN_API VstPluginWindow::resizeView (IPlugView* view, ViewRect* newSize)
{
    SetWindowSize(newSize->getWidth(), newSize->getHeight());
    return kResultOk;
}


#endif
