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

#include "maingraphicsview.h"
#include "globals.h"
#include "programmanager.h"
#include "viewconfig.h"

//using namespace View;

MainGraphicsView::MainGraphicsView(QWidget * parent) :
    QGraphicsView(parent),
    currentProgId(0),
    moving(false),
    actZoomIn(0),
    actZoomOut(0),
    actZoomReset(0),
    config(0)
{
    actZoomIn = new QAction(tr("Zoom in"), this);
    actZoomIn->setShortcutContext(Qt::WidgetShortcut);
    connect(actZoomIn,SIGNAL(triggered()),
            this, SLOT(zoomIn()));
    addAction(actZoomIn);

    actZoomOut = new QAction(tr("Zoom out"), this);
    actZoomOut->setShortcutContext(Qt::WidgetShortcut);
    connect(actZoomOut,SIGNAL(triggered()),
            this, SLOT(zoomOut()));
    addAction(actZoomOut);

    actZoomReset = new QAction(tr("Zoom reset"), this);
    actZoomReset->setShortcutContext(Qt::WidgetShortcut);
    connect(actZoomReset,SIGNAL(triggered()),
            this,SLOT(zoomReset()));
    addAction(actZoomReset);
}

void MainGraphicsView::SetViewConfig(View::ViewConfig *conf)
{
    config = conf;
    connect(config->keyBinding,SIGNAL(BindingChanged()),
            this,SLOT(UpdateKeyBinding()));
    UpdateKeyBinding();
}

void MainGraphicsView::UpdateKeyBinding()
{
    if(actZoomIn) actZoomIn->setShortcut( config->keyBinding->GetMainShortcut(KeyBind::zoomIn) );
    if(actZoomOut) actZoomOut->setShortcut( config->keyBinding->GetMainShortcut(KeyBind::zoomOut) );
    if(actZoomReset) actZoomReset->setShortcut( config->keyBinding->GetMainShortcut(KeyBind::zoomReset) );
}

void MainGraphicsView::ForceResize()
{
    emit viewResized( mapToScene(viewport()->rect()).boundingRect() );
}

void MainGraphicsView::wheelEvent(QWheelEvent * event)
{
    event->ignore();

    QGraphicsSceneWheelEvent wheelEvent(QEvent::GraphicsSceneWheel);
    wheelEvent.setWidget(viewport());
    wheelEvent.setScenePos(mapToScene(event->pos()));
    wheelEvent.setScreenPos(event->globalPos());
    wheelEvent.setButtons(event->buttons());
    wheelEvent.setModifiers(event->modifiers());
    wheelEvent.setDelta(event->delta());
    wheelEvent.setOrientation(event->orientation());
    wheelEvent.setAccepted(false);
    QApplication::sendEvent(scene(), &wheelEvent);
    event->setAccepted(wheelEvent.isAccepted());
    if(event->isAccepted())
        return;

    const KeyBind::MoveBind b = config->keyBinding->GetMoveSortcuts(KeyBind::zoom);
    if(b.input == KeyBind::mouseWheel && b.modifier == event->modifiers()) {
        event->accept();
        if(event->delta()>0)
            zoomIn();
        else
            zoomOut();
        return;
    }

}

void MainGraphicsView::mousePressEvent ( QMouseEvent * event )
{
    event->setAccepted(false);
    QGraphicsView::mousePressEvent(event);

    if(!event->isAccepted()) {
        {
            const KeyBind::MoveBind b = config->keyBinding->GetMoveSortcuts(KeyBind::zoomResetMouse);
            if(b.input == KeyBind::none && b.buttons == event->buttons() && b.modifier == event->modifiers()) {
                event->accept();
                zoomReset();
                return;
            }
        }

        {
            const KeyBind::MoveBind b = config->keyBinding->GetMoveSortcuts(KeyBind::moveView);
            if(b.input == KeyBind::mouse && b.buttons == event->buttons() && b.modifier == event->modifiers()) {
                event->accept();
                startMovePos=event->pos();
                startDragMousePos=event->pos();
                moving=true;
                return;
            }
        }
    }

    if(moving)
        moving=false;
}

void MainGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    event->setAccepted(false);
    QGraphicsView::mouseMoveEvent(event);
    if(event->isAccepted())
        return;

    if(moving && QLineF(event->pos(), startDragMousePos).length() > QApplication::startDragDistance()) {
        event->accept();
        QScrollBar *hBar = horizontalScrollBar();
        QScrollBar *vBar = verticalScrollBar();
        QPoint delta = event->pos() - startMovePos;
        hBar->setValue(hBar->value() + (isRightToLeft() ? delta.x() : -delta.x()));
        vBar->setValue(vBar->value() - delta.y());
        startMovePos=event->pos();
    }
}

void MainGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    if(moving) {
        event->accept();
        moving=false;
        return;
    }
    QGraphicsView::mouseReleaseEvent(event);
}

