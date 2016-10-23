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
    TestsMain.cpp \
    tst_AbstractNameUnifier.cpp \
    tst_FileNameCollection.cpp \
    tst_NameEnumerator.cpp

HEADERS += \
    TestsMain.h \
    tst_AbstractNameUnifier.h \
    tst_FileNameCollection.h \
    tst_NameEnumerator.h
