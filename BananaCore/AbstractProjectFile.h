/*******************************************************************************
Banana Qt Libraries

Copyright (c) 2016 Alexandra Cherdantseva

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

namespace Banana
{

class SearchPaths;
class Directory;
class OpenedFiles;
class AbstractProjectFile : public VariantMapFile
{
	Q_OBJECT

	Q_PROPERTY(
		Banana::SearchPaths * mSearchPaths
		READ getSearchPaths
		RESET resetSearchPaths
		DESIGNABLE true
		STORED false)

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

	QString getAbsoluteFilePathFor(const QString &filepath) const;

signals:
	void changedHideIgnoredFiles();
	void changedIgnoredFilesPattern();

protected:
	virtual void saveData(QVariantMap &output) override;
	virtual bool loadData(const QVariantMap &input) override;

	virtual void doUpdateFilePath(bool check_oldpath) override;

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

private:
	enum class FileObjType
	{
		None,
		Directory,
		DirectoryLink,
		File,
		FileLink
	};

	static FileObjType getFileObjTypeFromString(const QString &str);

	void watch(bool yes);
	void saveProjectDirectory(
		Directory *root_dir,
		Directory *dir,
		QVariantList &output) const;

	OpenedFiles *openedFiles;
	SearchPaths *searchPaths;
};

}

Q_DECLARE_METATYPE(Banana::AbstractProjectFile *)
