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

#ifndef VST3PLUGIN_H
#define VST3PLUGIN_H

#ifdef VSTSDK

#include "object.h"

#ifdef _MSC_VER
#pragma warning( push, 1 )
#endif
//#include "public.sdk/samples/vst-hosting/audiohost/source/media/imediaserver.h"
//#include "public.sdk/samples/vst-hosting/audiohost/source/media/iparameterclient.h"
#include "public.sdk/source/vst/hosting/eventlist.h"
#include "public.sdk/source/vst/hosting/parameterchanges.h"
#include "public.sdk/source/vst/hosting/processdata.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "public.sdk/source/common/memorystream.h"
#include "public.sdk/source/vst/hosting/processdata.h"
#include "public.sdk/source/vst/hosting/parameterchanges.h"
#include "pluginterfaces/base/ipluginbase.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivsthostapplication.h"
#include "pluginterfaces/gui/iplugview.h"
#include "pluginterfaces/vst/ivstevents.h"
#include "pluginterfaces/vst/ivstcontextmenu.h"
#include "public.sdk/source/vst/hosting/plugprovider.h"
#include "public.sdk/source/vst/hosting/eventlist.h"
#ifdef _MSC_VER
#pragma warning( pop )
#endif

namespace View {
    class VstPluginWindow;
}

using namespace Steinberg;

namespace Connectables {

class Vst3Plugin : public Object,
        public Vst::IComponentHandler,
        public Vst::IComponentHandler2,
        public Vst::IComponentHandler3,
        public Vst::IContextMenuTarget

{
    Q_OBJECT
public:
    explicit Vst3Plugin(MainHost *host, int index, const ObjectInfo &info);
    ~Vst3Plugin();
    bool Open() override;
    bool Close() override;
    void Render() override;
    Pin* CreatePin(const ConnectionInfo &info) override;
    void SetSleep(bool sleeping) override;
    void SaveProgram() override;
    void LoadProgram(int prog) override;

    View::VstPluginWindow *editorWnd;

    tresult PLUGIN_API createInstance (TUID cid, TUID _iid, void** obj);

//    tresult PLUGIN_API notifyUnitSelection (UnitID unitId);
//    tresult PLUGIN_API notifyProgramListChange (Vst::ProgramListID listId, int32 programIndex);

    tresult PLUGIN_API queryInterface (const TUID iid, void** obj) override;
    uint32 PLUGIN_API addRef () override;
    uint32 PLUGIN_API release () override;
    tresult PLUGIN_API beginEdit (Vst::ParamID id) override;
    tresult PLUGIN_API performEdit (Vst::ParamID id, Vst::ParamValue valueNormalized) override;
    tresult PLUGIN_API endEdit (Vst::ParamID id) override;
    tresult PLUGIN_API restartComponent (int32 flags) override;
    tresult PLUGIN_API setDirty(TBool state) override;
    tresult PLUGIN_API requestOpenEditor (FIDString name = Vst::ViewType::kEditor) override;
    tresult PLUGIN_API startGroupEdit () override;
    tresult PLUGIN_API finishGroupEdit () override;

    Vst::IContextMenu* PLUGIN_API createContextMenu (IPlugView* plugView, const Vst::ParamID* paramID) override;
    tresult PLUGIN_API executeMenuItem (int32 tag) override;

    void MidiMsgFromInput(long msg) override;

    void fromJson(QJsonObject &json) override;
    void toJson(QJsonObject &json) const override;
    QDataStream & toStream (QDataStream &) const override;
    bool fromStream (QDataStream &) override;

    void SetContainerAttribs(const ObjectContainerAttribs &attr) override;
    void GetContainerAttribs(ObjectContainerAttribs &attr) override;
    void ReceiveMsg(const MsgObject &msg) override;



private:
    void Unload();
    bool CreateEditorWindow();
    void RemoveGui();
    bool initPlugin();
    bool initProcessor();
    bool initController();
    bool initAudioBuffers(Vst::BusDirection dir);
    void initProcessData();

    QLibrary *pluginLib;
    //IPluginFactory* factory;
//    Vst::IComponent* plugComponent;
    Vst::IComponent* component;
//    Vst::IEditController* editController;
    Vst::IEditController* editController;
//    Vst::IAudioProcessor* audioProcessor;

    Vst::IConnectionPoint* iConnectionPointComponent;
    Vst::IConnectionPoint* iConnectionPointController;
//    Vst::ParameterChanges vstParamChanges;
//    QMap<qint32,qint32>listParamQueue;
    bool hasEditor;
    IPlugView *pView;
    QMap<qint32,float>listParamChanged;

    QList<QVariant>listEditorVisible;
    QList<QVariant>listIsLearning;
    QList<QVariant>listBypass;
    QList<QVariant>listValues;
    QMutex paramLock;

//    char *savedChunk;
//    quint32 savedChunkSize;
    QByteArray savedState;
    bool bypass;

    qint32 progChangeParameter;
    qint32 bypassParameter;

    VST3::Hosting::Module::Ptr module {nullptr};
    IPtr<Vst::PlugProvider> plugProvider {nullptr};
    VST3::Hosting::ClassInfo vstinfo;

    Vst::HostProcessData processData;
	Vst::EventList inEvents;
    Vst::ParameterChanges inputParameterChanges;
    bool isProcessing;

signals:
    void WindowSizeChange(int newWidth, int newHeight);

public slots:
    void SetBufferSize(unsigned long size) override;
    void SetSampleRate(float rate=44100.0) override;
    void OnParameterChanged(ConnectionInfo pinInfo, float value) override;
    void EditorDestroyed();
    void OnEditorClosed();
    void OnShowEditor() override;
    void OnHideEditor() override;
    void UserRemovePin(const ConnectionInfo &info) override;
    void UserAddPin(const ConnectionInfo &info) override;

    friend class View::VstPluginWindow;
};
}

#endif
#endif // VST3PLUGIN_H
