#-------------------------------------------------
#
# Project created by QtCreator 2015-11-10T11:33:44
#
#-------------------------------------------------

QT       += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = timetableDbUpdateService
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    inacsrv.cpp

HEADERS  += mainwindow.h \
    inacsrv.h

FORMS    += mainwindow.ui \
    inacsrv.ui

DISTFILES +=

RESOURCES += \
    resource.qrc
