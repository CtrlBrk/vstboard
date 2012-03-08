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

#include "scripteditor.h"
#include "ui_scripteditor.h"
#include "connectables/script.h"

using namespace View;

ScriptEditor::ScriptEditor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScriptEditor),
    object(0)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::Tool);
    setWindowTitle(tr("Script editor"));
}

ScriptEditor::~ScriptEditor()
{
    delete ui;
}

void ScriptEditor::SetObject(Connectables::Script *script)
{
    object=script;
}

void ScriptEditor::closeEvent( QCloseEvent * event )
{
    if(!object) {
        event->accept();
        return;
    }

    hide();
    emit Hide();
    event->ignore();
}

void ScriptEditor::SetScript(const QString &script)
{
    ui->plainTextEdit->setPlainText( script );
}

void ScriptEditor::on_btExecute_clicked()
{
    emit Execute( ui->plainTextEdit->toPlainText() );
}
