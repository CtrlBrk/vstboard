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

#include "resizehandle.h"

ResizeHandle::ResizeHandle(QWidget *parent) :
    QWidget(parent)
{
//    setAutoFillBackground(true);
//    setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    resize(13,13);
}

//QSize ResizeHandle::sizeHint() const
//{
//    QStyleOption opt(0);
//    opt.init(this);
//    return (style()->sizeFromContents(QStyle::CT_SizeGrip, &opt, QSize(13, 13), this).
//            expandedTo(QApplication::globalStrut()));
//}

void ResizeHandle::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    QStyleOptionSizeGrip opt;
    opt.init(this);
    opt.corner = Qt::BottomRightCorner;
    style()->drawControl(QStyle::CE_SizeGrip, &opt, &painter, this);
}

void ResizeHandle::mousePressEvent ( QMouseEvent * event )
{
    if (event->button() != Qt::LeftButton) {
        event->ignore();
        return;
    }

    setCursor(Qt::ClosedHandCursor);
    offset = event->pos();
}

void ResizeHandle::mouseMoveEvent ( QMouseEvent * event )
{
    QPoint dest( mapToParent(event->pos()) - offset );
    if(dest.x()<100) dest.setX(100);
    if(dest.y()<50) dest.setY(50);

    move( dest );
    emit Moved( geometry().bottomRight() );
}

void ResizeHandle::mouseReleaseEvent (  QMouseEvent * /*event*/ )
{
    emit Moved( geometry().bottomRight() );
    setCursor(Qt::OpenHandCursor);
}
