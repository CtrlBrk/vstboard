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
//#ifndef QT_NO_DEBUG
//    #include "vld.h"
//#endif

#include "mainhosthost.h"
#include "mainwindowhost.h"
#include "objectinfo.h"


#ifndef QT_NO_DEBUG

    void myMessageOutput(QtMsgType type, const char *msg)
     {
        qInstallMsgHandler(0);
        qDebug(msg);
        qInstallMsgHandler(myMessageOutput);

        if(type!=QtDebugMsg) {
            qDebug("pause");
        }

         switch (type) {

         case QtWarningMsg:
             break;
         case QtCriticalMsg:
         case QtFatalMsg:
//#ifdef _MSC_VER
               // __asm int 3
            __debugbreak();
//#else
//                __asm("int3");
//#endif
             abort();
             break;

         case QtDebugMsg:
             break;
         }
     }
#endif
/*
class MyApp : public QApplication {
public:
    MyApp(int &c, char **v): QApplication(c, v) {}

    bool notify(QObject *rec, QEvent *ev) {
           try {
               return QApplication::notify(rec, ev);
           }
           catch (...) {
               debug("Unknown exception!")
#ifdef _MSC_VER
                __asm int 3
#else
                __asm("int3");
#endif
               abort();
           }
       }
};
*/

int main(int argc, char *argv[])
{
    qRegisterMetaType<MetaInfo>("MetaInfo");
    qRegisterMetaType<int>("MediaTypes::Enum");
    qRegisterMetaType<QVariant>("QVariant");
    qRegisterMetaType<AudioBuffer*>("AudioBuffer*");
    qRegisterMetaType<Qt::CheckState>("Qt::CheckState");
    qRegisterMetaType<ColorGroups::Enum>("ColorGroups::Enum");
    qRegisterMetaType<Colors::Enum>("Colors::Enum");
    qRegisterMetaType<PaHostApiIndex>("PaHostApiIndex");
    qRegisterMetaType<PaDeviceIndex>("PaDeviceIndex");
    qRegisterMetaType<PaTime>("PaTime");
    qRegisterMetaType<QMessageBox::Icon>("QMessageBox::Icon");
    qRegisterMetaType<QMessageBox::StandardButtons>("QMessageBox::StandardButtons");
    qRegisterMetaType<QMessageBox::StandardButton>("QMessageBox::StandardButton");
    qRegisterMetaType<Qt::Orientation>("Qt::Orientation");
    qRegisterMetaType<QUndoCommand*>("QUndoCommand*");
    qRegisterMetaTypeStreamOperators<MetaInfo>("ObjectInfo");

#ifndef QT_NO_DEBUG
    qInstallMsgHandler(myMessageOutput);
#endif

    QCoreApplication::setOrganizationName("CtrlBrk");
    QCoreApplication::setApplicationName("VstBoard");

    //MyApp app(argc, argv);
    QApplication app(argc, argv);

#ifdef QT_NO_DEBUG
    QTranslator qtTranslator;
    if(qtTranslator.load("qt_" + QLocale::system().name(), ":/translations/"))
        app.installTranslator(&qtTranslator);

    QTranslator commonTranslator;
    if(commonTranslator.load("common_" + QLocale::system().name(), ":/translations/"))
        app.installTranslator(&commonTranslator);

    QTranslator myappTranslator;
    if(myappTranslator.load("vstboard_" + QLocale::system().name(), ":/translations/"))
        app.installTranslator(&myappTranslator);
#endif

    EngineThread *engine = new EngineThread();
    MainHostHost host;
    host.moveToThread(engine);
    QMetaObject::invokeMethod(&host,"InitThread",Qt::BlockingQueuedConnection);

    MainWindowHost w(&host);
    host.SetMainWindow(&w);
    w.Init();


    w.readSettings();
    w.show();
    w.LoadDefaultFiles();
    app.exec();
    delete engine;
    return 0;
}
