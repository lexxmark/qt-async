CONFIG += staticlib
CONFIG += c++14

TARGET = qt-async-lib
TEMPLATE = lib
VERSION = 1.0.0

QT       += widgets concurrent
QT       -= gui

SOURCES += \
    values/AsyncValueBase.cpp \
    widgets/AsyncWidgetProxy.cpp \
    widgets/AsyncWidgetError.cpp \
    widgets/AsyncWidgetProgress.cpp

HEADERS += \
    values/AsyncValueBase.h \
    values/AsyncValueTemplate.h \
    values/AsyncError.h \
    values/AsyncProgress.h \
    values/AsyncValue.h \
    values/AsyncValueObtain.h \
    widgets/AsyncWidgetProxy.h \
    widgets/AsyncWidgetBase.h \
    widgets/AsyncWidget.h \
    widgets/AsyncWidgetError.h \
    widgets/AsyncWidgetProgress.h
	
DEFINES += QT_DEPRECATED_WARNINGS
