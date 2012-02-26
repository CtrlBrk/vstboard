/**************************************************************************
#    Copyright 2010-2011 Raphaël François
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

#include "objectview.h"
#include "pinview.h"
#include "../globals.h"
#include "commands/comremoveobject.h"
#include "minmaxpinview.h"

using namespace View;

/*!
  \class View::ObjectView
  \brief base class for the objects views
  */

/*!
  \param config pointer to the config object
  \param model pointer to the model
  \param parent the parent graphic item
  \param wFlags window flags
  \todo the model parameter can be removed
  */
ObjectView::ObjectView(ViewConfig *config, MsgController *msgCtrl, int objId, QGraphicsItem * parent) :
    QGraphicsWidget(parent),
    MsgHandler(msgCtrl,objId),
    listAudioIn(0),
    listAudioOut(0),
    listMidiIn(0),
    listMidiOut(0),
    listParametersIn(0),
    listParametersOut(0),
    listBridge(0),
    titleText(0),
    border(0),
    selectBorder(0),
    errorMessage(0),
    layout(0),
    actRemove(0),
    actRemoveBridge(0),
    actShowEditor(0),
    actLearnSwitch(0),
    actToggleBypass(0),
    shrinkAsked(false),
    highlighted(false),
    config(config),
    editorPin(0),
    learnPin(0),
    bypassPin(0),
    editorAutoOpened(false)
{
    setObjectName("objView");

    setAutoFillBackground(true);

    QPalette pal(palette());
    pal.setColor(QPalette::Window, config->GetColor(ColorGroups::Object,Colors::Background) );
    pal.setColor(QPalette::Text, config->GetColor(ColorGroups::Object,Colors::Text) );
    setPalette( pal );

    connect( config, SIGNAL(ColorChanged(ColorGroups::Enum,Colors::Enum,QColor)),
            this, SLOT(UpdateColor(ColorGroups::Enum,Colors::Enum,QColor)) );
    connect( config->keyBinding, SIGNAL(BindingChanged()),
            this, SLOT(UpdateKeyBinding()));
}

/*!
  setActive to false before destruction to move the focus to the next item
  */
ObjectView::~ObjectView()
{
    setActive(false);
}

void ObjectView::UpdateColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color)
{
    if(groupId!=ColorGroups::Object)
        return;

    switch(colorId) {
        case Colors::Background : {
            QPalette pal(palette());
            pal.setColor(QPalette::Window,color);
            setPalette( pal );
            break;
        }
        case Colors::Text : {
            QPalette pal(palette());
            pal.setColor(QPalette::Text,color);
            setPalette( pal );
            if(titleText)
                titleText->setBrush(color);
            break;
        }

        default:
            break;
    }
}

/*!
  Reimplements QGraphicsWidget::contextMenuEvent \n
  create a menu with all the actions
  */
void ObjectView::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if(actions().size()==0)
        return;

    QMenu menu;
    menu.exec(actions(),event->screenPos(),actions().at(0),event->widget());
}

void ObjectView::Init(const MsgObject &msg)
{
    if(msg.prop.contains(MsgObject::Type) && msg.prop[MsgObject::Type].toInt() != NodeType::bridge) {

        actRemoveBridge = new QAction(QIcon(":/img16x16/delete.png"),tr("Remove"),this);
        actRemoveBridge->setShortcutContext(Qt::WidgetShortcut);
        connect(actRemoveBridge,SIGNAL(triggered()),
                this,SLOT(RemoveWithBridge()));
        addAction(actRemoveBridge);

        actRemove = new QAction(QIcon(":/img16x16/delete.png"),tr("Remove with cables"),this);
        actRemove->setShortcutContext(Qt::WidgetShortcut);
        connect(actRemove,SIGNAL(triggered()),
                this,SLOT(close()));
        addAction(actRemove);

        actShowEditor = new QAction(tr("Show Editor"),this);
        actShowEditor->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        actShowEditor->setEnabled(false);
        actShowEditor->setCheckable(true);
        connect(actShowEditor,SIGNAL(toggled(bool)),
                this,SLOT(SwitchEditor(bool)));
        addAction(actShowEditor);

        actLearnSwitch = new QAction(tr("Learn Mode"),this);
        actLearnSwitch->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        actLearnSwitch->setEnabled(false);
        actLearnSwitch->setCheckable(true);
        connect(actLearnSwitch,SIGNAL(toggled(bool)),
                this,SLOT(SwitchLearnMode(bool)));
        addAction(actLearnSwitch);

        actToggleBypass = new QAction("Bypass",this);
        actToggleBypass->setShortcutContext(Qt::WidgetShortcut);
        actToggleBypass->setEnabled(false);
        actToggleBypass->setCheckable(true);
        connect(actToggleBypass,SIGNAL(toggled(bool)),
                this,SLOT(ToggleBypass(bool)));
        addAction(actToggleBypass);
    }

    if(msg.prop[MsgObject::Type].toInt() == ObjType::dummy) {
        SetErrorMessage("object not loaded");
    }

    UpdateKeyBinding();
    ReceiveMsg(msg);
}

