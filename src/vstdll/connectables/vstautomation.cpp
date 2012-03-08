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

#include "vstautomation.h"
#include "vstboardprocessor.h"
#include "commands/comaddpin.h"
#include "commands/comremovepin.h"

using namespace Connectables;

VstAutomation::VstAutomation(MainHost *myHost,int index) :
        Object(myHost,index, ObjectInfo(NodeType::object, ObjType::VstAutomation, tr("VstAutomation")) )
{
    for(int i=0;i<128;i++) {
        listValues << i;
    }

    //learning pin
    listIsLearning << "off";
    listIsLearning << "learn";
    listIsLearning << "unlearn";
    listParameterPinIn->AddPin(FixedPinNumber::learningMode);

    listParameterPinIn->AddPin(FixedPinNumber::numberOfPins);
    listParameterPinOut->AddPin(FixedPinNumber::vstProgNumber);

    listParameterPinIn->SetNbPins(VST_AUTOMATION_DEFAULT_NB_PINS);
    listParameterPinOut->SetNbPins(VST_AUTOMATION_DEFAULT_NB_PINS);

    connect(myHost,SIGNAL(HostChangedProg(int)),
            this,SLOT(OnHostChangedProg(int)));
}

VstAutomation::~VstAutomation()
{
    Close();
}


void VstAutomation::Render()
{
    if(!listChanged.isEmpty()) {
//        Steinberg::Vst::VstBoardProcessor *vst=static_cast<MainHostVst*>(myHost)->myVstPlugin;
        QHash<int,float>::const_iterator i = listChanged.constBegin();
        while(i!=listChanged.constEnd()) {
            if(listParameterPinOut->listPins.contains(i.key())) {
                static_cast<ParameterPin*>( listParameterPinOut->listPins.value(i.key()) )->ChangeValue(i.value());
            }
//            vst->updateParameter(i.key(),i.value());
            ++i;
        }
        listChanged.clear();
    }
}

void VstAutomation::OnHostChangedProg(int prog)
{
    static_cast<ParameterPin*>(listParameterPinOut->listPins.value(FixedPinNumber::vstProgNumber))->ChangeValue( prog, true );
}

void VstAutomation::ValueFromHost(int pinNum, float value)
{
    switch(GetLearningMode()) {
        case LearningMode::unlearn :
            {
                QUndoCommand *com = new QUndoCommand(tr("Remove pin"));
                if(listParameterPinOut->listPins.contains(pinNum)) {
                    new ComRemovePin(myHost, listParameterPinOut->listPins.value(pinNum)->GetConnectionInfo(),com);
                }
                if(listParameterPinIn->listPins.contains(pinNum)) {
                    new ComRemovePin(myHost, listParameterPinIn->listPins.value(pinNum)->GetConnectionInfo(),com);
                }
                if(com->childCount())
                    myHost->undoStack.push(com);
                else
                    delete com;
            }
            break;
        case LearningMode::learn :
            {
                QUndoCommand *com = new QUndoCommand(tr("Add pin"));
                if(!listParameterPinOut->listPins.contains(pinNum)) {
                    ConnectionInfo info = listParameterPinOut->connInfo;
                    info.pinNumber = pinNum;
                    info.isRemoveable = true;
                    new ComAddPin(myHost,info,com);
                }
                if(!listParameterPinIn->listPins.contains(pinNum)) {
                    ConnectionInfo info = listParameterPinIn->connInfo;
                    info.pinNumber = pinNum;
                    info.isRemoveable = true;
                    new ComAddPin(myHost,info,com);
                }
                if(com->childCount())
                    myHost->undoStack.push(com);
                else
                    delete com;
            }
        case LearningMode::off :
            if(listParameterPinOut->listPins.contains(pinNum))
                static_cast<ParameterPin*>(listParameterPinOut->listPins.value(pinNum))->ChangeValue( value, true );
            break;
    }
}

