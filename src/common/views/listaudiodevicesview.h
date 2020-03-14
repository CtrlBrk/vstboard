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

#ifndef LISTAUDIODEVICESVIEW_H
#define LISTAUDIODEVICESVIEW_H

#include "precomp.h"

class ListAudioDevicesView : public QTreeView
{
    Q_OBJECT
public:
    ListAudioDevicesView(QWidget *parent = 0);
    void setModel(QAbstractItemModel *model);

protected:
    QAction *audioDevConfig;
    QAction *updateList;
    QAction *disableApi;
    QAction *enableApis;

signals:
    void Config(const QModelIndex &device);
    void UpdateList();
    void ApiDisabled(const QModelIndex &api);
    void ResetApis();

public slots:
    void AudioDevContextMenu(const QPoint &pt);
private slots:
    void ConfigCurrentDevice();
    void DisableApi();
    void EnableApis();

};

#endif // LISTAUDIODEVICESVIEW_H
