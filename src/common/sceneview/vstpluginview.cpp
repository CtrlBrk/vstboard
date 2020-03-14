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

#include "vstpluginview.h"

using namespace View;

VstPluginView::VstPluginView(ViewConfig *config,MsgController *msgCtrl,int objId, MainContainerView *parent) :
    ConnectableObjectView(config,msgCtrl,objId,parent),
    actSaveBank(0),
    actSaveBankAs(0),
    actSaveProgram(0),
    actSaveProgramAs(0)
{
    setObjectName("vstPluginView");

    setAcceptDrops(true);

    QPalette pal(palette());
    pal.setColor(QPalette::Window, config->GetColor(ColorGroups::VstPlugin,Colors::Background) );
    setPalette( pal );
}

void VstPluginView::UpdateColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color)
{
    if(colorId==Colors::Background)  {
        if(groupId==ColorGroups::VstPlugin) {
            if(!highlighted) {
                QPalette pal(palette());
                pal.setColor(QPalette::Window,color);
                setPalette( pal );
            }
        }
        return;
    }

    if(groupId==ColorGroups::Object && colorId==Colors::HighlightBackground)  {
        if(highlighted) {
            QPalette pal(palette());
            pal.setColor(QPalette::Window,color);
            setPalette( pal );
        }
    }
    ConnectableObjectView::UpdateColor(groupId,colorId,color);
}

void VstPluginView::Init(const MsgObject &msg)
{
    actSaveBank = new QAction(QIcon(":/img16x16/filesave.png"),tr("Save Bank"),this);
    actSaveBank->setShortcutContext(Qt::WidgetShortcut);
    connect(actSaveBank,SIGNAL(triggered()),
            this,SLOT(SaveBank()));
    addAction(actSaveBank);

    actSaveBankAs = new QAction(QIcon(":/img16x16/filesaveas.png"),tr("Save Bank As..."),this);
    actSaveBankAs->setShortcutContext(Qt::WidgetShortcut);
    connect(actSaveBankAs,SIGNAL(triggered()),
            this,SLOT(SaveBankAs()));
    actSaveBankAs->setEnabled(false);
    addAction(actSaveBankAs);

    actSaveProgram = new QAction(QIcon(":/img16x16/filesave.png"),tr("Save Program"),this);
    actSaveProgram->setShortcutContext(Qt::WidgetShortcut);
    connect(actSaveProgram,SIGNAL(triggered()),
            this,SLOT(SaveProgram()));
    addAction(actSaveProgram);

    actSaveProgramAs = new QAction(QIcon(":/img16x16/filesaveas.png"),tr("Save Program As..."),this);
    actSaveProgramAs->setShortcutContext(Qt::WidgetShortcut);
    connect(actSaveProgramAs,SIGNAL(triggered()),
            this,SLOT(SaveProgramAs()));
    actSaveProgramAs->setEnabled(false);
    addAction(actSaveProgramAs);

    ObjectView::Init(msg);
}

void VstPluginView::UpdateKeyBinding()
{
    ObjectView::UpdateKeyBinding();
    if(actSaveBank) actSaveBank->setShortcut( config->keyBinding->GetMainShortcut(KeyBind::saveBank) );
    if(actSaveBankAs) actSaveBankAs->setShortcut( config->keyBinding->GetMainShortcut(KeyBind::saveBankAs) );
    if(actSaveProgram) actSaveProgram->setShortcut( config->keyBinding->GetMainShortcut(KeyBind::saveProgram) );
    if(actSaveProgramAs) actSaveProgramAs->setShortcut( config->keyBinding->GetMainShortcut(KeyBind::saveProgramAs) );
}

void VstPluginView::ReceiveMsg(const MsgObject &msg)
{
    if(msg.prop.contains(MsgObject::Load)) {
        QString name = msg.prop[MsgObject::Load].toString();
        if(name.endsWith("fxp",Qt::CaseInsensitive)) {
            currentProgFile = name;
            actSaveProgramAs->setEnabled(true);
            actSaveBankAs->setEnabled(false);
        } else if(name.endsWith("fxb",Qt::CaseInsensitive)) {
            currentBankFile = name;
            actSaveProgramAs->setEnabled(false);
            actSaveBankAs->setEnabled(true);
        }
    }
    ConnectableObjectView::ReceiveMsg(msg);
}

