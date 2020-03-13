/**************************************************************************
#    Copyright 2010-2012 RaphaÃ«l FranÃ§ois
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

#include "gradientwidgethue.h"

using namespace View;

const int default_diameter = 5;


GradientWidgetHue::GradientWidgetHue(QWidget *parent) :
        QWidget(parent),
        m_cursor_position(rect().center()),
        m_left_button_pressed(false)
{
    m_gradient_image = QImage(128, 128, QImage::Format_RGB32);
    setMainColor(Qt::red);
    updateGradientImage();
}

void GradientWidgetHue::enterEvent(QEvent *event)
{
    Q_UNUSED(event)
    setCursor(Qt::CrossCursor);
}

void GradientWidgetHue::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)
    unsetCursor();
}

void GradientWidgetHue::mouseMoveEvent(QMouseEvent *event)
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

void GradientWidgetHue::mousePressEvent(QMouseEvent *event)
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

void GradientWidgetHue::mouseReleaseEvent(QMouseEvent *event)
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

void GradientWidgetHue::paintEvent(QPaintEvent *event)
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

void GradientWidgetHue::resizeEvent(QResizeEvent * /*event*/)
{
    updateCursorPosition();
}

void GradientWidgetHue::setMainColor(const QColor &col)
{
    updateHueFromColor(col);
//    updateGradientImage();
}

void GradientWidgetHue::updateHueFromColor(const QColor &col)
{
    col.getHsvF(&hue,&saturation,&value,&alpha);
    if(hue<0.0)
        hue=0.0;
    m_selected_color.setHsvF(hue,saturation,value,alpha);
    updateCursorPosition();
}

void GradientWidgetHue::setRed(int r)
{
    QColor newCol( QColor::fromHsvF(hue,saturation,value,alpha) );
    newCol.setRed(r);
    setMainColor(newCol);
}

void GradientWidgetHue::setGreen(int g)
{
    QColor newCol( QColor::fromHsvF(hue,saturation,value,alpha) );
    newCol.setGreen(g);
    setMainColor(newCol);
}
void GradientWidgetHue::setBlue(int b)
{
    QColor newCol( QColor::fromHsvF(hue,saturation,value,alpha) );
    newCol.setBlue(b);
    setMainColor(newCol);
}
void GradientWidgetHue::setAlpha(int a)
{
    alpha=(qreal)a/255;
    m_selected_color.setHsvF(hue,saturation,value,alpha);
    updateCursorPosition();
}
void GradientWidgetHue::setHue(int h)
{
    hue=(qreal)h/359;
    m_selected_color.setHsvF(hue,saturation,value,alpha);
    updateCursorPosition();
}

void GradientWidgetHue::setSaturation(int s)
{
    saturation=(qreal)s/255;
    m_selected_color.setHsvF(hue,saturation,value,alpha);
    updateCursorPosition();
//    updateGradientImage();
}
void GradientWidgetHue::setValue(int v)
{
    value=(qreal)v/255;
    m_selected_color.setHsvF(hue,saturation,value,alpha);
    updateCursorPosition();
//    updateGradientImage();
}

void GradientWidgetHue::updateGradientImage()
{
    QPainter painter(&m_gradient_image);

    painter.setCompositionMode(QPainter::CompositionMode_Source);

    //alpha gradient
    QLinearGradient alphaGrad(0,-1,0,m_gradient_image.height()+1);
    alphaGrad.setColorAt(0.0, QColor(255,255,255,255));
    alphaGrad.setColorAt(1.0, QColor(255,255,255,0));
    painter.setBrush(alphaGrad);
    painter.drawRect(-1,-1,m_gradient_image.width()+1,m_gradient_image.height()+1);

    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);

    //hue gradient
    QLinearGradient hueGrad(-1,0,m_gradient_image.width()+1,0);
    hueGrad.setColorAt(0.00, QColor::fromHsvF(0.00, 1.0, 1.0, 1.0));
    hueGrad.setColorAt(0.33, QColor::fromHsvF(0.33, 1.0, 1.0, 1.0));
    hueGrad.setColorAt(0.66, QColor::fromHsvF(0.66, 1.0, 1.0, 1.0));
    hueGrad.setColorAt(1.00, QColor::fromHsvF(1.00, 1.0, 1.0, 1.0));
    painter.setBrush(hueGrad);
    painter.drawRect(-1,-1,m_gradient_image.width()+1,m_gradient_image.height()+1);

    painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);

    //checker background
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(130,130,130));
    for(int x=0;x<m_gradient_image.width();x+=20) {
        for(int y=0;y<m_gradient_image.height();y+=20) {
            painter.drawRect(x,y,10,10);
            painter.drawRect(x+10,y+10,10,10);
        }
    }
    painter.setBrush(QColor(125,125,125));
    painter.drawRect(m_gradient_image.rect());
    painter.end();

    update();
}

void GradientWidgetHue::cursorMoved()
{
    hue=(qreal)m_cursor_position.x()/(width()-1);
    alpha=1.0-(qreal)m_cursor_position.y()/(height()-1);
    m_selected_color.setHsvF(hue,saturation,value,alpha);
    update();
}

void GradientWidgetHue::updateCursorPosition()
{
    m_cursor_position.setX( hue*(width()-1) );
    m_cursor_position.setY( (1.0-alpha)*(height()-1) );
    update();
}
