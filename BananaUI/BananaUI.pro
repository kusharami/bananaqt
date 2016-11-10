TRANSLATIONS = \
    Translations/en.ts \
    Translations/ru.ts

include(../Banana.pri)

QT += core gui widgets script

LIBS += -lBananaCore

TARGET = BananaUI
TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += $$_PRO_FILE_PWD_/..
DEPENDPATH += $$_PRO_FILE_PWD_/../BananaCore

INCLUDEPATH += $$_PRO_FILE_PWD_/../QtnProperty
DEPENDPATH += $$_PRO_FILE_PWD_/../QtnProperty/QtnProperty

RESOURCES = BananaUI.qrc

FORMS += \
    AddFilesToIgnoreListDialog.ui \
    DeleteFilesDialog.ui \
    FileSelectDialog.ui \
    ListMessageDialog.ui \
    ScriptRunnerDialog.ui \
    SearchPathsDialog.ui \
    ProjectTreeWidget.ui \
    NewFileDialog.ui

HEADERS += \
    AbstractObjectTreeModel.h \
    AddFilesToIgnoreListDialog.h \
    BaseTreeView.h \
    CheckBoxHeader.h \
    ComboBoxWithToolButton.h \
    Config.h \
    DeleteFilesDialog.h \
    FileSelectDialog.h \
    FileTabBar.h \
    ListMessageDialog.h \
    ObjectPropertyWidget.h \
    PasteFileManager.h \
    ProjectDirectoryFilterModel.h \
    ProjectDirectoryModel.h \
    ProjectPropertyWidget.h \
    ProjectTreeView.h \
    QtnPropertySearchPaths.h \
    ScriptRunnerDialog.h \
    SearchPathsDialog.h \
    Utils.h \
    ProjectTreeWidget.h \
    NewFileDialog.h \
    SelectTreeItemsCommand.h \
    Const.h \
    MainWindow.h

SOURCES += \
    AbstractObjectTreeModel.cpp \
    AddFilesToIgnoreListDialog.cpp \
    BaseTreeView.cpp \
    CheckBoxHeader.cpp \
    ComboBoxWithToolButton.cpp \
    Config.cpp \
    DeleteFilesDialog.cpp \
    FileSelectDialog.cpp \
    FileTabBar.cpp \
    ListMessageDialog.cpp \
    ObjectPropertyWidget.cpp \
    PasteFileManager.cpp \
    ProjectDirectoryFilterModel.cpp \
    ProjectDirectoryModel.cpp \
    ProjectPropertyWidget.cpp \
    ProjectTreeView.cpp \
    QtnPropertySearchPaths.cpp \
    ScriptRunnerDialog.cpp \
    SearchPathsDialog.cpp \
    Utils.cpp \
    ProjectTreeWidget.cpp \
    NewFileDialog.cpp \
    SelectTreeItemsCommand.cpp \
    MainWindow.cpp
