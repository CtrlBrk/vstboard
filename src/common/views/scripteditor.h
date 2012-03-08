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

#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

//#include "precomp.h"

namespace Ui {
    class ScriptEditor;
}
namespace Connectables {
    class Script;
}
namespace View {

    class ScriptEditor : public QWidget
    {
        Q_OBJECT

    public:
        explicit ScriptEditor(QWidget *parent = 0);
        ~ScriptEditor();
        void SetScript(const QString &script);
        void SetObject(Connectables::Script *script);

    protected:
        void closeEvent ( QCloseEvent * event );

    signals:
        void Execute(const QString &str);
        void Hide();

    private slots:
        void on_btExecute_clicked();

    private:
        Ui::ScriptEditor *ui;
        Connectables::Script *object;
    };
}
#endif // SCRIPTEDITOR_H
