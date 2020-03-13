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

#include "hostcontroller.h"
#include "globals.h"
#include "mainhost.h"
#include "programmanager.h"

using namespace Connectables;

HostController::HostController(MainHost *myHost,int index):
    Object(myHost,index, ObjectInfo(NodeType::object, ObjType::HostController, tr("HostController") ) ),
    tempoChanged(false),
    progChanged(false),
    grpChanged(false),
    tapTrigger(false)
{

        for(int i=1;i<300;i++) {
            listTempo << i;
        }

        for(int i=2;i<30;i++) {
            listSign1 << i;
        }

        for(int i=0;i<9;i++) {
            listSign2 << (1<<i);
        }
        for(int i=0;i<128;i++) {
            listPrg << i;
        }
        for(int i=0;i<128;i++) {
            listGrp << i;
        }

    int tempo=120;
    int sign1=4;
    int sign2=4;
    myHost->GetTempo(tempo,sign1,sign2);

    listParameterPinIn->listPins.insert(Param_Tempo, new ParameterPinIn(this,Param_Tempo,tempo,&listTempo,"bpm"));
    listParameterPinIn->listPins.insert(Param_Sign1, new ParameterPinIn(this,Param_Sign1,sign1,&listSign1,"sign1"));
    listParameterPinIn->listPins.insert(Param_Sign2, new ParameterPinIn(this,Param_Sign2,sign2,&listSign2,"sign2"));
    listParameterPinIn->listPins.insert(Param_Group, new ParameterPinIn(this,Param_Group, myHost->programManager->GetCurrentMidiGroup(),&listGrp,"Group"));
    listParameterPinIn->listPins.insert(Param_Prog, new ParameterPinIn(this,Param_Prog, myHost->programManager->GetCurrentMidiProg(),&listPrg,"Prog"));
    listParameterPinIn->listPins.insert(Param_TapTempo, new ParameterPinIn(this,Param_TapTempo,.0f,"tapTempo"));

    listParameterPinOut->listPins.insert(Param_Tempo, new ParameterPinOut(this,Param_Tempo,tempo,&listTempo,"bpm"));
    listParameterPinOut->listPins.insert(Param_Sign1, new ParameterPinOut(this,Param_Sign1,sign1,&listSign1,"sign1"));
    listParameterPinOut->listPins.insert(Param_Sign2, new ParameterPinOut(this,Param_Sign2,sign2,&listSign2,"sign2"));
    listParameterPinOut->listPins.insert(Param_Group, new ParameterPinOut(this,Param_Group, myHost->programManager->GetCurrentMidiGroup(),&listGrp,"Group"));
    listParameterPinOut->listPins.insert(Param_Prog, new ParameterPinOut(this,Param_Prog, myHost->programManager->GetCurrentMidiProg(),&listPrg,"Prog"));
    listParameterPinOut->listPins.insert(Param_Bar, new ParameterPinOut(this,Param_Bar, 0,"Bar"));

    connect(this, SIGNAL(progChange(quint16)),
            myHost->programManager,SLOT(UserChangeProg(quint16)),
            Qt::QueuedConnection);
    connect(this, SIGNAL(grpChange(quint16)),
            myHost->programManager,SLOT(UserChangeGroup(quint16)),
            Qt::QueuedConnection);
    connect(this, SIGNAL(tempoChange(int,int,int)),
            myHost,SLOT(SetTempo(int,int,int)),
            Qt::QueuedConnection);

    connect(myHost->programManager,SIGNAL(MidiProgChanged(quint16)),
           this,SLOT(OnHostMidiProgChanged(quint16)));
    connect(myHost->programManager,SIGNAL(MidiGroupChanged(quint16)),
           this,SLOT(OnHostMidiGroupChanged(quint16)));
    connect(myHost,SIGNAL(TempoChanged(int,int,int)),
            this,SLOT(OnHostTempoChange(int,int,int)));
}

void HostController::Render()
{
    ParameterPin *pin=0;

    if(tempoChanged) {
        tempoChanged=false;
        int tempo=120;
        int sign1=4;
        int sign2=4;

        pin=static_cast<ParameterPin*>(listParameterPinIn->listPins.value(Param_Tempo));
        if(pin)
            tempo = pin->GetVariantValue().toInt();
        pin=static_cast<ParameterPin*>(listParameterPinIn->listPins.value(Param_Sign1));
        if(pin)
            sign1 = pin->GetVariantValue().toInt();
        pin=static_cast<ParameterPin*>(listParameterPinIn->listPins.value(Param_Sign2));
        if(pin)
            sign2 = pin->GetVariantValue().toInt();

        emit tempoChange(tempo,sign1,sign2);
    }

    if(progChanged) {
        progChanged=false;
        pin=static_cast<ParameterPin*>(listParameterPinIn->listPins.value(Param_Prog));
        if(pin)
            emit progChange( pin->GetVariantValue().toInt() );
    }
    if(grpChanged) {
        grpChanged=false;
        pin=static_cast<ParameterPin*>(listParameterPinIn->listPins.value(Param_Group));
        if(pin)
            emit grpChange( pin->GetVariantValue().toInt() );
    }

#ifdef VSTSDK
    pin=static_cast<ParameterPin*>(listParameterPinOut->listPins.value(Param_Bar));
    if(pin)
        pin->ChangeValue( myHost->vst3Host->GetCurrentBarTic() );
#endif
}