void VstPluginView::SaveBankAs()
{
    QString lastDir = config->settings->GetSetting("lastBankPath").toString();
    QString filename = QFileDialog::getSaveFileName(0, tr("Save Bank"), lastDir, tr("Bank File (*.%1)").arg(VST_BANK_FILE_EXTENSION) );
    if(filename.isEmpty())
        return;

    if( !filename.endsWith( QString(".")+VST_BANK_FILE_EXTENSION, Qt::CaseInsensitive ) ) {
        filename.append( QString(".")+VST_BANK_FILE_EXTENSION );
    }

    config->settings->SetSetting("lastBankPath",QFileInfo(filename).absolutePath());

    MsgObject msg(GetIndex());
    msg.prop[MsgObject::Save] =  filename;
    msgCtrl->SendMsg(msg);
}

void VstPluginView::SaveBank()
{
    if(actSaveBankAs->isEnabled()) {
        MsgObject msg(GetIndex());
        msg.prop[MsgObject::Save] =  currentBankFile;
        msgCtrl->SendMsg(msg);
    } else {
        SaveBankAs();
    }
}

void VstPluginView::SaveProgramAs()
{
    QString lastDir = config->settings->GetSetting("lastBankPath").toString();
    QString filename = QFileDialog::getSaveFileName(0, tr("Save Program"), lastDir, tr("Program File (*.%1)").arg(VST_PROGRAM_FILE_EXTENSION) );
    if(filename.isEmpty())
        return;

    if( !filename.endsWith(QString(".")+VST_PROGRAM_FILE_EXTENSION, Qt::CaseInsensitive) ) {
        filename.append(QString(".")+VST_PROGRAM_FILE_EXTENSION);
    }

    config->settings->SetSetting("lastBankPath",QFileInfo(filename).absolutePath());

    MsgObject msg(GetIndex());
    msg.prop[MsgObject::Save] =  filename;
    msgCtrl->SendMsg(msg);
}

void VstPluginView::SaveProgram()
{
    if(actSaveProgramAs->isEnabled()) {
        MsgObject msg(GetIndex());
        msg.prop[MsgObject::Save] =  currentProgFile;
        msgCtrl->SendMsg(msg);
    } else {
        SaveProgramAs();
    }
}

void VstPluginView::dragEnterEvent( QGraphicsSceneDragDropEvent *event)
{
    //accept fxp files and replacement plugin
    if (event->mimeData()->hasUrls()) {
        QString fName;
        QFileInfo info;

        QStringList acceptedFiles;
        acceptedFiles << "fxb" << "fxp" ;

        foreach(QUrl url,event->mimeData()->urls()) {
            fName = url.toLocalFile();
            info.setFile( fName );
            if ( info.isFile() && info.isReadable() ) {
                if( acceptedFiles.contains( info.suffix(), Qt::CaseInsensitive) ) {
                    event->setDropAction(Qt::TargetMoveAction);
                    event->accept();
                    HighlightStart();
                    return;
                }
            }
        }
    }
    event->ignore();
}

void VstPluginView::dragLeaveEvent( QGraphicsSceneDragDropEvent *event)
{
    HighlightStop();
}

void VstPluginView::dropEvent( QGraphicsSceneDragDropEvent *event)
{
    HighlightStop();
    QGraphicsWidget::dropEvent(event);
//    event->setAccepted(model->dropMimeData(event->mimeData(), event->proposedAction(), 0, 0, objIndex));
}

void VstPluginView::HighlightStart()
{
    QPalette pal(palette());
    pal.setColor(QPalette::Window, config->GetColor(ColorGroups::Object,Colors::HighlightBackground) );
    setPalette( pal );
}

void VstPluginView::HighlightStop()
{
    QPalette pal(palette());
    pal.setColor(QPalette::Window, config->GetColor(ColorGroups::VstPlugin,Colors::Background) );
    setPalette( pal );
}
