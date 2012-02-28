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

#ifndef VSTSHELLSELECT_H
#define VSTSHELLSELECT_H

#include "precomp.h"
#include "msghandler.h"
#include "connectables/objectinfo.h"

namespace Ui {
    class VstShellSelect;
}

namespace View {
    class VstShellSelect : public QDialog, public MsgHandler
    {
        Q_OBJECT
    public:
        VstShellSelect(MsgController *msgCtrl, int objId, QWidget *parent=0);
        ~VstShellSelect();

        void ReceiveMsg(const MsgObject & msg);

    private:
        int senderObjId;
        ObjectInfo info;
        Ui::VstShellSelect *ui;

    public slots:
        void accept();
        void reject();
    };
}
#endif // VSTSHELLSELECT_H
