#include "TestAsyncValue.h"
#include <QtTest/QtTest>
#include <thread>
#include "values/AsyncValue.h"
#include "values/AsyncValueRun.h"

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
    using namespace std::chrono_literals;

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

    asyncValueRun(&pool, value, [&](AsyncProgress&, AsyncValue<int>& value){
        sem.acquire(15);
        isRunning.notify_all();
        std::this_thread::sleep_for(1s);
       value.emplaceValue(42);
    }, "", ASYNC_CAN_REQUEST_STOP::NO);

    for (auto f : clients)
    {
        QCOMPARE(f.result(), 42);
    }
}
