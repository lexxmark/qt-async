QT += core  concurrent testlib
QT -= gui

TARGET = qt-async-tests

CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

HEADERS += \
    TestAsyncValue.h

SOURCES += main.cpp \
    TestAsyncValue.cpp

INCLUDEPATH += ../qt-async-lib

CONFIG(debug, debug|release): ASYNC_LIB_PATH = $$OUT_PWD/../qt-async-lib/debug
CONFIG(release, debug|release): ASYNC_LIB_PATH = $$OUT_PWD/../qt-async-lib/release

LIBS += -L$$ASYNC_LIB_PATH -lqt-async-lib

unix:PRE_TARGETDEPS += $$ASYNC_LIB_PATH/qt-async-lib.a
else:PRE_TARGETDEPS += $$ASYNC_LIB_PATH/qt-async-lib.lib
