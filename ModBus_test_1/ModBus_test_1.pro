#-------------------------------------------------
#
# Project created by QtCreator 2016-09-22T10:35:52
#
#-------------------------------------------------

QT += core gui
QT += serialbus
QT += serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ModBus_test_1
TEMPLATE = app


SOURCES += main.cpp\
        modbusslave.cpp

HEADERS  += modbusslave.h

FORMS    += modbusslave.ui
