#include "TestAsyncValue.h"
#include <QtTest/QtTest>
#include "values/AsyncValue.h"

void TestAsyncValue::simple()
{
    // init by value
    {
        AsyncValue<int> value(nullptr, AsyncInitByValue(), 8);

        {
            int v = 0;
            auto res = value.access([&v](const int& value){ v = value; } );
            QCOMPARE(v, 8);
            QVERIFY(res);

            res = value.accessError(AsyncNoOp());
            QVERIFY(!res);
        }
    }
}
/*
void TestEnum::enumValue()
{
    COLOR::Enum color = COLOR::RED;
    QVERIFY(color == COLOR::RED);
}

void TestEnum::forEachEnumValue()
{
    unsigned int count = 0;
    COLOR::info().forEachEnumValue([&count](const QtnEnumValueInfo&)->bool {
        ++count;
        return true;
    });
    QVERIFY(count == COLOR::values_count);
}

void TestEnum::find()
{
    QVERIFY(COLOR::info().findByValue(COLOR::RED));
    QVERIFY(!COLOR::info().findByValue(333));
    QVERIFY(COLOR::info().findByDisplayName("Blue"));
    QVERIFY(COLOR::info().findByName("BLUE"));
    QVERIFY(!COLOR::info().findByName("Brown"));
    QVERIFY(!COLOR::info().findByDisplayName("Brown"));
}

void TestEnum::state()
{
    QVERIFY(COLOR::info().findByValue(COLOR::RED)->state() == QtnEnumValueStateNone);
    QVERIFY(COLOR::info().findByValue(COLOR::BLUE)->state() == (QtnEnumValueStateHidden | QtnEnumValueStateObsolete));
}
*/
