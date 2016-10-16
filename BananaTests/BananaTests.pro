include(../Banana.pri)

QT       += testlib script gui widgets

TARGET = BananaTests
CONFIG   += qt console testcase
CONFIG   -= app_bundle

LIBS += -lBananaCore

TEMPLATE = app

INCLUDEPATH += $$_PRO_FILE_PWD_/..
DEPENDPATH += $$_PRO_FILE_PWD_/../BananaCore

DEFINES += SRCDIR=\\\"$$PWD/\\\"
SOURCES += \    
    TestsMain.cpp

HEADERS += \
    TestsMain.h
