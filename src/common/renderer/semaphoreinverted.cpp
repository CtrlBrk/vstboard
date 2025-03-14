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

#include "semaphoreinverted.h"

SemaphoreInverted::SemaphoreInverted() :
    lockCpt(0)
{
}

SemaphoreInverted::~SemaphoreInverted()
{
}

void SemaphoreInverted::AddLock(int n)
{
    QWriteLocker locker(&mutex);
    lockCpt+=n;
}

void SemaphoreInverted::Unlock(int n)
{
    QWriteLocker locker(&mutex);
//    if(n==-1) {
//        lockCpt=0;
//    } else {
        lockCpt-=n;
//    }

    if(lockCpt<=0) {
        lockCpt=0;
        condUnlock.wakeAll();
    }
}

void SemaphoreInverted::WaitUnlock()
{
    QReadLocker locker(&mutex);
    if(lockCpt==0)
        return;

    condUnlock.wait(&mutex);
}

bool SemaphoreInverted::IsLocked()
{
    QReadLocker locker(&mutex);
    return (lockCpt!=0);
}

bool SemaphoreInverted::WaitUnlock(int timeout)
{
    QReadLocker locker(&mutex);
    if(lockCpt==0)
        return true;

    QElapsedTimer timer;
    timer.start();
    while (lockCpt!=0) {
        qint64 elapsed = timer.elapsed();
        if (timeout - elapsed <= 0 || !condUnlock.wait(&mutex, timeout - elapsed))
            return false;
    }
    return true;
}

bool SemaphoreInverted::WaitUnlock(int timeout, int &timelocked)
{
    QReadLocker locker(&mutex);
    if(lockCpt==0)
        return true;

    QElapsedTimer timer;
    timer.start();
    while (lockCpt!=0) {
        qint64 elapsed = timer.elapsed();
        if (timeout - elapsed <= 0 || !condUnlock.wait(&mutex, timeout - elapsed))
            return false;
    }
    timelocked=timer.elapsed();
    return true;
}