//bool MainGraphicsView::event(QEvent *event)
//{
//    if(event->type() == Event::Shortcut) {
//        ShortcutEvent *e = static_cast<ShortcutEvent*>(event);
//        if(e->value == true) {
//            switch(e->id) {
//                case 0:
//                    zoomIn();
//                    break;
//                case 1:
//                    zoomOut();
//                    break;
//                default :
//                    break;
//            }
//        }
//        return true;
//    }
//    return QGraphicsView::event(event);
//}

void MainGraphicsView::zoomIn()
{
    scale(1.2, 1.2);
    emit viewResized( mapToScene(viewport()->rect()).boundingRect() );
}

void MainGraphicsView::zoomOut()
{
    scale(1.0/1.2, 1.0/1.2);
    emit viewResized( mapToScene(viewport()->rect()).boundingRect() );
}

void MainGraphicsView::zoomReset()
{
    setTransform( QTransform() );
    emit viewResized( mapToScene(viewport()->rect()).boundingRect() );
}

void MainGraphicsView::paintEvent ( QPaintEvent * event )
{
    emit viewResized( mapToScene(viewport()->rect()).boundingRect() );
    QGraphicsView::paintEvent(event);
}

void MainGraphicsView::scrollContentsBy ( int dx, int dy )
{
    QGraphicsView::scrollContentsBy(dx,dy);
    emit viewResized( mapToScene(viewport()->rect()).boundingRect() );
}

void MainGraphicsView::resizeEvent ( QResizeEvent * event )
{
    QGraphicsView::resizeEvent(event);
//    emit viewResized( mapToScene(viewport()->rect()).boundingRect() );
}

void MainGraphicsView::SaveProgram()
{
    SceneProg state;
    state.scale=transform().m11();
    state.scrollx=horizontalScrollBar()->value();
    state.scrolly=verticalScrollBar()->value();
    listPrograms.insert(currentProgId,state);
}

void MainGraphicsView::SetViewProgram(quint32 progId)
{
    if(progId!=currentProgId)
        SaveProgram();

    currentProgId = progId;
    if(!listPrograms.contains(progId)) {
        setTransform( QTransform() );
        horizontalScrollBar()->setValue( 0 );
        verticalScrollBar()->setValue( 0 );
        SaveProgram();
        return;
    }

    SceneProg state = listPrograms.value(currentProgId);
    setTransform( QTransform() );
    scale( state.scale, state.scale );
    horizontalScrollBar()->setValue( state.scrollx );
    verticalScrollBar()->setValue( state.scrolly );
}

void MainGraphicsView::RemoveViewProgram(quint32 progId)
{
    if(!listPrograms.contains(progId))
        return;
    listPrograms.remove(progId);
}

void MainGraphicsView::ClearViewPrograms()
{
    SetViewProgram(EMPTY_PROGRAM);
    listPrograms.clear();
}

QDataStream & MainGraphicsView::toStream (QDataStream& out) const
{
    out << (quint16)listPrograms.size();
    QMap<qint16,SceneProg>::ConstIterator i = listPrograms.constBegin();
    while(i!=listPrograms.constEnd()) {
        qint16 prog = i.key();
        out << prog;
        SceneProg state = i.value();
        out << state.scale;
        out << state.scrollx;
        out << state.scrolly;
        ++i;
    }
    return out;
}

QDataStream & MainGraphicsView::fromStream (QDataStream& in)
{
    ClearViewPrograms();

    quint16 nbProg;
    in >> nbProg;

    for(quint16 i=0; i<nbProg; i++) {
        qint16 key;
        in >> key;
        SceneProg state;
        in >> state.scale;
        in >> state.scrollx;
        in >> state.scrolly;
        listPrograms.insert(key,state);
    }
    return in;
}

void MainGraphicsView::ProgramToStream (int progId, QDataStream &out)
{
    if(progId == currentProgId)
        SaveProgram();

    if(!listPrograms.contains(progId)) {
        out << (quint8)0;
        return;
    }

    out << (quint8)1;

    SceneProg p = listPrograms.value(progId);
    out << p.scale;
    out << p.scrollx;
    out << p.scrolly;
}

void MainGraphicsView::ProgramFromStream (int progId, QDataStream &in)
{
    quint8 valid=0;
    in >> valid;
    if(valid==0)
        return;

    SceneProg p;
    in >> p.scale;
    in >> p.scrollx;
    in >> p.scrolly;
    listPrograms.insert(progId,p);

    if(progId==currentProgId)
        SetViewProgram(progId);
}

QDataStream & operator<< (QDataStream & out, const MainGraphicsView& value)
{
    return value.toStream(out);
}

QDataStream & operator>> (QDataStream & in, MainGraphicsView& value)
{
    return value.fromStream(in);
}
