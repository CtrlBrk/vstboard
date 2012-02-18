#ifndef VST3PLUGIN_H
#define VST3PLUGIN_H

#ifdef VSTSDK

#include "object.h"
#include "public.sdk/source/common/memorystream.h"
#include "public.sdk/source/vst/hosting/processdata.h"
#include "public.sdk/source/vst/hosting/parameterchanges.h"
#include "pluginterfaces/base/ipluginbase.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/gui/iplugview.h"

namespace View {
    class VstPluginWindow;
}

using namespace Steinberg;

namespace Connectables {

class Vst3Plugin : public Object, public Vst::IComponentHandler
{
    Q_OBJECT
public:
    explicit Vst3Plugin(MainHost *host, int index, const ObjectInfo &info);
    ~Vst3Plugin();
    bool Open();
    bool Close();
    void Render();
    Pin* CreatePin(const ConnectionInfo &info);
    void SetSleep(bool sleeping);
    View::VstPluginWindow *editorWnd;

    tresult PLUGIN_API queryInterface (const TUID iid, void** obj);
    uint32 PLUGIN_API addRef ();
    uint32 PLUGIN_API release ();
    tresult PLUGIN_API beginEdit (Vst::ParamID id);
    tresult PLUGIN_API performEdit (Vst::ParamID id, Vst::ParamValue valueNormalized);
    tresult PLUGIN_API endEdit (Vst::ParamID id);
    tresult PLUGIN_API restartComponent (int32 flags);
//    tresult PLUGIN_API setDirty (TBool state);
//    tresult PLUGIN_API requestOpenEditor (FIDString name=Vst::ViewType::kEditor);
//    tresult PLUGIN_API startGroupEdit ();
//    tresult PLUGIN_API finishGroupEdit ();

    QDataStream & toStream (QDataStream &) const;
    bool fromStream (QDataStream &);

private:
    void Unload();
    void CreateEditorWindow();
    void RemoveGui();

    QLibrary *pluginLib;
    IPluginFactory* factory;
    Vst::IComponent* processorComponent;
    Vst::IEditController* editController;
    Vst::IAudioProcessor* audioEffect;
    Vst::HostProcessData processData;
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

signals:
    void WindowSizeChange(int newWidth, int newHeight);

public slots:
    void OnParameterChanged(ConnectionInfo pinInfo, float value);
    void EditorDestroyed();
    void OnEditorClosed();
    void OnShowEditor();
    void OnHideEditor();
    void UserRemovePin(const ConnectionInfo &info);
    void UserAddPin(const ConnectionInfo &info);
};
}

#endif
#endif // VST3PLUGIN_H
