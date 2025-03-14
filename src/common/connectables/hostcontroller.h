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

#ifndef HOSTCONTROLLER_H
#define HOSTCONTROLLER_H

//#include "precomp.h"
#include "object.h"

namespace Connectables {

    class HostController : public Object
    {
        enum Param {
            Param_Tempo,
            Param_Sign1,
            Param_Sign2,
            Param_Group,
            Param_Prog,
            Param_Bar,
            Param_TapTempo
        };

    Q_OBJECT
    public:
        explicit HostController(MainHost *myHost,int index);
        void Render();
        void SetContainerId(qint32 id);
        Pin* CreatePin(const ConnectionInfo &info);
    protected:
        QList<QVariant> listTempo;
        QList<QVariant> listSign1;
        QList<QVariant> listSign2;
        QList<QVariant> listPrg;
        QList<QVariant> listGrp;

        bool tempoChanged;
        bool progChanged;
        bool grpChanged;
        bool tapTempoChanged;

        QElapsedTimer tapTempoTimer;
        QList<int>taps;
        bool tapTrigger;

    signals:
        void progChange(quint16 prog);
        void grpChange(quint16 grp);
        void tempoChange(int tempo, int sign1=0, int sign2=0);

    public slots:
        void OnHostMidiProgChanged(quint16 prg);
        void OnHostMidiGroupChanged(quint16 grp);
        void OnHostTempoChange(int tempo, int sign1, int sign2);
        void OnParameterChanged(ConnectionInfo pinInfo, float value);
        void TapTempo();
    };

}


#endif // HOSTCONTROLLER_H
