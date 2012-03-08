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

#ifndef RESIZEHANDLE_H
#define RESIZEHANDLE_H

#include "precomp.h"

class ResizeHandle : public QWidget
{
    Q_OBJECT
public:
    explicit ResizeHandle(QWidget *parent = 0);
//    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent ( QMouseEvent * event );
    void mouseMoveEvent ( QMouseEvent * event );
    void mouseReleaseEvent (  QMouseEvent * event );

    QPoint offset;

signals:
    void Moved(const QPoint &pt);

public slots:

};

#endif // RESIZEHANDLE_H
