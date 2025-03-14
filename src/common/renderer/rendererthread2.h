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

#ifndef RENDERERTHREAD2_H
#define RENDERERTHREAD2_H

//#include "precomp.h"
#include "rendermap.h"

class Renderer2;
class RendererThread2 : public QThread
{
    Q_OBJECT

public:
    RendererThread2(Renderer2 *renderer, int id);
    ~RendererThread2();
    void run();
    void SetListOfNodes(const ThreadNodes &n);
    void ResetNodes();
    void LockAllSteps();
    void Stop();
    bool IsStopped();
    ThreadNodes currentNodes;

private:

    QMutex mutexNewNodes;
    bool stop;
    QMutex mutexStop;
    Renderer2 *renderer;
    int id;

#ifdef WIN32
    HMODULE DllAvRt;
    HANDLE hMmTask;
    typedef enum _PA_AVRT_PRIORITY
    {
        PA_AVRT_PRIORITY_LOW = -1,
        PA_AVRT_PRIORITY_NORMAL,
        PA_AVRT_PRIORITY_HIGH,
        PA_AVRT_PRIORITY_CRITICAL
    } PA_AVRT_PRIORITY, *PPA_AVRT_PRIORITY;
    typedef HANDLE WINAPI AVSETMMTHREADCHARACTERISTICS(LPCSTR, LPDWORD TaskIndex);
    typedef BOOL WINAPI AVREVERTMMTHREADCHARACTERISTICS(HANDLE);
    typedef BOOL WINAPI AVSETMMTHREADPRIORITY(HANDLE, PA_AVRT_PRIORITY);
    AVSETMMTHREADCHARACTERISTICS* FunctionAvSetMmThreadCharacteristics;
    AVREVERTMMTHREADCHARACTERISTICS* FunctionAvRevertMmThreadCharacteristics;
    AVSETMMTHREADPRIORITY* FunctionAvSetMmThreadPriority;
#endif

signals:
    void Timeout();
};

#endif // RENDERERTHREAD2_H
