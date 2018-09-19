
QT += core gui widgets
QT += concurrent

TARGET = qt-async-demo
TEMPLATE = app

SOURCES +=  main.cpp\
            MainWindow.cpp 

HEADERS  += MainWindow.h

FORMS    += MainWindow.ui

INCLUDEPATH += ../qt-async-lib

CONFIG(debug, debug|release): ASYNC_LIB_PATH = $$OUT_PWD/../qt-async-lib/debug
CONFIG(release, debug|release): ASYNC_LIB_PATH = $$OUT_PWD/../qt-async-lib/release

LIBS += -L$$ASYNC_LIB_PATH -lqt-async-lib

unix:PRE_TARGETDEPS += $$ASYNC_LIB_PATH/qt-async-lib.a
else:PRE_TARGETDEPS += $$ASYNC_LIB_PATH/qt-async-lib.lib

#win32 {
#} else:unix {
#    QMAKE_LFLAGS += -Wl,-rpath,\'\$$ORIGIN\'
#}

