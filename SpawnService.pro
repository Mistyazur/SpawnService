QT += core
QT -= gui

CONFIG += c++11

TARGET = SpawnService
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    process.cpp

HEADERS += \
    process.h

LIBS += -lUser32 -lAdvapi32 -lUserenv -lWtsapi32

include(QtService/QtService.pri)

win32 {
QMAKE_LFLAGS += /MANIFESTUAC:\"level=\'requireAdministrator\' uiAccess=\'false\'\"
}
