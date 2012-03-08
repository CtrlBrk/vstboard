/**************************************************************************
#    Copyright 2010-2012 Raphaël François
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

#ifndef RENDERERNODE2_H
#define RENDERERNODE2_H

#include "connectables/object.h"

class SemaphoreInverted;
class OptimizerNode;
class RendererNode2
{
public:
#ifdef TESTING
    RendererNode2(int id, int minRenderOrder, int maxRenderOrder, long cpuTime);
#endif
    RendererNode2(const OptimizerNode& c);
    RendererNode2(const RendererNode2& c);
    ~RendererNode2();
    void NewRenderLoop() const;
    void Render() const;
    void GetInfo(MsgObject &msg) const;

    qint32 id;
    qint32 minRenderOrder;
    qint32 maxRenderOrder;
    qint32 minRenderOrderOri;
    qint32 maxRenderOrderOri;
    mutable qint32 cpuTime;

    SemaphoreInverted *startSemaphore;
    SemaphoreInverted *nextStepSemaphore;

    QList< QWeakPointer<Connectables::Object> >listOfObj;
};

#endif // RENDERERNODE2_H
