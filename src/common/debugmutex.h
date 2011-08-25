/**************************************************************************
#    Copyright 2010-2011 Raphaël François
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

#ifndef DEBUGMUTEX_H
#define DEBUGMUTEX_H

#ifndef DEBUG_MUTEX
    #define DMutex QMutex
    #define SET_MUTEX_NAME(MUT,NAME);
#else

#include <QDebug>
#include "precomp.h"
#define DMutex DebugMutex
#define SET_MUTEX_NAME(MUT,NAME); MUT.name=NAME;


class DebugMutex : public QMutex
{
public:
    DebugMutex(RecursionMode mode = NonRecursive) :
        QMutex(mode),
        countUsage(0),
        countLocked(0),
        countLockedTime(0),
        maxLockedTime(0)
    {
        name=QString("%1").arg((long)this,16);
    }

    ~DebugMutex()
    {
#ifdef DEUBG_MUTEX_REPORT_UNUSED
        if(countUsage==0) {
            qDebug()<<name<<"unused mutex";
        }
#endif
        if(countUsage!=0) {
            qDebug()<<name<<"mutex usage:"<<countUsage
                      <<"locks:"<<countLocked
                      <<"time locked:"<<countLockedTime
                      <<"maxTimeLock:"<<maxLockedTime;
        }
    }

    void lock()
    {
        ++countUsage;

        if(!QMutex::tryLock()) {
            ++countLocked;
//            qDebug()<<name<<"wait locked";

            elapsed.restart();
            QMutex::lock();
            qint64 t = elapsed.elapsed();

            countLockedTime+=t;
            if(t>maxLockedTime)
                maxLockedTime=t;
        }
    }

    bool tryLock()
    {
        ++countUsage;

        if(!QMutex::tryLock()) {
            ++countLocked;
            qDebug()<<name<<"trylock failed";
            return false;
        }
        return true;
    }

    bool tryLock(int timeout)
    {
        ++countUsage;

        if(QMutex::tryLock()) {
            return true;
        }

        ++countLocked;
        qDebug()<<name<<"trylock locked";


        elapsed.restart();
        bool res=QMutex::tryLock(timeout);
        qint64 t = elapsed.elapsed();

        countLockedTime+=t;

        if(!res) {
            qDebug()<<name<<"trylock timeout"<<timeout<<t;
        } else {
            if(t>maxLockedTime)
                maxLockedTime=t;
        }

        return res;
    }

    void unlock()
    {
        QMutex::unlock();
    }

    QString name;

private:
    Q_DISABLE_COPY(DebugMutex)

    int countUsage;
    int countLocked;
    qint64 countLockedTime;
    qint64 maxLockedTime;
    QElapsedTimer elapsed;
};

#endif

#endif // DEBUGMUTEX_H
