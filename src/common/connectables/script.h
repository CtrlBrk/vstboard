/**************************************************************************
#    Copyright 2010-2012 Raphaël François
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
