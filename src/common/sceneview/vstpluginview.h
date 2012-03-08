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

#ifndef VSTPLUGINVIEW_H
#define VSTPLUGINVIEW_H

#include "connectableobjectview.h"

namespace View {
    class VstPluginView : public ConnectableObjectView
    {
    Q_OBJECT
    public:
        VstPluginView(ViewConfig *config,MsgController *msgCtrl, int objId, MainContainerView * parent = 0);
        void Init(const MsgObject &msg);

    protected:
        void dragEnterEvent( QGraphicsSceneDragDropEvent *event);
        void dragLeaveEvent( QGraphicsSceneDragDropEvent *event);
        void dropEvent( QGraphicsSceneDragDropEvent *event);
        QAction *actSaveBank;
        QAction *actSaveBankAs;
        QAction *actSaveProgram;
        QAction *actSaveProgramAs;

    public slots:
        void SaveBankAs();
        void SaveBank();
        void SaveProgramAs();
        void SaveProgram();
        void UpdateColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color);
        void HighlightStart();
        void HighlightStop();
        void UpdateKeyBinding();
    };
}

#endif // VSTPLUGINVIEW_H
