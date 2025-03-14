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

#include "listaudiodevicesview.h"
#include "globals.h"


ListAudioDevicesView::ListAudioDevicesView(QWidget *parent) :
    QTreeView(parent),
    audioDevConfig(0),
    updateList(0)
{



    audioDevConfig = new QAction( QIcon::fromTheme("configure") , tr("Config device"), this);
    audioDevConfig->setShortcut(Qt::Key_F2);
    audioDevConfig->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(audioDevConfig, SIGNAL(triggered()),
            this, SLOT(ConfigCurrentDevice()));
    addAction(audioDevConfig);

    disableApi = new QAction( QIcon::fromTheme("dash") , tr("Disable API"), this);
    connect(disableApi, SIGNAL(triggered()),
            this, SLOT(DisableApi()));
    addAction(disableApi);

    enableApis = new QAction( QIcon::fromTheme("plus") , tr("Enable all APIs"), this);
    connect(enableApis, SIGNAL(triggered()),
            this, SLOT(EnableApis()));
    addAction(enableApis);

    updateList = new QAction( QIcon::fromTheme("view-refresh") , tr("Refresh list"), this);
    updateList->setShortcut(Qt::Key_F5);
    updateList->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(updateList, SIGNAL(triggered()),
            this, SIGNAL(UpdateList()));
    addAction(updateList);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested( const QPoint& )),
            this, SLOT(AudioDevContextMenu(const QPoint &)));
}

void ListAudioDevicesView::setModel(QAbstractItemModel *model)
{
    QTreeView::setModel(model);
    connect(this, SIGNAL(ApiDisabled(QModelIndex)),
            model,SLOT(ApiDisabled(QModelIndex)));
    connect(this, SIGNAL(ResetApis()),
            model, SLOT(ResetApis()));
    connect(this, SIGNAL(Config(QModelIndex)),
            model, SLOT(ConfigDevice(QModelIndex)));

    connect(model, SIGNAL(rowsInserted ( const QModelIndex &, int, int )),
            this, SLOT(expandAll()));
}

void ListAudioDevicesView::AudioDevContextMenu(const QPoint &pt)
{
    QList<QAction *> lstActions;

    if(currentIndex().parent().isValid() && currentIndex().parent().data(UserRoles::type).toString()=="api")
        lstActions << audioDevConfig;

    lstActions << updateList;

    if(currentIndex().data(UserRoles::type).toString()=="api") {
        disableApi->setChecked( indexAt(pt).data(UserRoles::enable).toBool() );
        lstActions << disableApi;
    }

    lstActions << enableApis;

    QMenu::exec(lstActions, mapToGlobal(pt),nullptr,this);
}

void ListAudioDevicesView::ConfigCurrentDevice()
{
    if(currentIndex().parent().isValid() && currentIndex().parent().data(UserRoles::type).toString()=="api")
        emit Config(currentIndex().sibling(currentIndex().row(),0));
}

void ListAudioDevicesView::DisableApi()
{
    emit ApiDisabled(currentIndex());
}

void ListAudioDevicesView::EnableApis()
{
    emit ResetApis();
}