void VstAutomation::OnParameterChanged(ConnectionInfo pinInfo, float value)
{
    Object::OnParameterChanged(pinInfo,value);
    if(pinInfo.pinNumber==FixedPinNumber::numberOfPins) {
        int nbPins=static_cast<ParameterPin*>( listParameterPinIn->listPins.value(FixedPinNumber::numberOfPins) )->GetStepIndex();

        QUndoCommand *com = new QUndoCommand(tr("Change number of pins"));
        QList<quint16>listAdded;
        QList<quint16>listRemoved;

        listParameterPinOut->SetNbPins(nbPins,&listAdded,&listRemoved);
        qSort(listRemoved.begin(),listRemoved.end(),qGreater<quint16>());
        foreach(quint16 i, listRemoved)
            new ComRemovePin(myHost,listParameterPinOut->listPins.value(i)->GetConnectionInfo(),com);
        foreach(quint16 i, listAdded) {
            ConnectionInfo info = listParameterPinOut->connInfo;
            info.pinNumber = i;
            info.isRemoveable = true;
            new ComAddPin(myHost,info,com);
        }

        listAdded.clear();
        listRemoved.clear();

        listParameterPinIn->SetNbPins(nbPins,&listAdded,&listRemoved);
        qSort(listRemoved.begin(),listRemoved.end(),qGreater<quint16>());
        foreach(quint16 i, listRemoved)
            new ComRemovePin(myHost,listParameterPinIn->listPins.value(i)->GetConnectionInfo(),com);
        foreach(quint16 i, listAdded) {
            ConnectionInfo info = listParameterPinIn->connInfo;
            info.pinNumber = i;
            info.isRemoveable = true;
            new ComAddPin(myHost,info,com);
        }

        if(com->childCount())
            myHost->undoStack.push(com);
        else
            delete com;
        return;
    }

    if(pinInfo.direction==PinDirection::Input && pinInfo.pinNumber<200)
        listChanged.insert(pinInfo.pinNumber,value);
}

bool VstAutomation::Close()
{
//    static_cast<MainHostVst*>(myHost)->myVstPlugin->removeVstAutomation(this);
    return Object::Close();
}

bool VstAutomation::Open()
{
//    static_cast<MainHostVst*>(myHost)->myVstPlugin->addVstAutomation(this);
    return Object::Open();
}

Pin* VstAutomation::CreatePin(const ConnectionInfo &info)
{
    Pin *newPin = Object::CreatePin(info);
    if(newPin)
        return newPin;

    if(info.type!=PinType::Parameter) {
        LOG("wrong PinType"<<info.type);
        return 0;
    }

    switch(info.direction) {
        case PinDirection::Output :
//            if(info.pinNumber == FixedPinNumber::vstProgNumber) {
//                int prog=(int)static_cast<MainHostVst*>(myHost)->myVstPlugin->getProgram();
//                ParameterPin *newPin = new ParameterPinOut(this,FixedPinNumber::vstProgNumber,prog,&listValues,tr("Prog"));
//                newPin->SetLimitsEnabled(false);
//                return newPin;
//            }

            return new ParameterPinOut(this,info.pinNumber,0,QString("autom%1").arg(info.pinNumber),false,true);

        case PinDirection::Input :
            if(info.pinNumber == FixedPinNumber::numberOfPins) {
                ParameterPinIn *newPin = new ParameterPinIn(this,FixedPinNumber::numberOfPins,VST_AUTOMATION_DEFAULT_NB_PINS,&listValues,tr("NbPins"));
                newPin->SetLimitsEnabled(false);
                return newPin;
            }
            if(info.pinNumber == FixedPinNumber::learningMode) {
                ParameterPin *newPin = new ParameterPinIn(this,FixedPinNumber::learningMode,"off",&listIsLearning,tr("Learn"));
                newPin->SetLimitsEnabled(false);
                return newPin;
            }

            return new ParameterPinIn(this,info.pinNumber,0,QString("autom%1").arg(info.pinNumber),false,true);

        default :
            LOG("wrong PinDirection"<<info.direction);
            return 0;

    }
    return 0;
}
