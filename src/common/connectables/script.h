#ifndef SCRIPT_H
#define SCRIPT_H

#ifdef SCRIPTENGINE

#include "object.h"
#include "views/scripteditor.h"

namespace Connectables {

    class Script : public Object
    {
    Q_OBJECT

    public:
        Script(MainHost *host, int index, const ObjectInfo &info);
        virtual ~Script();
        bool Open();
        bool Close();
        void Render();
        Pin* CreatePin(const ConnectionInfo &info);

    protected:
        /// list of values used by the editor pin (0 and 1)
        QList<QVariant>listEditorVisible;

        QString objScriptName;
        QString scriptText;
        QString comiledScript;
        View::ScriptEditor *editorWnd;
        QMutex mutexScript;

        QScriptValue scriptThisObj;
        QScriptValue objScript;
        QScriptValue openScript;
        QScriptValue renderScript;

    signals :
        void _dspMsg(const QString &title, const QString &str);

    public slots:
        void SaveProgram();
        void LoadProgram(int prog);
        void ReplaceScript(const QString &str);
        void alert(const QString &str);
        void OnShowEditor();
        void OnHideEditor();
        void OnEditorClosed();
        void EditorDestroyed();

    private slots:
        void DspMsg(const QString &title, const QString &str);
    };
}

#endif

#endif // SCRIPT_H
