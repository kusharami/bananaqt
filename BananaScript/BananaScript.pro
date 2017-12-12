TRANSLATIONS = \
    Translations/en.ts \
    Translations/ru.ts

include(../Banana.pri)

QT += core gui network script

LIBS += -lBananaCore

TARGET = BananaScript
TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += $$_PRO_FILE_PWD_/..

RESOURCES = BananaScript.qrc

HEADERS += \
    AbstractScriptRunner.h \
    Config.h \
    ScriptQFileInfo.h \
    IScriptRunner.h \
    ScriptCommand.h \
    ScriptManager.h \
    ScriptRunner.h \
    ScriptUtils.h \
    ScriptableProjectFile.h \
    ScriptQByteArray.h \
    ScriptQColor.h \
    ScriptQDir.h \
    ScriptQFileDevice.h \
    ScriptQFile.h \
    ScriptQFont.h \
    ScriptQIODevice.h \
    ScriptQRect.h \
    ScriptQPoint.h \
    ScriptQSize.h \
    ScriptQUrl.h \
    ScriptQSaveFile.h \
    ScriptQTemporaryFile.h \
    ScriptQNetworkRequest.h \
    ScriptQNetworkAccessManager.h \
    ScriptQNetworkReply.h


SOURCES += \
    AbstractScriptRunner.cpp \
    Config.cpp \
    ScriptCommand.cpp \
    ScriptManager.cpp \
    ScriptRunner.cpp \
    ScriptUtils.cpp \
    ScriptableProjectFile.cpp \
    ScriptQByteArray.cpp \
    ScriptQColor.cpp \
    ScriptQDir.cpp \
    ScriptQFileDevice.cpp \
    ScriptQFileInfo.cpp \
    ScriptQFile.cpp \
    ScriptQFont.cpp \
    ScriptQIODevice.cpp \
    ScriptQSize.cpp \
    ScriptQUrl.cpp \
    ScriptQPoint.cpp \
    ScriptQRect.cpp \
    ScriptQSaveFile.cpp \
    ScriptQTemporaryFile.cpp \
    ScriptQNetworkRequest.cpp \
    ScriptQNetworkAccessManager.cpp \
    ScriptQNetworkReply.cpp

OTHER_FILES += \
    Tests/Tests.js \
    Tests/TestQPoint.js \
    Tests/TestQSize.js \
    Tests/TestQRect.js \
    Tests/TestQColor.js \
    Tests/TestQByteArray.js \
    Tests/TestQIODevice.js \
    Tests/TestQFileDevice.js \
    Tests/TestQFile.js \
    Tests/TestQSaveFile.js \
    Tests/TestQTemporaryFile.js \
    Tests/TestQFileInfo.js \
    Tests/TestQDir.js \
    Tests/TestQUrl.js \
    Tests/TestQNetworkRequest.js \
    Tests/TestQNetworkAccessManager.js \
    Tests/TestQNetworkReply.js
