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

#include <QtCore/QString>
#include <QtTest/QtTest>

#include "audiobuffer.h"
#include "circularbuffer.h"
#include "renderer/optimizemap.h"
#include "renderer/solver.h"
#include "renderer/semaphoreinverted.h"
//#include "renderer/optimizernode.h"
#include "renderer/renderer2.h"
#include "renderer/renderernode2.h"

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
//    OptimizeMap opt(nodes, nbThreads );

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
        opt.GetBestMap(map);
    }

    qDebug() << "best"<<opt.bestTime<<"nbIter"<<opt.nbIter<<"skipedIter"<<opt.skipedIter;
    qDebug() << OptimizeMap::OptMap2Txt(map);
}


class TestSem : public QObject
{
    Q_OBJECT

public:
    SemaphoreInverted sem;
    static void unlockThread(SemaphoreInverted *sem);

private Q_SLOTS:
    void initTestCase();
 };

void TestSem::unlockThread(SemaphoreInverted *sem)
{
    Sleep(500);
    LOG("unlock")
    sem->Unlock();
}

void TestSem::initTestCase()
{
   sem.AddLock(1);
   LOG("lock")
   QtConcurrent::run(TestSem::unlockThread,&sem);
   bool ret = sem.WaitUnlock(1000);
   LOG("run"<<ret)
   Sleep(1000);
}

class TestObj : public Connectables::Object
{
public:
    TestObj::TestObj(int id, long cpuTime) :
        Connectables::Object(0,id, ObjectInfo()),
        cpuTime(cpuTime)
    {
        SetSleep(false);
    }

    void Render()
    {
//        Sleep(cpuTime);
    }

    long cpuTime;
};

class TimerRender : public QThread
{
public:
    TimerRender(Renderer2 *renderer, unsigned long t);
    ~TimerRender();
    void run();

private:
    bool stop;
    Renderer2 *renderer;
    unsigned long t;
};

TimerRender::TimerRender(Renderer2 *renderer, unsigned long t) :
    QThread(renderer),
    renderer(renderer),
    stop(false),
    t(t)
{
    start(QThread::NormalPriority);
}

TimerRender::~TimerRender()
{
    stop=true;
    wait();
}

void TimerRender::run()
{
    stop=false;
renderer->StartRender();
    while(!stop) {
        msleep(t);
//        renderer->StartRender();
    }
}

class TestSolver : public QObject
{
    Q_OBJECT

public:
    TimerRender *renderTh;
    Renderer2 renderer;
    Solver solv;
    hashObjects lstObjects;
    hashCables lstCables;
    int nbThreads;

    QList<QSharedPointer<Connectables::Object> >lstObj;

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void Test();
 };

void TestSolver::initTestCase()
{
    nbThreads = 2;

    for(int i=0; i<5; i++) {
        QSharedPointer<Connectables::Object>obj(new TestObj(i, 10));
        lstObj << obj;
        lstObjects.insert(i, obj );
    }
    ConnectionInfo inf1(0,0,PinType::Audio,PinDirection::Output,0,false,false);
    ConnectionInfo inf2(0,1,PinType::Audio,PinDirection::Input,0,false,false);
    QSharedPointer<Connectables::Cable>cab(new Connectables::Cable(0,inf1,inf2));
    lstCables.insert(inf1, cab);

//    renderTh = new TimerRender(&renderer, 15);


}

void TestSolver::cleanupTestCase()
{
//    delete renderTh;
    lstObj.clear();
}

void TestSolver::Test()
{
    RenderMap rMap;
//    solv.GetMap(lstObjects,lstCables,7,rMap);

    for(int i=0; i<4; i++) {
        for(int s=0; s<10; s++) {
            RendererNode2 *n = new RendererNode2(i*s,s,s,10);
            n->listOfObj << lstObj.first();
            rMap[i][s] << n;
        }
    }
    LOG( Solver::RMap2Txt(rMap) )
    renderer.SetMap(rMap,4);
    renderer.StartRender();
    Sleep(1000);
}

QTEST_APPLESS_MAIN(TestSolver)

#include "tst_testtest.moc"
