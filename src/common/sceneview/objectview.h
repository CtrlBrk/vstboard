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

#ifndef OBJECTVIEW_H
#define OBJECTVIEW_H

//#include "precomp.h"
#include "globals.h"
#include "connectables/object.h"
#include "listpinsview.h"
//#include "textbutton.h"
#include "mainwindow.h"
#include "msghandler.h"

namespace View {

    class MinMaxPinView;
    class PinView;
    class ObjectView : public QGraphicsWidget, public MsgHandler
    {
    Q_OBJECT
    public:
        explicit ObjectView(ViewConfig *config, MsgController *msgCtrl, int objId, QGraphicsItem * parent = 0);
        virtual ~ObjectView();
        virtual void Init(const MsgObject &msg);
        virtual void ReceiveMsg(const MsgObject &msg);
        void Shrink();

        /// list of audio inputs
        ListPinsView *listAudioIn;

        /// list of audio outputs
        ListPinsView *listAudioOut;

        /// list of midi inputs
        ListPinsView *listMidiIn;

        /// list of midi outputs
        ListPinsView *listMidiOut;

        /// list of parameters pins inputs
        ListPinsView *listParametersIn;

        /// list of parameters pins outputs
        ListPinsView *listParametersOut;

        /// list of bridges pins
        ListPinsView *listBridge;

        ViewConfig *config;

        void SetEditorPin(MinMaxPinView *pin, float value);
        void SetLearnPin(MinMaxPinView *pin, float value);
        void SetBypassPin(MinMaxPinView *pin, float value);

        ColorGroups::Enum myColorGroupId = ColorGroups::Object;

    protected:
        void AddListPins(const MsgObject &msg);

        virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
        void resizeEvent ( QGraphicsSceneResizeEvent * event );
        virtual void closeEvent ( QCloseEvent * event );
        virtual void focusInEvent ( QFocusEvent * event );
        virtual void focusOutEvent ( QFocusEvent * event );
        void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

        void SetErrorMessage(const QString & msg);
        void UpdateTitle(const QString &name);

        /// the title text
        QGraphicsSimpleTextItem *titleText;

        /// the object border
        QGraphicsRectItem *border;

        /// the selected object border
        QGraphicsRectItem *selectBorder;

        /// error message icon
        QGraphicsPixmapItem *errorMessage;

        /// objects layout
        QGraphicsGridLayout *layout;

        /// objects model index
        QPersistentModelIndex objIndex;

        /// the remove action
        QAction *actRemove;
        /// the remove+bridge action
        QAction *actRemoveBridge;
        /// show editor
        QAction *actShowEditor;
        //switch learn mode
        QAction *actLearnSwitch;

        QAction *actToggleBypass;


        /// true if a shrink is already in progress
        bool shrinkAsked;

        bool highlighted;

        MinMaxPinView *editorPin;
        MinMaxPinView *learnPin;
        MinMaxPinView *bypassPin;

        bool editorAutoOpened;

    private slots:
        void SwitchEditor(bool show);
        void SwitchLearnMode(bool on);
        void ToggleBypass(bool b);

    public slots:
        void ShrinkNow();
        virtual void UpdateColor(ColorGroups::Enum groupId, Colors::Enum colorId, const QColor &color);
        virtual void HighlightStart() {}
        virtual void HighlightStop() {}
        void RemoveWithBridge();
        void ToggleEditor();
        virtual void UpdateKeyBinding();

    friend class PinView;
    };
}

#endif // OBJECTVIEW_H
