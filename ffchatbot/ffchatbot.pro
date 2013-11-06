#-------------------------------------------------
#
# Project created by QtCreator 2013-10-31T10:10:49
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ffchatbot
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    connectwindow.cpp

HEADERS  += mainwindow.h \
    connectwindow.h

FORMS    += mainwindow.ui \
    connectwindow.ui

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../qxmpp/src/ -lqxmpp
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../qxmpp/src/ -lqxmpp_d0
else:unix: LIBS += -L$$OUT_PWD/../qxmpp/src/ -lqxmpp_d

INCLUDEPATH += $$PWD/../qxmpp/src
DEPENDPATH += $$PWD/../qxmpp/src

CONFIG += c++11
