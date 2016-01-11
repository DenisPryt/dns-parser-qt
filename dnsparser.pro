#-------------------------------------------------
#
# Project created by QtCreator 2015-10-21T15:53:20
#
#-------------------------------------------------

QT       += core network
QT       -= gui

CONFIG   += c++11 console
CONFIG   -= app_bundle

TARGET    = dnsparser

SOURCES += main.cpp\
    dnspacket.cpp \
    resourcerecord.cpp \
    datatransformer.cpp

HEADERS  += \
    dnspacket.h \
    resourcerecord.h \
    datatransformer.h

FORMS    +=
