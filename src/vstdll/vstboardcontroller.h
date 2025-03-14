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

#ifndef VSTBOARDCONTROLLER_H
#define VSTBOARDCONTROLLER_H

//#include "precomp.h"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning (disable: 4200)
#endif
#include "public.sdk/source/vst/vsteditcontroller.h"
#include "public.sdk/source/vst/vstparameters.h"
#include "pluginterfaces/vst/ivstmidicontrollers.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

//#include "settings.h"

//class MainWindowVst;
namespace Steinberg {
    class Gui;
}

using namespace Steinberg;

//-----------------------------------------------------------------------------
class VstBoardController : public Vst::EditController, public Vst::IMidiMapping
{
public:
    static FUnknown* createInstance (void*) { return (IEditController*)new VstBoardController (); }
    ~VstBoardController();
    tresult PLUGIN_API initialize (FUnknown* context) SMTG_OVERRIDE;
    tresult PLUGIN_API setComponentState (IBStream* state) SMTG_OVERRIDE;
    IPlugView* PLUGIN_API createView (const char* name) SMTG_OVERRIDE;
    void editorAttached (Vst::EditorView* editor) SMTG_OVERRIDE;
    void editorRemoved (Vst::EditorView* editor) SMTG_OVERRIDE;
    void editorDestroyed (Vst::EditorView* editor) SMTG_OVERRIDE;
    tresult PLUGIN_API notify (Vst::IMessage* message) SMTG_OVERRIDE;
    tresult PLUGIN_API setState (IBStream* state) SMTG_OVERRIDE;
    tresult PLUGIN_API getState (IBStream* state) SMTG_OVERRIDE;
	tresult PLUGIN_API getMidiControllerAssignment(int32 busIndex, int16 channel, Vst::CtrlNumber midiControllerNumber, Vst::ParamID& tag /*out*/) SMTG_OVERRIDE;
	DELEGATE_REFCOUNT(EditController)
	tresult PLUGIN_API queryInterface(const char* iid, void** obj) SMTG_OVERRIDE;

protected:
	int32 midiCCParamID[Vst::kCountCtrlNumber];

private:
    QList<Gui*> listGui;
//    MainWindowVst *mainWindow;
//    Gui* view;
//    Settings *settings;
};


#endif // VSTBOARDCONTROLLER_H
