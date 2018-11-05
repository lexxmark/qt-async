CONFIG += c++14

QT += core gui widgets network
QT += concurrent

TARGET = qt-async-demo
TEMPLATE = app

SOURCES +=  main.cpp\
            MainWindow.cpp 

HEADERS  += MainWindow.h \
    mypixmap.h

FORMS    += MainWindow.ui

INCLUDEPATH += ../qt-async-lib

win32 {
    CONFIG(debug, debug|release): ASYNC_LIB_PATH = $$OUT_PWD/../qt-async-lib/debug
    CONFIG(release, debug|release): ASYNC_LIB_PATH = $$OUT_PWD/../qt-async-lib/release
} else:unix {
    ASYNC_LIB_PATH = $$OUT_PWD/../qt-async-lib
}

LIBS += -L$$ASYNC_LIB_PATH -lqt-async-lib

win32:PRE_TARGETDEPS += $$ASYNC_LIB_PATH/qt-async-lib.lib
