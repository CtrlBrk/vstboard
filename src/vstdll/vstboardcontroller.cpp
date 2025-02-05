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

#include "vstboardcontroller.h"
#include "ids.h"
// #include "pluginterfaces/base/ustring.h"
#include "gui.h"
// #include "mainwindowvst.h"
#include "public.sdk/source/common/memorystream.h"

using namespace Steinberg;

//-----------------------------------------------------------------------------
tresult PLUGIN_API VstBoardController::initialize (FUnknown* context)
{


//    mainWindow=0;
//    view=0;

    //already initialized
    if(parameters.getParameterCount()>0) {
        return kResultTrue;
    }

    tresult result = EditController::initialize (context);
    if (result != kResultTrue)
        return result;

    QCoreApplication::setOrganizationName("CtrlBrk");
    QCoreApplication::setApplicationName("VstBoard");

    int32 stepCount = 127;
    double defaultVal = 0;
    int32 flags = Vst::ParameterInfo::kCanAutomate;

    parameters.addParameter (STR16 ("Program"), nullptr, stepCount, defaultVal, flags, paramProgChange);
    parameters.addParameter (STR16 ("Group"), nullptr, stepCount, defaultVal, flags, paramGroupChange);

    for(int32 i=0; i<NB_PARAM; i++) {
        parameters.addParameter( (char16*)QString("Param%1").arg(i).utf16(), nullptr, stepCount, defaultVal, flags, i);
    }

    stepCount=1;
    defaultVal = 0;
    flags = Vst::ParameterInfo::kCanAutomate | Vst::ParameterInfo::kIsBypass;
    parameters.addParameter (STR16 ("Bypass"), nullptr, 1, defaultVal, flags, paramByPass);

    return kResultTrue;
}

VstBoardController::~VstBoardController()
{
//    if(view)
//        delete view;
//    if(mainWindow)
//        delete mainWindow;


}

tresult PLUGIN_API VstBoardController::setComponentState (IBStream* state)
{
    int size = 0;
    state->read(&size, sizeof(int));
    //don't know how to write directly to a bytearray
    char* buf = new char[size];
    state->read(buf, size);

    QByteArray bArray(QByteArray::fromRawData(buf, size));

    // QJsonDocument loadDoc(QJsonDocument::fromBinaryData(qUncompress(bArray)));
    QJsonDocument loadDoc(QJsonDocument::fromJson(bArray));
    QJsonObject json = loadDoc.object();
    if (json.contains("proc")) {
        json = json.value("proc").toObject();
        if (json.contains("programs")) {
            json = json.value("programs").toObject();
            bool bypass = static_cast<Qt::CheckState>(json["bypass"].toBool());
            setParamNormalized (paramByPass, bypass);
        }
    }

    return kResultOk;
}

IPlugView* PLUGIN_API VstBoardController::createView (const char* name)
{
    if (name && strcmp (name, "editor") == 0)
    {
        return new Gui(this);
    }
    return 0;
}

void VstBoardController::editorAttached (Vst::EditorView* editor)
{
    Gui* view = dynamic_cast<Gui*> (editor);
    if (view)
        listGui << view;
}

void VstBoardController::editorRemoved (Vst::EditorView* editor)
{
    Gui* view = dynamic_cast<Gui*> (editor);
    if (view)
        listGui.removeAll(view);
}

void VstBoardController::editorDestroyed (Vst::EditorView* editor)
{
    Gui* view = dynamic_cast<Gui*> (editor);
    if (view)
        listGui.removeAll(view);
}

tresult PLUGIN_API VstBoardController::notify (Vst::IMessage* message)
{
    if (!message)
        return kInvalidArgument;

    if (!strcmp (message->getMessageID (), "msg"))
    {
        const void* data;
        uint32 size;
        if (message->getAttributes ()->getBinary ("data", data, size) == kResultOk)
        {
//            if(!mainWindow)
//                return kResultOk;
            if(listGui.isEmpty())
                return kResultOk;

            QByteArray br((char*)data,size);
            QDataStream str(&br, QIODevice::ReadOnly);
            MsgObject msg;
            str >> msg;
            foreach(Gui *g, listGui) {
                g->ReceiveMsg( msg );
            }

//            mainWindow->ReceiveMsg( msg );
            return kResultOk;
        }
    }

    return EditController::notify(message);
}

tresult PLUGIN_API VstBoardController::setState (IBStream* state)
{
	int size = 0;
	state->read(&size, sizeof(int));
	//don't know how to write directly to a bytearray
	char* buf = new char[size];
	state->read(buf, size);
	
	QByteArray bArray(QByteArray::fromRawData(buf, size));

    // QJsonDocument loadDoc(QJsonDocument::fromBinaryData(qUncompress(bArray)));
    QJsonDocument loadDoc(QJsonDocument::fromJson(bArray));
	QJsonObject json = loadDoc.object();
	if (json.contains("ctrl")) {
		QJsonArray jLstCtrl = json["ctrl"].toArray();
		for (int i = 0; i < jLstCtrl.size(); ++i) {
			QJsonObject jCtrl = jLstCtrl[i].toObject();
			if (listGui.count() > i) {
				listGui[i]->fromJson(jCtrl);
			}
		}
	}
		
    return kResultOk;
}

tresult PLUGIN_API VstBoardController::getState (IBStream* state)
{
	QJsonObject jsonObj;
	QJsonArray jLst;
	for (int i = 0; i < listGui.count(); i++) {
		QJsonObject jObj;
		listGui[i]->toJson(jObj);
		jLst.append(jObj);
	}

	jsonObj["ctrl"] = jLst;
	QJsonDocument saveDoc(jsonObj);
    // QByteArray bArray = qCompress(saveDoc.toBinaryData());
    QByteArray bArray = saveDoc.toJson(QJsonDocument::Indented);
	int size = bArray.size();
	state->write(&size, sizeof(int));
	state->write(bArray.data(), bArray.size());

	return kResultOk;
}

tresult PLUGIN_API VstBoardController::getMidiControllerAssignment(int32 busIndex, int16 /*channel*/, Vst::CtrlNumber midiControllerNumber, Vst::ParamID& tag /*out*/)
{
	if (busIndex == 0 && midiControllerNumber < Vst::kCountCtrlNumber && midiCCParamID[midiControllerNumber] != -1)
	{
		tag = midiCCParamID[midiControllerNumber];
		return kResultTrue;
	}
	return kResultFalse;
}

tresult PLUGIN_API VstBoardController::queryInterface(const char* iid, void** obj)
{
	QUERY_INTERFACE(iid, obj, IMidiMapping::iid, IMidiMapping)
	return EditController::queryInterface(iid, obj);
}
