/**************************************************************************
#    Copyright 2010-2020 Raphaël François
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


#include "parameterpin.h"
#include "object.h"
#include "../globals.h"
#include "mainhost.h"

using namespace Connectables;

ParameterPin::ParameterPin(const pinConstructArgs &conf) :
	Pin(conf),
	listValues(conf.listValues),
	stepIndex(0),
    defaultVisible(conf.visible),
//	defaultValue(conf.defaultValue),
	defaultIndex(0),
	loading(false),
	nameCanChange(conf.nameCanChange),
	dirty(false),
	limitInMin(0),
	limitInMax(0),
	limitOutMin(0),
	limitOutMax(0),
	outStepIndex(0),
	outValue(.0f),
	limitsEnabled(true)
{
	InitCursors();
	setObjectName(conf.name);
	
	loading = true;

	if (listValues != 0) {
		stepSize = 1.0f / (listValues->size() - 1);
		stepIndex = listValues->indexOf(conf.defaultVariantValue);
		defaultIndex = stepIndex;
		//update display
		OnValueChanged(.0f);
		UpdateView();
	}
	else 
	{
        defaultValue = value;
		OnValueChanged(defaultValue);
	}

	loading = false;
}

ParameterPin::~ParameterPin()
{
    delete limitInMin;
    delete limitInMax;
    delete limitOutMin;
    delete limitOutMax;
}

void ParameterPin::InitCursors()
{
    limitInMin = new Cursor(parent->getHost(),.0f);
    limitInMax = new Cursor(parent->getHost(),1.0f);
    limitOutMin = new Cursor(parent->getHost(),.0f);
    limitOutMax = new Cursor(parent->getHost(),1.0f);

    connect(limitInMin, SIGNAL(valueChanged(float)),
            this, SLOT(CursorValueChanged(float)));
    connect(limitInMax, SIGNAL(valueChanged(float)),
            this, SLOT(CursorValueChanged(float)));
    connect(limitOutMin, SIGNAL(valueChanged(float)),
            this, SLOT(CursorValueChanged(float)));
    connect(limitOutMax, SIGNAL(valueChanged(float)),
            this, SLOT(CursorValueChanged(float)));
}

void ParameterPin::ReceiveMsg(const MsgObject &msg)
{
    if(msg.prop.contains(MsgObject::Value)) {
        ChangeValue( msg.prop[MsgObject::Value].toFloat() );
    }
    if(msg.prop.contains(MsgObject::Increment)) {
        ChangeValue( value + stepSize * msg.prop[MsgObject::Increment].toInt() );
    }

    Pin::ReceiveMsg(msg);
}

void ParameterPin::SetRemoveable()
{
    connectInfo.isRemoveable=true;
}

void ParameterPin::GetDefault(ObjectParameter &param)
{
    param.index=defaultIndex;
    param.value=defaultValue;
    param.visible=defaultVisible;
}

void ParameterPin::GetValues(ObjectParameter &param)
{
    param.index=stepIndex;
    param.value=value;
    param.visible=visible;
    param.limitInMin=limitInMin->GetValue();
    param.limitInMax=limitInMax->GetValue();
    param.limitOutMin=limitOutMin->GetValue();
    param.limitOutMax=limitOutMax->GetValue();
}

void ParameterPin::ChangeOutputValue(float val, bool fromObj)
{
    val = std::min(val,1.0f);
    val = std::max(val,.0f);

    //scale value
    val-=limitOutMin->GetValue();
    val/=(limitOutMax->GetValue()-limitOutMin->GetValue());
    if(limitInMax->GetValue()!=limitInMin->GetValue())
        val*=(limitInMax->GetValue()-limitInMin->GetValue());
    val+=limitInMin->GetValue();

//    if(val>limitInMax->GetValue())
//        val=limitInMax->GetValue();
//    if(val<limitInMin->GetValue())
//        val=limitInMin->GetValue();

    ChangeValue(val,fromObj);


}

//from float
void ParameterPin::ChangeValue(float val, bool fromObj)
{
    if(listValues) {
        //we got a float, but we need a int
        ChangeValue((int)( 0.5f + val/stepSize ));
        return;
    }

    val = std::min(val,1.0f);
    val = std::max(val,.0f);

    if(!loading && std::abs(val-value)<0.00001f)
        return;

    OnValueChanged(val);

    if(!fromObj)
        parent->OnParameterChanged(connectInfo,outValue);

    UpdateView();
}

//from int
void ParameterPin::ChangeValue(int index, bool fromObj)
{
	if (listValues != 0) {
		index = std::min(index, listValues->size() - 1);
	}
	else {
		LOG("listValues not defined");
	}
    
    index = std::max(index,0);

    if(!loading && index==stepIndex)
        return;

    stepIndex=index;
    OnValueChanged(stepSize*index);

    if(!fromObj)
        parent->OnParameterChanged(connectInfo,outValue);
    UpdateView();
}

//from variant
void ParameterPin::ChangeValue(const QVariant &variant, bool fromObj)
{
    if(listValues)
        ChangeValue(listValues->indexOf(variant),fromObj);
    else
        ChangeValue(variant.toFloat(),fromObj);
}

void ParameterPin::SetVariantValue(const QVariant &val)
{
    if(!listValues) {
        ChangeValue(val.toFloat());
        return;
    }

    int index=listValues->indexOf(val);
    if(index==-1)
        return;

    ChangeValue(listValues->value(index));
}

void ParameterPin::OnStep(int delta)
{
    if(listValues) {
        ChangeValue(stepIndex + delta);
    } else {
        ChangeValue(value + delta*stepSize);
    }

}

//load from program
void ParameterPin::Load(const ObjectParameter &param)
{
    loading = true;

    limitInMin->SetValue(param.limitInMin);
    limitInMax->SetValue(param.limitInMax);
    limitOutMin->SetValue(param.limitOutMin);
    limitOutMax->SetValue(param.limitOutMax);

    if(listValues)
        ChangeValue(param.index);
    else
        ChangeValue(param.value);

    SetVisible(param.visible);

    dirty = false;
    loading = false;
}

void ParameterPin::OnValueChanged(float val)
{
//    float test = std::abs(value-val);
//    if(std::abs(value-val)>0.001f)
        valueChanged=true;

    value=val;

    //scale value
    if(val>limitInMax->GetValue())
        val=limitInMax->GetValue();
    if(val<limitInMin->GetValue())
        val=limitInMin->GetValue();
    val-=limitInMin->GetValue();
    if(limitInMax->GetValue()!=limitInMin->GetValue())
        val/=(limitInMax->GetValue()-limitInMin->GetValue());
    val*=(limitOutMax->GetValue()-limitOutMin->GetValue());
    val+=limitOutMin->GetValue();
    outValue=val;
    outStepIndex=(int)( 0.5f + outValue/stepSize );

    if(!loading && !dirty && connectInfo.direction==PinDirection::Input) {
        dirty=true;
        parent->OnProgramDirty();
    }
}

void ParameterPin::UpdateView()
{
    if(!visible)
        return;

    if(nameCanChange)
        displayedText = parent->GetParameterName(connectInfo);

    if(listValues && listValues->count()>outStepIndex) {
        displayedText = QString("%1:%2").arg(objectName()).arg(listValues->at(outStepIndex).toString());
    }
}

void ParameterPin::SetFixedName(QString fixedName)
{
    setObjectName(fixedName);
    nameCanChange=false;
}

void ParameterPin::SetLimit(ObjType::Enum type, float newVal)
{
//    switch(type) {
//        case ObjType::limitInMin:
//            limitInMin->value = newVal;
//            if(limitInMax->value<=limitInMin->value) {
//                limitInMax->value=limitInMin->value+0.005;
//                if(indexLimitInMax.isValid())
//                    parent->getHost()->GetModel()->setData( indexLimitInMax, limitInMax->value, UserRoles::value);
//            }
//            break;
//        case ObjType::limitInMax:
//            limitInMax->value = newVal;
//            if(limitInMax->value<=limitInMin->value) {
//                limitInMin->value=limitInMax->value-0.005;
//                if(indexLimitInMin.isValid())
//                    parent->getHost()->GetModel()->setData( indexLimitInMin, limitInMin->value, UserRoles::value);
//            }
//            break;
//        case ObjType::limitOutMin:
//            limitOutMin->value = newVal;
//            break;
//        case ObjType::limitOutMax:
//            limitOutMax->value = newVal;
//            break;
//        default:
//            LOG("unknown cursor type"<<type)
//            return;
//    }

//    OnValueChanged(value);
//    parent->OnParameterChanged(connectInfo,outValue);
//    UpdateView();
//    if(!dirty) {
//        dirty=true;
//        parent->OnProgramDirty();
//    }
}

void ParameterPin::SetVisible(bool vis)
{
//    if(visible==vis)
//        return;

    Pin::SetVisible(vis);

    if(limitsEnabled) {
//        QStandardItem *pinItem = parent->getHost()->GetModel()->itemFromIndex(modelIndex);
//        if(!pinItem)
//            return;

//        ObjectInfo info;
//        info.nodeType=NodeType::cursor;


//        QStandardItem *item = new QStandardItem("limitInMin");
//        info.objType=ObjType::limitInMin;
//        item->setData(QVariant::fromValue(info),UserRoles::objInfo);
//        item->setData(limitInMin->GetValue(),UserRoles::value);
//        pinItem->appendRow(item);
//        indexLimitInMin=item->index();

//        item = new QStandardItem("limitInMax");
//        info.objType=ObjType::limitInMax;
//        item->setData(QVariant::fromValue(info),UserRoles::objInfo);
//        item->setData(limitInMax->GetValue(),UserRoles::value);
//        pinItem->appendRow(item);
//        indexLimitInMax=item->index();

//        item = new QStandardItem("limitOutMin");
//        info.objType=ObjType::limitOutMin;
//        item->setData(QVariant::fromValue(info),UserRoles::objInfo);
//        item->setData(limitOutMin->GetValue(),UserRoles::value);
//        pinItem->appendRow(item);
//        indexLimitOutMin=item->index();

//        item = new QStandardItem("limitOutMax");
//        info.objType=ObjType::limitOutMax;
//        item->setData(QVariant::fromValue(info),UserRoles::objInfo);
//        item->setData(limitOutMax->GetValue(),UserRoles::value);
//        pinItem->appendRow(item);
//        indexLimitOutMax=item->index();
    }
}

void ParameterPin::GetInfos(MsgObject &msg)
{
    Pin::GetInfos(msg);

//    MsgObject l1(GetIndex());
    _MSGOBJ(l1,GetIndex());
    l1.prop[MsgObject::Id]=limitInMin->GetIndex();
    l1.prop[MsgObject::Add]=NodeType::cursor;
    l1.prop[MsgObject::Type]=ObjType::limitInMin;
    l1.prop[MsgObject::Value]=limitInMin->GetValue();
    msg.children << l1;

//    MsgObject l2(GetIndex());
    _MSGOBJ(l2,GetIndex());
    l2.prop[MsgObject::Id]=limitInMax->GetIndex();
    l2.prop[MsgObject::Add]=NodeType::cursor;
    l2.prop[MsgObject::Type]=ObjType::limitInMax;
    l2.prop[MsgObject::Value]=limitInMax->GetValue();
    msg.children << l2;

//    MsgObject l3(GetIndex());
    _MSGOBJ(l3,GetIndex());
    l3.prop[MsgObject::Id]=limitOutMin->GetIndex();
    l3.prop[MsgObject::Add]=NodeType::cursor;
    l3.prop[MsgObject::Type]=ObjType::limitOutMin;
    l3.prop[MsgObject::Value]=limitOutMin->GetValue();
    msg.children << l3;

//    MsgObject l4(GetIndex());
    _MSGOBJ(l4,GetIndex());
    l4.prop[MsgObject::Id]=limitOutMax->GetIndex();
    l4.prop[MsgObject::Add]=NodeType::cursor;
    l4.prop[MsgObject::Type]=ObjType::limitOutMax;
    l4.prop[MsgObject::Value]=limitOutMax->GetValue();
    msg.children << l4;
}

void ParameterPin::SetMsgEnabled(bool enab)
{
    MsgHandler::SetMsgEnabled(enab);
    limitInMin->SetMsgEnabled(enab);
    limitInMax->SetMsgEnabled(enab);
    limitOutMin->SetMsgEnabled(enab);
    limitOutMax->SetMsgEnabled(enab);
}

void ParameterPin::CursorValueChanged(float val)
{
    if(sender()==limitInMin) {
        if(limitInMax->GetValue()<=val) {
            limitInMax->SetValue(val+0.005);
        }
        limitInMin->SetValue(val);
    }

    if(sender()==limitInMax) {
        if(limitInMin->GetValue()>=val) {
            limitInMin->SetValue(val-0.005);
        }
        limitInMax->SetValue(val);
    }

    if(sender()==limitOutMin) {
        limitOutMin->SetValue(val);
    }
    if(sender()==limitOutMax) {
        limitOutMax->SetValue(val);
    }

    OnValueChanged(value);
    parent->OnParameterChanged(connectInfo,outValue);
    UpdateView();
    if(!dirty) {
        dirty=true;
        parent->OnProgramDirty();
    }
}
