QT += core
QT -= gui

CONFIG += c++11

TARGET = SpawnService
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

HEADERS +=

LIBS += -lUser32 -lAdvapi32

include(QtService/QtService.pri)
