
QT += core gui widgets
QT += concurrent

TARGET = qt-async-demo
TEMPLATE = app

SOURCES +=  main.cpp\
            MainWindow.cpp 

HEADERS  += MainWindow.h

FORMS    += MainWindow.ui

INCLUDEPATH += ../qt-async-lib

CONFIG(debug, debug|release):LIBS += -L$$OUT_PWD/../qt-async-lib/debug -lqt-async-lib
CONFIG(release, debug|release):LIBS += -L$$OUT_PWD/../qt-async-lib/release -lqt-async-lib

#unix:PRE_TARGETDEPS += $$BIN_DIR/libQtnPropertyCore.a $$BIN_DIR/libQtnPropertyWidget.a
#else:PRE_TARGETDEPS += $$BIN_DIR/QtnPropertyCore.lib  $$BIN_DIR/QtnPropertyWidget.lib

#win32 {
#} else:unix {
#    QMAKE_LFLAGS += -Wl,-rpath,\'\$$ORIGIN\'
#}

