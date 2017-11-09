/*******************************************************************************
Banana Qt Libraries

Copyright (c) 2016-2017 Alexandra Cherdantseva

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*******************************************************************************/

#pragma once

#include "VariantMapFile.h"
#include "NamingPolicy.h"

#include "PropertyDef.h"

#include <set>
#include <QJsonObject>

namespace Banana
{
class ScriptManager;
class SearchPaths;
class Directory;
class OpenedFiles;
class AbstractProjectDirectory;

class AbstractProjectFile : public VariantMapFile
{
	Q_OBJECT

	Q_PROPERTY(Banana::SearchPaths *mSearchPaths READ getSearchPaths RESET
			resetSearchPaths DESIGNABLE true STORED false)
	Q_PROPERTY(Banana::ScriptManager *mScriptManager READ getScriptManager RESET
			resetScriptManager DESIGNABLE true STORED false)

	struct FileInfo;

	enum class FileObjType
	{
		None,
		Directory,
		DirectoryLink,
		File,
		FileLink
	};

	OpenedFiles *openedFiles;
	SearchPaths *searchPaths;
	ScriptManager *scriptManager;
	QJsonObject userPaths;

protected:
	static const QString SEARCH_PATHS_KEY;
	static const QString IGNORED_FILES_KEY;
	static const QString FILES_KEY;
	static const QString TYPE_KEY;
	static const QString PATH_KEY;
	static const QString TARGET_KEY;
	static const QString TYPE_FILE;
	static const QString TYPE_DIR;
	static const QString TYPE_DIR_LINK;
	static const QString TYPE_FILE_LINK;
	static const QString USER_SPECIFIC_KEY;
	static const QString USER_PATHS_KEY;
	static const QString SCRIPTS_KEY;
	static const QString CAPTION_KEY;
	static const QString OBJECT_TYPE_KEY;

public:
	CUSTOM_PROPERTY(bool, HideIgnoredFiles)
	CUSTOM_REF_PROPERTY_EX(QString, IgnoredFilesPattern, STORED false)

	explicit AbstractProjectFile(const QString &name, const QString &extension);
	virtual ~AbstractProjectFile();

	virtual bool isWatched() const override;

	virtual void unwatchFile() override;
	virtual void watchFile() override;

	QStringList getIgnoredFilesList() const;
	void setIgnoredFilesList(const QStringList &value);

	SearchPaths *getSearchPaths();
	Q_INVOKABLE void resetSearchPaths();

	ScriptManager *getScriptManager();
	Q_INVOKABLE void resetScriptManager();

	QString getAbsoluteFilePathFor(const QString &filepath) const;

	QString getUserSettingsPath() const;

	void setUserSpecificPathFor(AbstractFileSystemObject *file, bool user);

signals:
	void changedHideIgnoredFiles();
	void changedIgnoredFilesPattern();

protected:
	void loadUserSettings();
	void saveUserSettings();
	bool fetchUserSpecificPath(
		bool dir, const QString &relativePath, QString &out);
	virtual void saveData(QVariantMap &output) override;
	virtual bool loadData(const QVariantMap &input) override;

	virtual void doUpdateFilePath(bool checkOldPath) override;

private:
	static FileObjType getFileObjTypeFromString(const QString &str);

	bool finalizeFileInfo(
		FileObjType type, FileInfo &info, const QVariantMap &map);
	void watch(bool yes);
	void saveProjectDirectory(
		Directory *root_dir, Directory *dir, QVariantList &output) const;
	bool loadFileEntries(
		AbstractProjectDirectory *projectDir, const QVariantMap &input);
	void saveFileEntries(QVariantMap &output);

	bool loadIgnoredFileEntries(const QVariantMap &input);
	void saveIgnoredFileEntries(QVariantMap &output);

	bool loadSearchPathEntries(const QVariantMap &input);
	void saveSearchPathEntries(QVariantMap &output);

	bool loadScriptEntries(const QVariantMap &input);
	void saveScriptEntries(QVariantMap &output);
};
}

Q_DECLARE_METATYPE(Banana::AbstractProjectFile *)
