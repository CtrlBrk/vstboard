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
//#ifndef QT_NO_DEBUG
//    #include "vld.h"
//#endif

#include <QLibraryInfo>
#include <QTranslator>

#include "mainhosthost.h"
#include "mainwindowhost.h"
#include "connectables/objectinfo.h"
#include "msgobject.h"

#ifndef QT_NO_DEBUG

    void myMessageOutput(QtMsgType type, const char *msg)
     {
        qInstallMsgHandler(0);

        switch (type) {

            case QtWarningMsg:
                qDebug(msg);
                break;

            case QtCriticalMsg:
            case QtFatalMsg:
                qDebug(msg);
                abort();
                break;

            default:
                qDebug(msg);
                break;

        }

         qInstallMsgHandler(myMessageOutput);
     }
#endif

int main(int argc, char *argv[])
{
    qRegisterMetaType<ConnectionInfo>("ConnectionInfo");
    qRegisterMetaType<ObjectInfo>("ObjectInfo");
    qRegisterMetaType<ObjectContainerAttribs>("ObjectContainerAttribs");
    qRegisterMetaType<MsgObject>("MsgObject");
    qRegisterMetaType<int>("ObjType::Enum");
    qRegisterMetaType<QVariant>("QVariant");
    qRegisterMetaType<AudioBuffer*>("AudioBuffer*");

    qRegisterMetaTypeStreamOperators<ObjectInfo>("ObjectInfo");
    qRegisterMetaTypeStreamOperators<ObjectContainerAttribs>("ObjectContainerAttribs");

#ifndef QT_NO_DEBUG
    qInstallMsgHandler(myMessageOutput);
#endif

    QCoreApplication::setOrganizationName("CtrlBrk");
    QCoreApplication::setApplicationName("VstBoard");

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

#if defined(_M_X64) || defined(__amd64__)
    Settings *set = new Settings("x64/",qApp);
#else
    Settings *set = new Settings("x86/",qApp);
#endif


    MainHostHost host(set);
//    QThread th;
//    host.moveToThread(&th);
//    th.start();

    MainWindowHost w(set, &host);

    host.connect(&host,SIGNAL(SendMsgSignal(MsgObject)),
                &w,SLOT(ReceiveMsgSignal(MsgObject)),
                 Qt::QueuedConnection);
    host.connect(&w,SIGNAL(SendMsgSignal(MsgObject)),
                &host,SLOT(ReceiveMsgSignal(MsgObject)),
                 Qt::QueuedConnection);
//    QTimer::singleShot(0,&host, SLOT(Init()));
    host.Init();
    w.Init();
//    QTimer::singleShot(0,&host, SLOT(Open()));
    host.Open();
    w.readSettings();
    w.show();
    w.LoadDefaultFiles();

    app.exec();

//    QTimer::singleShot(0,&host, SLOT(Kill()));
//    qApp->processEvents();
//    th.quit();
    return 0;
}