void ObjectView::ReceiveMsg(const MsgObject &msg)
{
    if(msg.prop.contains(MsgObject::Add)) {
        switch(msg.prop[MsgObject::Add].toInt()) {
        case NodeType::listPin :
            AddListPins(msg);
            return;
        }
    }

    if(msg.prop.contains(MsgObject::Message)) {
        SetErrorMessage(msg.prop[MsgObject::Message].toString());
    }

    if(msg.prop.contains(MsgObject::Name)) {
#ifndef QT_NO_DEBUG
    UpdateTitle( QString("%1 (%2)").arg(msg.prop[MsgObject::Name].toString()).arg(GetIndex()) );
#else
    UpdateTitle( msg.prop[MsgObject::Name].toString() );
#endif
    }

    if(msg.prop.contains(MsgObject::State)) {
        ObjectContainerAttribs attr = msg.prop[MsgObject::State].value<ObjectContainerAttribs>();
        setPos(attr.position);
    }
}

void ObjectView::UpdateKeyBinding()
{
    if(actRemoveBridge) actRemoveBridge->setShortcut( config->keyBinding->GetMainShortcut(KeyBind::deleteObject) );
    if(actRemove) actRemove->setShortcut( config->keyBinding->GetMainShortcut(KeyBind::deleteObjectWithCables) );
    if(actShowEditor) actShowEditor->setShortcut( config->keyBinding->GetMainShortcut(KeyBind::toggleEditor) );
    if(actLearnSwitch) actLearnSwitch->setShortcut( config->keyBinding->GetMainShortcut(KeyBind::toggleLearnMode) );
    if(actToggleBypass) actToggleBypass->setShortcut( config->keyBinding->GetMainShortcut(KeyBind::toggleBypass) );
}

void ObjectView::UpdateTitle(const QString &name)
{
    if(titleText) {
        QString newTitle=name;
        titleText->setText(newTitle);
    }
}


/*!
  Set the error message, add the error icon
  \param msg the error string
  */
void ObjectView::SetErrorMessage(const QString & msg)
{
    if(msg.isEmpty()) {
        if(errorMessage) {
            delete errorMessage;
            errorMessage=0;
            setToolTip("");
        }
    } else {
        if(!errorMessage) {
            QPixmap pix(":/img32x32/agt_action_fail.png");
            errorMessage = new QGraphicsPixmapItem(pix,this,scene());
//            QPointF pt = geometry().center();
//            pt.rx() -= pix.width()/2;
//            pt.ry() -= pix.height()/2;
//            errorMessage->setPos(pt);
            errorMessage->setPos(0,0);
        }
        setToolTip(msg);
    }
}

/*!
  Reimplements QGraphicsWidget::closeEvent \n
  setActive to false, tell the objectFactory to park this object
  */
void ObjectView::closeEvent ( QCloseEvent * event )
{
    setActive(false);

    MsgObject msg(GetIndex());
    msg.prop[MsgObject::Remove]=RemoveType::RemoveWithCables;
    msgCtrl->SendMsg(msg);
    event->ignore();
}

void ObjectView::RemoveWithBridge()
{
    setActive(false);

    MsgObject msg(GetIndex());
    msg.prop[MsgObject::Remove]=RemoveType::BridgeCables;
    msgCtrl->SendMsg(msg);
}

/*!
  Reimplements QGraphicsItem::focusInEvent
  draw a border around focused object
  */
void ObjectView::focusInEvent ( QFocusEvent * event )
{
    if(event->reason()==Qt::MouseFocusReason &&
            config->AutoOpenGui &&
            editorPin &&
            editorPin->GetValue() < .5f) {
        editorAutoOpened=true;
        editorPin->ValueChanged(1.0f);
    }

    if(selectBorder)
        delete selectBorder;
    selectBorder=new QGraphicsRectItem( -2,-2, size().width()+4, size().height()+4 , this );
    QGraphicsWidget::focusInEvent(event);
}

