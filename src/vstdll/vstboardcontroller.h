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

#ifndef VSTBOARDCONTROLLER_H
#define VSTBOARDCONTROLLER_H

//#include "precomp.h"
#include "public.sdk/source/vst/vsteditcontroller.h"

class MainWindowVst;
namespace Steinberg {
    class Gui;
}

using namespace Steinberg;

//-----------------------------------------------------------------------------
class VstBoardController : public Vst::EditController
{
public:
    static FUnknown* createInstance (void*) { return (IEditController*)new VstBoardController (); }

    tresult PLUGIN_API initialize (FUnknown* context);
    IPlugView* PLUGIN_API createView (const char* name);
    void editorAttached (Vst::EditorView* editor);
    void editorRemoved (Vst::EditorView* editor);
    void editorDestroyed (Vst::EditorView* editor);
    tresult PLUGIN_API notify (Vst::IMessage* message);
private:
    QList<Gui*> listGui;
    MainWindowVst *mainWindow;
};


#endif // VSTBOARDCONTROLLER_H
