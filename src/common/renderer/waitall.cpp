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

#include "waitall.h"


WaitAll::WaitAll() :
    cptAtStart(0)
{
}

WaitAll::~WaitAll()
{
    QMutexLocker l(&mutex);
    condStart.wakeAll();
}

void WaitAll::AddClient()
{
    QMutexLocker l(&mutex);
    ++cptAtStart;
}

//void WaitAll::WakeAll()
//{
//    condStart.wakeAll();
//}

void WaitAll::RemoveClient()
{
    QMutexLocker l(&mutex);
    --cptAtStart;
    if(cptAtStart<=0) {
        condStart.wakeAll();
    }
}

//bool WaitAll::IsReady()
//{
//    QMutexLocker l(&mutex);
//    return (cptAtStart<=1);
//}

bool WaitAll::WaitAllThreads(int timeout)
{
    QMutexLocker l(&mutex);


    if(cptAtStart<=1) {
        cptAtStart=1;
        condStart.wakeAll();
        return true;
    }

    --cptAtStart;

    if(!condStart.wait(l.mutex(),timeout)) {
        ++cptAtStart;
        condStart.wakeAll();
        return false;
    }

    ++cptAtStart;
    return true;
}

