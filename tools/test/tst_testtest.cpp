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

#include "audiobuffer.h"
#include "circularbuffer.h"
#include "renderer/optimizemap.h"

class TestBuffers : public QObject
{
    Q_OBJECT

public:
    TestBuffers();

    AudioBuffer *pinInBuf;
    AudioBuffer *pinOutBuf;
    CircularBuffer *ring;
    float *buf;
    float *bufOut;

    int bufSize;
    int ringSize;

private Q_SLOTS:
    void writeToRing();
 };

TestBuffers::TestBuffers()
{
    bufSize = 200;
    ringSize = 450;

    pinInBuf = new AudioBuffer(false,false);
    pinInBuf->SetSize(bufSize);
    pinOutBuf = new AudioBuffer(false,false);
    pinOutBuf->SetSize(bufSize);

    ring = new CircularBuffer(ringSize);

    buf = new float[bufSize];
    for(int i=0; i<bufSize; ++i) {
        buf[i]=1+.01f*i;
    }
    bufOut = new float[bufSize];
    for(int i=0; i<bufSize; ++i) {
        bufOut[i]=.0f;
    }

    ring->Put( buf, bufSize );
    ring->Put( buf, bufSize );
//    ring->Get( bufOut, bufSize );
//    ring->Put( buf, bufSize );
}

void TestBuffers::writeToRing()
{
    QBENCHMARK {
        for(int i=0; i<100000; i++) {
            ring->Put( buf, bufSize );
            ring->Get( bufOut, bufSize );

        }
    }

//    for(int i=0; i<bufSize; i++) {
//        qDebug()<<buf[i]<<bufOut[i];
//    }
//    QVERIFY2(true, "Failure");
}


class TestOptimizer : public QObject
{
    Q_OBJECT

public:
    QList<OptimizerNode*> nodes;
    int nbThreads;

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void Test();
 };

void TestOptimizer::initTestCase()
{
    nbThreads = 4;

    int id=0;
//    nodes << new OptimizerNode(id++,29484,2,2);
//    nodes << new OptimizerNode(id++,2496,0,0);
//    nodes << new OptimizerNode(id++,4212,0,0);
//    nodes << new OptimizerNode(id++,1560,1,1);
//    nodes << new OptimizerNode(id++,93756,0,2);
//    nodes << new OptimizerNode(id++,2808,1,1);
//    nodes << new OptimizerNode(id++,471744,2,2);
//    nodes << new OptimizerNode(id++,431652,0,2);
//    nodes << new OptimizerNode(id++,100,3,3);

    nodes << new OptimizerNode(id++,0,2,2);
    nodes << new OptimizerNode(id++,0,0,0);
    nodes << new OptimizerNode(id++,0,0,0);
    nodes << new OptimizerNode(id++,1560,1,1);
    nodes << new OptimizerNode(id++,0,0,2);
    nodes << new OptimizerNode(id++,0,1,1);
    nodes << new OptimizerNode(id++,0,2,2);
    nodes << new OptimizerNode(id++,0,0,2);
    nodes << new OptimizerNode(id++,101,3,3);
    OptimizeMap opt(nodes, nbThreads );

//    id=0;
//    QList<OptimizerNode*> nodes2;
//    nodes2 << new OptimizerNode(id++,105,3,3);
//    nodes2 << new OptimizerNode(id++,29484,2,2);
//    nodes2 << new OptimizerNode(id++,2496,0,0);
//    nodes2 << new OptimizerNode(id++,2808,1,1);
//    nodes2 << new OptimizerNode(id++,471744,2,2);
//    nodes2 << new OptimizerNode(id++,431652,0,2);
//    nodes2 << new OptimizerNode(id++,4212,0,0);
//    nodes2 << new OptimizerNode(id++,1560,1,1);
//    nodes2 << new OptimizerNode(id++,93756,0,2);
//    OptimizeMap opt2(nodes2, nbThreads );

//    id=0;
//    QList<OptimizerNode*> nodes3;
//    nodes3 << new OptimizerNode(id++,431652,0,2);
//    nodes3 << new OptimizerNode(id++,4212,0,0);
//    nodes3 << new OptimizerNode(id++,1560,1,1);
//    nodes3 << new OptimizerNode(id++,93756,0,2);
//    nodes3 << new OptimizerNode(id++,110,3,3);
//    nodes3 << new OptimizerNode(id++,29484,2,2);
//    nodes3 << new OptimizerNode(id++,2496,0,0);
//    nodes3 << new OptimizerNode(id++,2808,1,1);
//    nodes3 << new OptimizerNode(id++,471744,2,2);
//    OptimizeMap opt3(nodes3, nbThreads );

//    QVERIFY(opt==opt2);
//    QVERIFY(opt!=opt3);
//    QVERIFY(opt2==opt3);

//    qDeleteAll(nodes2);
//    qDeleteAll(nodes3);
}

void TestOptimizer::cleanupTestCase()
{
    qDeleteAll(nodes);
}

void TestOptimizer::Test()
{
    OptMap map;
    OptimizeMap opt(nodes, nbThreads);

    QBENCHMARK {
        map = opt.GetBestMap();
    }

    qDebug() << "best"<<opt.bestTime<<"nbIter"<<opt.nbIter<<"skipedIter"<<opt.skipedIter;
    qDebug() << OptimizeMap::OptMap2Txt(map);
}

QTEST_APPLESS_MAIN(TestOptimizer)

#include "tst_testtest.moc"
