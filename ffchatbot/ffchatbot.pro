#-------------------------------------------------
#
# Project created by QtCreator 2013-10-31T10:10:49
#
#-------------------------------------------------

QT       += core network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ffchatbot
TEMPLATE = app


SOURCES += main.cpp\
#        mainwindow.cpp \
#    connectwindow.cpp \
    xmppclient.cpp \
    prefixmanager.cpp

HEADERS  += xmppclient.h \
	#mainwindow.h \
	#connectwindow.h \
	prefixmanager.h

#FORMS    += mainwindow.ui \
#    connectwindow.ui

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../qxmpp/src/ -lqxmpp
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../qxmpp/src/ -lqxmpp_d0
else:unix:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../qxmpp/src/ -lqxmpp
else:unix:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../qxmpp/src/ -lqxmpp_d

INCLUDEPATH += $$PWD/../qxmpp/src $$PWD/../qxmpp/src/base $$PWD/../qxmpp/src/client
DEPENDPATH += $$PWD/../qxmpp/src $$PWD/../qxmpp/src/base $$PWD/../qxmpp/src/client

CONFIG += c++11
CONFIG += console

#DESTDIR = $$PWD/../bin
