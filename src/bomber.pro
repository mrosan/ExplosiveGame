#-------------------------------------------------
#
# Project created by QtCreator 2017-03-08T19:52:25
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = bomber
TEMPLATE = app


SOURCES += main.cpp\
        gameview.cpp \
    gamemodel.cpp

HEADERS  += gameview.h \
    gamemodel.h

RESOURCES += \
    images.qrc

CONFIG += C++11
