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

#include "bridgeview.h"

using namespace View;

BridgeView::BridgeView(ViewConfig *config,MsgController *msgCtrl, int objId, QGraphicsItem * parent) :
        ObjectView(config,msgCtrl,objId,parent),
        lLayout(0)
{
    setObjectName("bridgeView");
    setGeometry(QRectF(0,0,0,0));

    lLayout = new QGraphicsLinearLayout(Qt::Horizontal) ;
    lLayout->setSpacing(0);
    lLayout->setContentsMargins(0,0,0,0);
    setLayout(lLayout);

    listBridge = new ListPinsView(config,msgCtrl,-1,this);
    listBridge->layout->setOrientation(Qt::Horizontal);
    lLayout->addItem(listBridge);

    QPalette pal(palette());
    pal.setColor(QPalette::Window, config->GetColor(ColorGroups::Bridge,Colors::Background) );
    setPalette( pal );
}

void BridgeView::UpdateColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color)
{
    if(groupId!=ColorGroups::Bridge)
        return;

    if(colorId==Colors::Background) {
        QPalette pal(palette());
        pal.setColor(QPalette::Window,color);
        setPalette( pal );
    }
}

/*!
  update the connected cables position
  */
void BridgeView::moveEvent ( QGraphicsSceneMoveEvent * /*event*/ )
{
    listBridge->UpdateCablesPosition();
}