void HostController::OnParameterChanged(ConnectionInfo pinInfo, float value)
{
    Object::OnParameterChanged(pinInfo,value);

    if(pinInfo.direction!=PinDirection::Input)
        return;

    switch(pinInfo.pinNumber) {
        case Param_Tempo :
        case Param_Sign1 :
        case Param_Sign2 :
            tempoChanged=true;
            break;
        case Param_Group :
            grpChanged=true;
            break;
        case Param_Prog :
            progChanged=true;
            break;
        case Param_TapTempo :
            if(value>=0.5f && !tapTrigger) {
                tapTrigger=true;
                TapTempo();
            }
            if(value<0.5f)
                tapTrigger=false;
            break;
    }
}

void HostController::OnHostMidiProgChanged(quint16 prg)
{
    if(listParameterPinOut->listPins.contains(Param_Prog))
        static_cast<ParameterPin*>(listParameterPinOut->listPins.value(Param_Prog))->ChangeValue( prg, true );
}

void HostController::OnHostMidiGroupChanged(quint16 grp)
{
    if(listParameterPinOut->listPins.contains(Param_Group))
        static_cast<ParameterPin*>(listParameterPinOut->listPins.value(Param_Group))->ChangeValue( grp, true );
}

void HostController::OnHostTempoChange(int tempo, int sign1, int sign2)
{
    static_cast<ParameterPin*>(listParameterPinOut->listPins.value(Param_Tempo))->SetVariantValue( tempo);
    static_cast<ParameterPin*>(listParameterPinOut->listPins.value(Param_Sign1))->SetVariantValue( sign1 );
    static_cast<ParameterPin*>(listParameterPinOut->listPins.value(Param_Sign2))->SetVariantValue( sign2 );
}

void HostController::SetContainerId(quint16 id)
{
    switch(id) {
        case FixedObjId::programContainer :
            listParameterPinIn->RemovePin(Param_Prog);
            disconnect(this, SIGNAL(progChange(quint16)),
                    myHost->programManager,SLOT(UserChangeProg(quint16)));
            disconnect(myHost->programManager,SIGNAL(MidiProgChanged(quint16)),
                   this,SLOT(OnHostMidiProgChanged(quint16)));

        case FixedObjId::groupContainer :
            listParameterPinIn->RemovePin(Param_Group);
            disconnect(this, SIGNAL(grpChange(quint16)),
                    myHost->programManager,SLOT(UserChangeGroup(quint16)));
            disconnect(myHost->programManager,SIGNAL(MidiGroupChanged(quint16)),
                   this,SLOT(OnHostMidiGroupChanged(quint16)));
    }

    Object::SetContainerId(id);
}

void HostController::TapTempo()
{
    if(taps.size()>8)
        taps.removeFirst();

    float tapMoy=.0f;
    if(taps.size()>2) {
        for(int i=1; i<taps.size(); ++i) {
            tapMoy+=taps.at(i);
        }
        tapMoy/=(taps.size()-1);
    }

    if(tapMoy>1.0f && (tapTempoTimer.elapsed()>tapMoy*1.5 || tapTempoTimer.elapsed()<tapMoy*0.5)) {
        taps.clear();
    } else {
        taps<<tapTempoTimer.elapsed();
    }
    tapTempoTimer.restart();

//    double intPart;
//    double fractPart = modf (myHost->vstHost->vstTimeInfo.ppqPos, &intPart);
//    LOG(fractPart);

#ifdef VSTSDK
    myHost->vst3Host->processContext.projectTimeMusic = (int)myHost->vst3Host->processContext.projectTimeMusic;
#endif

    if(taps.size()>2) {
        float tempo=.0f;
        for(int i=1; i<taps.size(); ++i) {
            tempo+=taps.at(i);
        }
        tempo/=(taps.size()-1);
        if(tempo<=0)
            return;
        tempo=(1000*60)/tempo;
        if(tempo<1 || tempo>300)
            return;

        ParameterPin *pin=static_cast<ParameterPin*>(listParameterPinIn->listPins.value(Param_Tempo));
        if(!pin)
            return;
        pin->SetVariantValue((int)tempo);
    }
}

