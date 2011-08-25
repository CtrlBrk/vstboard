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

#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QtCore/QCoreApplication>

#include "mainhosthost.h"
#include "mainwindowhost.h"
#include "objectinfo.h"

class TestTest : public QObject
{
    Q_OBJECT

public:
    TestTest();
    MainHostHost host;

private Q_SLOTS:
    void testCase1();
};

TestTest::TestTest()
{
//    int argc=0;
//    char *argv=new char[0];
//    QApplication app(argc,&argv);

    EngineThread *engine = new EngineThread();

    host.moveToThread(engine);
    QMetaObject::invokeMethod(&host,"InitThread",Qt::BlockingQueuedConnection);

    MainWindowHost w(&host);
    host.SetMainWindow(&w);
    w.Init();


    w.readSettings();
    w.show();
    w.LoadDefaultFiles();
//    qapp.exec();
    delete engine;
}

void TestTest::testCase1()
{
    host.LoadFile("C:/Users/CtrlBrk/Documents/banks/t.vstbproject");
    QVERIFY2(true, "Failure");
}

QTEST_MAIN(TestTest);

#include "tst_testtest.moc"
