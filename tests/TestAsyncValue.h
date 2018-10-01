#ifndef TEST_ASYNC_VALUE_H
#define TEST_ASYNC_VALUE_H

#include <QObject>

class TestAsyncValue: public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE TestAsyncValue() {}

private Q_SLOTS:

    void simple();
    void catchDeadlock();
    void wait();
};

#endif // TEST_ASYNC_VALUE_H