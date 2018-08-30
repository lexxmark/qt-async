CONFIG += staticlib
CONFIG += c++14

TARGET = qt-async-lib
TEMPLATE = lib
VERSION = 1.0.0

QT       += widgets concurrent
QT       -= gui

SOURCES += AsyncValue.cpp \
            AsyncWidget.cpp

HEADERS += AsyncValue.h \
            AsyncWidget.h
	
DEFINES += QT_DEPRECATED_WARNINGS
