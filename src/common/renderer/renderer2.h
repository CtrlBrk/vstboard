/**************************************************************************
#    Copyright 2010-2012 RaphaÃ«l FranÃ§ois
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

#ifndef RENDERER2_H
#define RENDERER2_H

#include "semaphoreinverted.h"
#include "waitall.h"
#include "rendermap.h"

class RendererThread2;
class Renderer2 : public QObject
{
    Q_OBJECT
public:
    Renderer2(QObject *parent=0);
    ~Renderer2();
    void SetMap(const RenderMap &rMap, int nbThreads);
    void StartRender();
//    void SetEnabled(bool enab) {LOG("enable here"<<enab)}
    RenderMap currentMap;

private:
    void ThreadCleanup();
    void ChangeNbOfThreads(int newNbThreads);
    QList<RendererThread2*>threads;
    QList<RendererThread2*>threadsToDelete;
    QMutex mutexThreadList;
    int nbThreads;
    int nbSteps;

    WaitAll waitThreadReady;
    WaitAll waitThreadEnd;
    QList<SemaphoreInverted*>stepCanStart;

    QTimer signalTimeoutTimer;

signals:
    void Timeout();
public slots:
    void OnThreadTimeout();

friend class RendererThread2;
};

#endif // RENDERER2_H