/*!
  Reimplements QGraphicsItem::focusOutEvent
  remove border
  */
void ObjectView::focusOutEvent ( QFocusEvent * event )
{
    if(event->reason()==Qt::MouseFocusReason &&
            config->AutoOpenGui &&
            editorAutoOpened) {
        editorAutoOpened=false;
        editorPin->ValueChanged(0.0f);
    }

    if(selectBorder) {
        delete selectBorder;
        selectBorder =0;
    }
    QGraphicsWidget::focusOutEvent(event);
}

/*!
  Reimplements QGraphicsWidget::resizeEvent \n
  resize the selected object borders on resize
  */
void ObjectView::resizeEvent ( QGraphicsSceneResizeEvent * event )
{
    if(selectBorder)
        selectBorder->setRect( -2, -2, event->newSize().width()+4, event->newSize().height()+4);
}

/*!
  Shrink the object to its minimal size after a 100ms delay
  */
void ObjectView::Shrink()
{
    if(shrinkAsked)
        return;

    shrinkAsked=true;
    QTimer::singleShot(100, this, SLOT(ShrinkNow()));
}

/*!
  Shrink the object to its minimal size
  */
void ObjectView::ShrinkNow()
{
    shrinkAsked=false;
    resize(0,0);
}

void ObjectView::SetEditorPin(MinMaxPinView *pin, float value)
{
//    if(editorPin==pin)
//        return;
    editorPin = pin;
    actShowEditor->setEnabled(editorPin);
    actShowEditor->setChecked(value>.5f);
}

void ObjectView::SetLearnPin(MinMaxPinView *pin, float value)
{
    learnPin = pin;
    actLearnSwitch->setEnabled(learnPin);
    actLearnSwitch->setChecked(value>.33f);
}

void ObjectView::SetBypassPin(MinMaxPinView *pin, float value)
{
    bypassPin = pin;
    actToggleBypass->setEnabled(bypassPin);
    actToggleBypass->setChecked(value>.33f);
}

void ObjectView::SwitchLearnMode(bool on)
{
    if(!learnPin)
        return;

    if(learnPin->GetValue() > .33f && !on)
        learnPin->ValueChanged(.0f);
    if(learnPin->GetValue() < .33f && on)
        learnPin->ValueChanged(.34f);
}

void ObjectView::SwitchEditor(bool show)
{
    if(!editorPin)
        return;

    if(editorAutoOpened)
        editorAutoOpened=false;

    if(editorPin->GetValue() > .5f && !show)
        editorPin->ValueChanged(.0f);
    if(editorPin->GetValue() < .5f && show)
        editorPin->ValueChanged(1.0f);
}

void ObjectView::ToggleBypass(bool b)
{
    if(!bypassPin)
        return;

    if(bypassPin->GetValue() > .33f && !b)
        bypassPin->ValueChanged(.0f);
    if(bypassPin->GetValue() < .33f && b)
        bypassPin->ValueChanged(0.34f);
}

void ObjectView::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    ToggleEditor();
}

void ObjectView::ToggleEditor()
{
    if(!actShowEditor || !actShowEditor->isEnabled())
        return;

    actShowEditor->toggle();
}

void ObjectView::AddListPins(const MsgObject &msg)
{
    ListPinsView *v=0;

    switch(msg.prop[MsgObject::Type].toInt()) {
        case ObjType::listAudioIn :
            v = listAudioIn;
            break;
        case ObjType::listAudioOut :
            v = listAudioOut;
            break;
        case ObjType::listMidiIn :
            v = listMidiIn;
            break;
        case ObjType::listMidiOut :
            v = listMidiOut;
            break;
        case ObjType::listParamIn :
            v = listParametersIn;
            break;
        case ObjType::listParamOut :
            v = listParametersOut;
            break;
        case ObjType::listBridgeIn :
        case ObjType::listBridgeOut :
            v = listBridge;
            break;
        default:
            LOG("unknown pin type");
            return;
    }

    if(!v) {
        //debug("SceneView::rowsInserted listpin not found")
        return;
    }
    v->SetIndex(msg.prop[MsgObject::Id].toInt());
}
