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
#include "connectablepinview.h"

using namespace View;

/*!
  \class View::ObjectView
  \brief base class for the objects views
  */

/*!
  \param myHost pointer to the MainHost
  \param model pointer to the model
  \param parent the parent graphic item
  \param wFlags window flags
  \todo the model parameter can be removed
  */
ObjectView::ObjectView(const MetaInfo &info, QGraphicsItem * parent ) :
    QGraphicsWidget(parent),
    MetaInfo(info),
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
    shrinkAsked(false),
    highlighted(false),
    config(0),
    editorPin(0),
    learnPin(0)
{
    setObjectName("objView");

    setAutoFillBackground(true);

    if(Meta(MetaInfos::ObjType).toInt() == ObjTypes::Dummy) {
        SetErrorMessage("object not loaded");
    }

    if(MetaInfo::Type() != MetaTypes::bridge) {
        actRemoveBridge = new QAction(QIcon(":/img16x16/delete.png"),tr("Remove"),this);
        actRemoveBridge->setShortcut( Qt::Key_Delete );
        actRemoveBridge->setShortcutContext(Qt::WidgetShortcut);
        connect(actRemoveBridge,SIGNAL(triggered()),
                this,SLOT(RemoveWithBridge()));
        addAction(actRemoveBridge);

        actRemove = new QAction(QIcon(":/img16x16/delete.png"),tr("Remove with cables"),this);
        actRemove->setShortcut( Qt::CTRL + Qt::Key_Delete );
        actRemove->setShortcutContext(Qt::WidgetShortcut);
        connect(actRemove,SIGNAL(triggered()),
                this,SLOT(close()));
        addAction(actRemove);
    }

    UpdateTitle();
}

void ObjectView::SetConfig(ViewConfig *conf)
{
    config=conf;

    QPalette pal(palette());
    pal.setColor(QPalette::Window, config->GetColor(ColorGroups::Object,Colors::Background) );
    pal.setColor(QPalette::Text, config->GetColor(ColorGroups::Object,Colors::Text) );
    setPalette( pal );

    connect( config, SIGNAL(ColorChanged(ColorGroups::Enum,Colors::Enum,QColor)),
            this, SLOT(UpdateColor(ColorGroups::Enum,Colors::Enum,QColor)) );
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

/*!
  Set the model index of this object
  \param index the index
  */
void ObjectView::SetModelIndex(const MetaInfo &info)
{

    if(info.Type() != MetaTypes::bridge) {
        if(!actRemoveBridge) {
            actRemoveBridge = new QAction(QIcon(":/img16x16/delete.png"),tr("Remove"),this);
            actRemoveBridge->setShortcut( Qt::Key_Delete );
            actRemoveBridge->setShortcutContext(Qt::WidgetShortcut);
            connect(actRemoveBridge,SIGNAL(triggered()),
                    this,SLOT(RemoveWithBridge()));
            addAction(actRemoveBridge);
        }

        if(!actRemove) {
            actRemove = new QAction(QIcon(":/img16x16/delete.png"),tr("Remove with cables"),this);
            actRemove->setShortcut( Qt::CTRL + Qt::Key_Delete );
            actRemove->setShortcutContext(Qt::WidgetShortcut);
            connect(actRemove,SIGNAL(triggered()),
                    this,SLOT(close()));
            addAction(actRemove);
        }

        if(!actShowEditor) {
            actShowEditor = new QAction(tr("Show Editor"),this);
            actShowEditor->setShortcut( Qt::Key_E );
            actShowEditor->setShortcutContext(Qt::WidgetWithChildrenShortcut);
            actShowEditor->setEnabled(false);
            actShowEditor->setCheckable(true);
            connect(actShowEditor,SIGNAL(toggled(bool)),
                    this,SLOT(SwitchEditor(bool)));
            addAction(actShowEditor);
        }

        if(!actLearnSwitch) {
            actLearnSwitch = new QAction(tr("Learn Mode"),this);
            actLearnSwitch->setShortcut( Qt::Key_L );
            actLearnSwitch->setShortcutContext(Qt::WidgetWithChildrenShortcut);
            actLearnSwitch->setEnabled(false);
            actLearnSwitch->setCheckable(true);
            connect(actLearnSwitch,SIGNAL(toggled(bool)),
                    this,SLOT(SwitchLearnMode(bool)));
            addAction(actLearnSwitch);
        }
    }

    UpdateTitle();
}

void ObjectView::UpdateTitle()
{
    if(titleText) {
        QString newTitle = Name();

        if(Meta(MetaInfos::Dirty).toBool())
            newTitle.prepend("*");

        if(Meta(MetaInfos::DoublePrecision).toBool())
            newTitle.append("(D)");

        titleText->setText( newTitle );
    }
}

/*!
  Update the view, base on the model index
  */
void ObjectView::UpdateModelIndex(const MetaInfo &info)
{
    *(MetaInfo*)this=info;

    setToolTip(info.toStringFull());

    if(Meta(MetaInfos::Position).isValid())
        setPos( Meta(MetaInfos::Position).toPointF() );

    if(Meta(MetaInfos::errorMessage).isValid())
        SetErrorMessage( Meta(MetaInfos::errorMessage).toString() );

    UpdateTitle();
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
    emit RemoveWithCables(info());
    event->ignore();
}

void ObjectView::RemoveWithBridge()
{
    setActive(false);
    emit RemoveKeepCables(info());
}

/*!
  Reimplements QGraphicsItem::focusInEvent
  draw a border around focused object
  */
void ObjectView::focusInEvent ( QFocusEvent * event )
{
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
  Reimplements QGraphicsWidget::mouseReleaseEvent \n
  update the model with the new position
  */
void ObjectView::mouseReleaseEvent ( QGraphicsSceneMouseEvent * event )
{
    QGraphicsWidget::mouseReleaseEvent(event);
    SetMeta(MetaInfos::Position,pos());
//    model->setData(objIndex,pos(),UserRoles::position);
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

void ObjectView::SetEditorPin(ConnectablePinView *pin, float value)
{
    editorPin = pin;

    if(!actShowEditor) {
        actShowEditor = new QAction(tr("Show Editor"),this);
        actShowEditor->setShortcut( Qt::Key_E );
        actShowEditor->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        actShowEditor->setEnabled(false);
        actShowEditor->setCheckable(true);
        connect(actShowEditor,SIGNAL(toggled(bool)),
                this,SLOT(SwitchEditor(bool)));
        addAction(actShowEditor);
    }
    actShowEditor->setEnabled(editorPin);
    actShowEditor->setChecked(value>.5f);
}

void ObjectView::SetLearnPin(ConnectablePinView *pin, float value)
{
    learnPin = pin;

    if(!actLearnSwitch) {
        actLearnSwitch = new QAction(tr("Learn Mode"),this);
        actLearnSwitch->setShortcut( Qt::Key_L );
        actLearnSwitch->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        actLearnSwitch->setEnabled(false);
        actLearnSwitch->setCheckable(true);
        connect(actLearnSwitch,SIGNAL(toggled(bool)),
                this,SLOT(SwitchLearnMode(bool)));
        addAction(actLearnSwitch);
    }
    actLearnSwitch->setEnabled(learnPin);
    actLearnSwitch->setChecked(value>.33f);
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

    if(editorPin->GetValue() > .5f && !show)
        editorPin->ValueChanged(.0f);
    if(editorPin->GetValue() < .5f && show)
        editorPin->ValueChanged(1.0f);
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
