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
