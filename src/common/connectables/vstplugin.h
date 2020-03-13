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
        ~VstPlugin();
        bool Open();
        bool Close();
        void Render();
        VstIntPtr OnMasterCallback(long opcode, long index, long value, void *ptr, float opt, long currentReturnCode);
        void SetSleep(bool sleeping);
        void MidiMsgFromInput(long msg);
        QString GetParameterName(ConnectionInfo pinInfo);
        inline AEffect* GetPlugin() {return pEffect;}

        View::VstPluginWindow *editorWnd;

        static QMap<AEffect*,VstPlugin*>mapPlugins;
        static VstPlugin *pluginLoading;

        void CreateEditorWindow();

//        static View::VstShellSelect *shellSelectView;

        void SetContainerAttribs(const ObjectContainerAttribs &attr);
        void GetContainerAttribs(ObjectContainerAttribs &attr);
        Pin* CreatePin(const ConnectionInfo &info);

        bool DropFile(const QString &filename);

        QDataStream & toStream (QDataStream &) const;
        bool fromStream (QDataStream &);

        void AddPluginToDatabase();
        void ReceiveMsg(const MsgObject &msg);
        void GetInfos(MsgObject &msg);

    protected:
        void SetId(int id) {objInfo.id = id;}
        bool initPlugin();
        void processEvents(VstEvents* events);
        void onVstProgramChanged();
        bool FilenameFromDatabase(VstInt32 id, QString &filename);
        VstInt32 IdFromFxb(const QString &fxbFile);

        QString bankToLoad;
        QString currentBankFile;
        float sampleRate;
        unsigned long bufferSize;
        VstEvents *listEvnts;

        /// list of values used by the editor pin (0 and 1)
        QList<QVariant>listEditorVisible;

        /// list of values used by the learn pin (off, learn, unlearn)
        QList<QVariant>listIsLearning;

        QMutex midiEventsMutex;
        QList<VstMidiEvent*>listVstMidiEvents;
        QList<QVariant>listValues;
        QList<QVariant>listBypass;

        char *savedChunk;
        quint32 savedChunkSize;

        bool bypass;

    signals:
        void WindowSizeChange(int newWidth, int newHeight);

    public slots:
        void SetBufferSize(unsigned long size);
        void SetSampleRate(float rate=44100.0);
        void RaiseEditor();
        void EditorDestroyed();
        void EditIdle();
        void OnParameterChanged(ConnectionInfo pinInfo, float value);
        void OnShowEditor();
        void OnHideEditor();
        void OnEditorClosed();
        void LoadBank();
        bool LoadBank(const QString &filename);
        void SaveBank(const QString &filename);
        bool LoadProgramFile(const QString &filename);
        void SaveProgramFile(const QString &filename);
//        void TakeScreenshot();
        void UserRemovePin(const ConnectionInfo &info);
        void UserAddPin(const ConnectionInfo &info);

        friend class View::VstPluginWindow;
//        friend class View::VstShellSelect;
    };

}

#endif
#endif // VSTPLUGIN_H
