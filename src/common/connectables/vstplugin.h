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

#ifndef VSTPLUGIN_H
#define VSTPLUGIN_H

#ifdef VSTSDK

#include "object.h"
#include "../vst/ceffect.h"
//#include "../views/vstshellselect.h"

namespace View {
    class VstPluginWindow;
}


namespace Connectables {

    class VstPlugin : public Object , public vst::CEffect
    {
    Q_OBJECT

    public:
        VstPlugin(MainHost *myHost,int index, const ObjectInfo & info);
        virtual ~VstPlugin();
        bool Open() override;
        bool Close();
        void Render() override;
        VstIntPtr OnMasterCallback(long opcode, long index, long value, void *ptr, float opt, long currentReturnCode) override;
        void SetSleep(bool sleeping) override;
        void MidiMsgFromInput(long msg) override;
        QString GetParameterName(ConnectionInfo pinInfo) override;
        inline AEffect* GetPlugin() {return pEffect;}

        View::VstPluginWindow *editorWnd;

        static QMap<AEffect*,VstPlugin*>mapPlugins;
        static VstPlugin *pluginLoading;

        virtual void CreateEditorWindow();

//        static View::VstShellSelect *shellSelectView;

        void SetContainerAttribs(const ObjectContainerAttribs &attr) override;
        void GetContainerAttribs(ObjectContainerAttribs &attr) override;
        Pin* CreatePin(const ConnectionInfo &info) override;

        bool DropFile(const QString &filename);

        void fromJson(QJsonObject &json) override;
        void toJson(QJsonObject &json)  override;
        QDataStream & toStream (QDataStream &) override;
        bool fromStream (QDataStream &) override;

        void AddPluginToDatabase();
        void ReceiveMsg(const MsgObject &msg) override;
        void GetInfos(MsgObject &msg) override;

    protected:
        virtual void ProcessMidi();

        void SetId(int id) {objInfo.id = id;}
        virtual bool initPlugin();
        void processEvents(VstEvents* events);
        void onVstProgramChanged();
        bool FilenameFromDatabase(VstInt32 id, QString &filename);
        VstInt32 IdFromFxb(const QString &fxbFile);

        QString bankToLoad;
        QString currentBankFile;
        float sampleRate;
        qint32 bufferSize;

        /// list of values used by the learn pin (off, learn, unlearn)
        QList<QVariant>listIsLearning;

        QMutex midiEventsMutex;
        QList<VstMidiEvent*>listVstMidiEvents;
        QList<QVariant>listValues;
        QList<QVariant>listBypass;

        char *savedChunk;
        quint32 savedChunkSize;

        bool bypass;
        bool hasEditor;

    signals:
        void WindowSizeChange(int newWidth, int newHeight);

    public slots:
        void SetBufferSize(qint32 size) override;
        void SetSampleRate(float rate=44100.0) override;
        void RaiseEditor();
        void EditorDestroyed();
        void EditIdle();
        void OnParameterChanged(ConnectionInfo pinInfo, float value) override;
        void OnShowEditor() override;
        void OnHideEditor() override;
        void OnEditorClosed();
        void _LoadBank();
        bool _LoadBank(const QString &filename);
        void _SaveBank(const QString &filename);
        bool LoadProgramFile(const QString &filename);
        void SaveProgramFile(const QString &filename);
//        void TakeScreenshot();
        void UserRemovePin(const ConnectionInfo &info) override;
        void UserAddPin(const ConnectionInfo &info) override;

        friend class View::VstPluginWindow;
//        friend class View::VstShellSelect;
    };

}

#endif
#endif // VSTPLUGIN_H
