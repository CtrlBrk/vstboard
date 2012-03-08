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

#ifndef VSTPLUGINWINDOW_H
#define VSTPLUGINWINDOW_H

#ifdef VSTSDK

#include "precomp.h"
#include "base/source/fobject.h"
#include "pluginterfaces/gui/iplugview.h"

namespace Ui {
    class VstPluginWindow;
}

namespace Connectables {
    class Object;
    class VstPlugin;
    class Vst3Plugin;
}

class ObjectContainerAttribs;

using namespace Steinberg;

namespace View {


    class VstPluginWindow : public QFrame, public FObject, public IPlugFrame
    {
        Q_OBJECT
    public:
        VstPluginWindow(QWidget *parent = 0);
        ~VstPluginWindow();

        bool SetPlugin(Connectables::Vst3Plugin *plugin);
        bool SetPlugin(Connectables::VstPlugin *plugin);
        void UnsetPlugin();
        WId GetWinId();
//        const QPixmap GetScreenshot();

        void LoadAttribs(const ObjectContainerAttribs &attr);
        void SaveAttribs(ObjectContainerAttribs &attr);

//        tresult PLUGIN_API queryInterface (const TUID iid, void** obj);
//        uint32 PLUGIN_API addRef ();
//        uint32 PLUGIN_API release ();
        tresult PLUGIN_API resizeView (IPlugView* view, ViewRect* newSize);


        //---Interface------
        OBJ_METHODS (VstPluginWindow, FObject)
        DEFINE_INTERFACES
            DEF_INTERFACE (IPlugFrame)
        END_DEFINE_INTERFACES (FObject)
        REFCOUNT_METHODS(FObject)

    protected:
        void resizeEvent ( QResizeEvent * event );
        void showEvent ( QShowEvent * event );
        void closeEvent ( QCloseEvent * event );
        Connectables::Object *plugin;

        QModelIndex modelIndex;

    private:
        Ui::VstPluginWindow *ui;
        bool canResize;

    signals:
        void Hide();

    public slots:
        void SetWindowSize(int newWidth, int newHeight);

    };
}
#endif
#endif // VSTPLUGINWINDOW_H
