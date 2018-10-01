QT += core  concurrent testlib
QT -= gui

TARGET = qt-async-tests

CONFIG   += console
CONFIG   -= app_bundle
CONFIG   += c++14

TEMPLATE = app

HEADERS += \
    TestAsyncValue.h

SOURCES += main.cpp \
    TestAsyncValue.cpp

INCLUDEPATH += ../qt-async-lib

win32 {
    CONFIG(debug, debug|release): ASYNC_LIB_PATH = $$OUT_PWD/../qt-async-lib/debug
    CONFIG(release, debug|release): ASYNC_LIB_PATH = $$OUT_PWD/../qt-async-lib/release
} else:unix {
    ASYNC_LIB_PATH = $$OUT_PWD/../qt-async-lib
}

LIBS += -L$$ASYNC_LIB_PATH -lqt-async-lib

win32:PRE_TARGETDEPS += $$ASYNC_LIB_PATH/qt-async-lib.lib
