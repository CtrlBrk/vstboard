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
#include "pluginterfaces/base/ustring.h"
#include "gui.h"
#include "mainwindowvst.h"
#include "projectfile/jsonwriter.h"
#include "projectfile/jsonreader.h"
#include "public.sdk/source/common/memorystream.h"

//-----------------------------------------------------------------------------
tresult PLUGIN_API VstBoardController::initialize (FUnknown* context)
{


//    mainWindow=0;
//    view=0;

    tresult result = EditController::initialize (context);
    if (result != kResultTrue)
        return result;

    QCoreApplication::setOrganizationName("CtrlBrk");
    QCoreApplication::setApplicationName("VstBoard");

    for(int32 i=0; i<NB_PARAM; i++) {
        parameters.addParameter( (char16*)QString("Param%1").arg(i).utf16(), 0, 127, 0, Vst::ParameterInfo::kCanAutomate, i);
    }
    parameters.addParameter (STR16 ("Program"), 0, 127, 0, Vst::ParameterInfo::kCanAutomate, paramProgChange);
    parameters.addParameter (STR16 ("Group"), 0, 127, 0, Vst::ParameterInfo::kCanAutomate, paramGroupChange);
    parameters.addParameter (STR16 ("Bypass"), 0, 1, 0, Vst::ParameterInfo::kCanAutomate | Vst::ParameterInfo::kIsBypass, paramByPass);

    return kResultTrue;
}

VstBoardController::~VstBoardController()
{
//    if(view)
//        delete view;
//    if(mainWindow)
//        delete mainWindow;


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
	MemoryStream* stateCtrl = static_cast<MemoryStream*>(state);
	QByteArray bArray(stateCtrl->getData(), stateCtrl->getSize());
	QBuffer buffer(&bArray);
	buffer.open(QIODevice::ReadOnly);

	QByteArray saveData = buffer.readAll();

	MainWindow *win = dynamic_cast<MainWindow*>(listGui.first());
	if (win) {
		//QJsonDocument loadDoc(QJsonDocument::fromBinaryData(qUncompress(saveData));
		QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
		QJsonObject json = loadDoc.object();

		if (json.contains("view")) {
			JsonReader::readProjectView(json["view"].toObject(), win, 0);
		}
	}
    return kResultOk;
}

tresult PLUGIN_API VstBoardController::getState (IBStream* state)
{
	QByteArray bArray;
	QBuffer buffer(&bArray);
	buffer.open(QIODevice::WriteOnly);

	MainWindow *win = dynamic_cast<MainWindow*>( listGui.first() );
	if (win) {
		QJsonObject jsonObj;
		jsonObj["proc"] = JsonWriter::writeProjectView(win, true, true);
		QJsonDocument saveDoc(jsonObj);
		buffer.write(saveDoc.toJson(QJsonDocument::Indented));
		state->write(bArray.data(), bArray.size());
	}

	return kResultOk;
}
