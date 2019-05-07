#include "TestAsyncValue.h"
#include <QtTest/QtTest>
#include "values/AsyncValue.h"
#include "values/AsyncValueRunThread.h"
#include "values/AsyncValueRunThreadPool.h"
#include "values/AsyncValueRunNetwork.h"
#include "values/AsyncValueRunable.h"

void TestAsyncValue::simple()
{
    // init by value
    {
        AsyncValue<int> value(AsyncInitByValue(), 8);

        {
            auto res = value.accessValue([](const int& value){
                QCOMPARE(value, 8);
            });
            QVERIFY(res);

            res = value.accessError(AsyncNoOp());
            QVERIFY(!res);

            res = value.accessProgress(AsyncNoOp());
            QVERIFY(!res);
        }
    }

    // init by error
    {
        AsyncValue<int> value(AsyncInitByError(), "no value");

        {
            auto res = value.accessError([](const AsyncError& error){
                QCOMPARE(error.text(), QString("no value"));
            });
            QVERIFY(res);

            res = value.accessValue(AsyncNoOp());
            QVERIFY(!res);

            res = value.accessProgress(AsyncNoOp());
            QVERIFY(!res);
        }
    }
}

void TestAsyncValue::runInThread()
{
    AsyncValue<int> value(AsyncInitByValue(), 8);

    asyncValueRunThread(value, [&](AsyncProgress&, AsyncValue<int>& value) {
        QThread::sleep(1);
       value.emplaceValue(42);
    }, "", ASYNC_CAN_REQUEST_STOP::NO);

    value.wait([](int val){
        QCOMPARE(val, 42);
    }, AsyncNoOp());
}

void TestAsyncValue::runInThreadPool()
{
    AsyncValue<int> value(AsyncInitByValue(), 8);

    asyncValueRunThreadPool(value, [&](AsyncProgress&, AsyncValue<int>& value) {
        QThread::sleep(1);
       value.emplaceValue(42);
    }, "", ASYNC_CAN_REQUEST_STOP::NO);

    value.wait([](int val){
        QCOMPARE(val, 42);
    }, AsyncNoOp());
}

void TestAsyncValue::catchDeadlock()
{
    AsyncValue<int> value(AsyncInitByValue(), 8);

    QObject::connect(&value, &AsyncValue<int>::stateChanged, [&value](ASYNC_VALUE_STATE){
        value.emplaceValue(3);
    });

    bool deadlock = false;

    try
    {
        value.emplaceValue(7);
    }
    catch (const std::logic_error&)
    {
        deadlock = true;
    }

    QVERIFY(deadlock);
}

void TestAsyncValue::wait()
{
    AsyncValue<int> value(AsyncInitByValue(), 8);

    QMutex lock;
    QWaitCondition isRunning;
    QSemaphore sem(15);
    sem.acquire(15);

    QThreadPool pool;
    pool.setMaxThreadCount(20);

    std::vector<QFuture<int>> clients;
    for (int i = 0; i < 15; ++i)
    {
        QFuture<int> future = QtConcurrent::run(&pool, [&](){
            {
                QMutexLocker locker(&lock);
                sem.release(1);
                isRunning.wait(&lock);
            }

            int res = 0;

            value.wait([&](int val){
                res = val;
            }, AsyncNoOp());

            return res;
        });

        clients.push_back(future);
    }

    asyncValueRunThreadPool(&pool, value, [&](AsyncProgress&, AsyncValue<int>& value){
        sem.acquire(15);
        isRunning.notify_all();
        QThread::sleep(1);
       value.emplaceValue(42);
    }, "", ASYNC_CAN_REQUEST_STOP::NO);

    for (auto f : clients)
    {
        QCOMPARE(f.result(), 42);
    }
}

void TestAsyncValue::run()
{
    AsyncValueRunableFn<int> value(AsyncInitByValue(), 8);

    int rerunTotal = 0;

    value.deferFn = [&value](const AsyncValueRunableFn<int>::RunFnType& fn) {
        asyncValueRunThreadPool(value, fn, "", ASYNC_CAN_REQUEST_STOP::YES);
    };
    value.runFn = [&](AsyncProgressRerun&, AsyncValueRunableFn<int>& value) {
        QThread::sleep(1);
        value.emplaceValue(42);
        ++rerunTotal;
    };

    value.run();
    value.run();
    value.run();
    value.run();
    value.run();

    value.wait();

    // at least two runs should happen
    QVERIFY(rerunTotal >= 2);
}

void TestAsyncValue::network()
{
    AsyncValue<int> value(AsyncInitByValue(), 8);
    QNetworkAccessManager network;
    QEventLoop loop;

    asyncValueRunNetwork(&network, QNetworkRequest(QUrl("http://doc.qt.io/qt-5/images/qml-uses-visual-transforms.png")), value, [&loop](const QNetworkReply& reply, AsyncValue<int>& value){

        // process error
        if (reply.error() != QNetworkReply::NoError)
            value.emplaceError(QString("Network error code: %1").arg(reply.error()));
        else
            value.emplaceValue(reply.bytesAvailable());

        loop.quit();
    }, "", ASYNC_CAN_REQUEST_STOP::NO);

    loop.exec();

    auto success = value.accessValue([](int value){
        QVERIFY(value > 0);
    });

    QVERIFY(success);
}
