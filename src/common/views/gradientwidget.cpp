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

#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>

#include "gradientwidget.h"

using namespace View;

const int default_diameter = 5;

GradientWidget::GradientWidget(QWidget *parent) :
        QWidget(parent), 
        m_cursor_position(rect().center()),
        m_left_button_pressed(false)
{
    m_gradient_image = QImage(128, 128, QImage::Format_RGB32);
    setMainColor(Qt::red);
    updateGradientImage();
}

void GradientWidget::enterEvent(QEvent *event)
{
    Q_UNUSED(event)
    setCursor(Qt::CrossCursor);
}

void GradientWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)
    unsetCursor();
}

void GradientWidget::mouseMoveEvent(QMouseEvent *event)
{
     if (!m_left_button_pressed) {
        event->ignore();
        return;
    }

    m_cursor_position.setX(event->pos().x());
    if (m_cursor_position.x() >= rect().width())
        m_cursor_position.setX(rect().width() - 1);
    if (m_cursor_position.x() < 0)
        m_cursor_position.setX(0);

    m_cursor_position.setY(event->pos().y());
    if (m_cursor_position.y() >= rect().height())
        m_cursor_position.setY(rect().height() - 1);
    if (m_cursor_position.y() < 0)
        m_cursor_position.setY(0);

    cursorMoved();
    emit colorSelected(m_selected_color);
    event->accept();
}

void GradientWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        event->ignore();
        return;
    }

    m_cursor_position = event->pos();
    cursorMoved();
    emit colorSelected(m_selected_color);
    m_left_button_pressed = true;
    event->accept();
}

void GradientWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        event->ignore();
        return;
    }

    if (rect().contains(event->pos())) {
        m_cursor_position = event->pos();
        cursorMoved();
        emit colorSelected(m_selected_color);
    }

    m_left_button_pressed = false;
    event->accept();
}

void GradientWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.drawImage(rect(), m_gradient_image);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_Darken);
    painter.setBrush(QColor(110,110,110));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(m_cursor_position, 5, 5);
    painter.setCompositionMode(QPainter::CompositionMode_Lighten);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QColor(140,140,140));
    painter.drawEllipse(m_cursor_position, 5, 5);
    painter.setRenderHint(QPainter::Antialiasing,false);
    painter.end();
}

void GradientWidget::resizeEvent(QResizeEvent * /*event*/)
{
    updateCursorPosition();
}

void GradientWidget::setMainColor(const QColor &col)
{
    col.getHsvF(&hue,&saturation,&value,&alpha);
    if(hue<0.0)
        hue=0.0;
    m_selected_color.setHsvF(hue,saturation,value,alpha);
    updateCursorPosition();
    updateGradientImage();
}

void GradientWidget::setRed(int r)
{
    QColor newCol( QColor::fromHsvF(hue,saturation,value,alpha) );
    newCol.setRed(r);
    setMainColor(newCol);
}

void GradientWidget::setGreen(int g)
{
    QColor newCol( QColor::fromHsvF(hue,saturation,value,alpha) );
    newCol.setGreen(g);
    setMainColor(newCol);
}
void GradientWidget::setBlue(int b)
{
    QColor newCol( QColor::fromHsvF(hue,saturation,value,alpha) );
    newCol.setBlue(b);
    setMainColor(newCol);
}
void GradientWidget::setAlpha(int a)
{
    alpha=(qreal)a/255;
    m_selected_color.setHsvF(hue,saturation,value,alpha);
    updateGradientImage();
}
void GradientWidget::setHue(int h)
{
    hue=(qreal)h/359;
    m_selected_color.setHsvF(hue,saturation,value,alpha);
    updateGradientImage();
}
void GradientWidget::setSaturation(int s)
{
    saturation=(qreal)s/255;
    m_selected_color.setHsvF(hue,saturation,value,alpha);
//    updateGradientImage();
}
void GradientWidget::setValue(int v)
{
    value=(qreal)v/255;
    m_selected_color.setHsvF(hue,saturation,value,alpha);
//    updateGradientImage();
}

void GradientWidget::updateGradientImage()
{
    QPainter painter(&m_gradient_image);

    //checker background
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(125,125,125));
    painter.drawRect(m_gradient_image.rect());
    painter.setBrush(QColor(130,130,130));
    for(int x=0;x<m_gradient_image.width();x+=20) {
        for(int y=0;y<m_gradient_image.height();y+=20) {
            painter.drawRect(x,y,10,10);
            painter.drawRect(x+10,y+10,10,10);
        }
    }

    //saturation gradient
    QLinearGradient hueGrad(0,0,m_gradient_image.width(),0);
    hueGrad.setColorAt(0, QColor::fromHsvF(hue, 0.0, 1.0, alpha));
    hueGrad.setColorAt(1, QColor::fromHsvF(hue, 1.0, 1.0, alpha));
    painter.setBrush(hueGrad);
    painter.drawRect(m_gradient_image.rect());

    //lightness gradient
    QLinearGradient lumGrad(0,0,0,m_gradient_image.height());
    lumGrad.setColorAt(0, QColor(0,0,0,0));
    lumGrad.setColorAt(1, QColor::fromRgbF(0,0,0,alpha));
    painter.setBrush(lumGrad);
    painter.drawRect(m_gradient_image.rect());

    update();
}

void GradientWidget::cursorMoved()
{
    saturation=(qreal)m_cursor_position.x()/(width()-1);
    value=1.0 - (qreal)m_cursor_position.y()/(height()-1);
    m_selected_color.setHsvF(hue,saturation,value,alpha);

    update();
}

void GradientWidget::updateCursorPosition()
{
    m_cursor_position.setX( saturation*(width()-1) );
    m_cursor_position.setY( (1.0-value)*(height()-1) );
    update();
}
