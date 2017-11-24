TRANSLATIONS = \
    Translations/en.ts \
    Translations/ru.ts

include(../Banana.pri)

QT += core gui script

TARGET = BananaCore
TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += $$_PRO_FILE_PWD_/..

SOURCES += \
    UniqueNameScope.cpp \
    PropertyDef.cpp \
    Utils.cpp \
    AbstractFileSystemObject.cpp \
    ObjectGroup.cpp \
    Directory.cpp \
    NamingPolicy.cpp \
    AbstractObjectSelector.cpp \
    ProjectGroup.cpp \
    AbstractFile.cpp \
    AbstractDirectory.cpp \
    Const.cpp \
    VariantMapFile.cpp \
    OpenedFiles.cpp \
    FileRegistrator.cpp \
    Object.cpp \
    Core.cpp \
    AbstractProjectDirectory.cpp \
    AbstractProjectFile.cpp \
    ScriptRunner.cpp \
    ScriptTemplates.cpp \
    MetaObjectChildFilter.cpp \
    ChildFilter.cpp \
    NameChildFilter.cpp \
    AbstractObjectGroup.cpp \
    BinaryFile.cpp \
    UniqueFileNameScope.cpp \
    BaseFileManager.cpp \
    SearchPaths.cpp \
    Config.cpp \
    DirectoryLinker.cpp \
    AbstractNameUnifier.cpp \
    FileNameCollection.cpp \
    NameEnumerator.cpp \
    ScriptManager.cpp \
    AbstractScriptRunner.cpp \
    ScriptCommand.cpp

HEADERS += \
    Const.h \
    UniqueNameScope.h \
    PropertyDef.h \
    Utils.h \
    AbstractFileSystemObject.h \
    ObjectGroup.h \
    Directory.h \
    NamingPolicy.h \
    AbstractObjectSelector.h \
    ProjectGroup.h \
    AbstractFile.h \
    AbstractDirectory.h \
    VariantMapFile.h \
    OpenedFiles.h \
    FileRegistrator.h \
    Object.h \
    Core.h \
    AbstractProjectDirectory.h \
    AbstractProjectFile.h \
    ScriptRunner.h \
    IProjectGroupDelegate.h \
    ScriptTemplates.h \
    IChildFilter.h \
    MetaObjectChildFilter.h \
    ChildFilter.h \
    NameChildFilter.h \
    AbstractObjectGroup.h \
    BinaryFile.h \
    UniqueFileNameScope.h \
    BaseFileManager.h \
    SearchPaths.h \
    Config.h \
    ScriptUtils.h \
    DirectoryLinker.h \
    ContainerTypes.h \
    AbstractNameUnifier.h \
    FileNameCollection.h \
    NameEnumerator.h \
    AbstractNameCollection.h \
    ScriptManager.h \
    IScriptRunner.h \
    AbstractScriptRunner.h \
    IAbortDelegate.h \
    IUndoStack.h \
    IUndoCommand.h \
    ScriptCommand.h

RESOURCES = BananaCore.qrc

DISTFILES +=
