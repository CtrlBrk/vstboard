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
#include "clappluginwindow.h"
#include "ui_vstpluginwindow.h"
#include "connectables/clapplugin.h"

using namespace View;


ClapPluginWindow::ClapPluginWindow(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::VstPluginWindow),
    canResize(false)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setWindowFlags(Qt::Tool);
    ui->setupUi(this);
}

ClapPluginWindow::~ClapPluginWindow()
{
    delete ui;
}

WId ClapPluginWindow::GetWinId()
{
    return ui->scrollAreaWidgetContents->winId();
}


void ClapPluginWindow::SetWindowSize(int newWidth, int newHeight)
{
    if(!canResize) {
        ui->scrollAreaWidgetContents->setFixedSize(newWidth,newHeight);
        setMaximumSize(newWidth,newHeight);
    }
    resize(newWidth,newHeight);
}


void ClapPluginWindow::resizeEvent ( QResizeEvent * event )
{
    // Connectables::ClapPlugin* p = qobject_cast<Connectables::ClapPlugin*>(plugin);
    // if(p) {
    //     if(canResize) {
    //         ViewRect r(0,0,event->size().width(),event->size().height());
    //         if(p->pView->checkSizeConstraint(&r) == kResultTrue) {
    //             p->pView->onSize(&r);
    //             return;
    //         }
    //     }
    // }

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

void ClapPluginWindow::closeEvent( QCloseEvent * event )
{
    // if(!plugin) {
    //     event->accept();
    //     return;
    // }

    hide();
    emit Hide();
    event->ignore();
}

void ClapPluginWindow::LoadAttribs(const ObjectContainerAttribs &attr)
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

void ClapPluginWindow::SaveAttribs(ObjectContainerAttribs &attr)
{
    attr.editorSize = size();
    attr.editorPosition = pos();
    attr.editorVScroll = ui->scrollArea->verticalScrollBar()->value();
    attr.editorHScroll = ui->scrollArea->horizontalScrollBar()->value();
}
