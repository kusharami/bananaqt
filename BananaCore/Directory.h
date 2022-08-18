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

#include "AbstractDirectory.h"
#include "ObjectGroup.h"
#include "NamingPolicy.h"

#include <QStringList>

#include <memory>
#include <set>
#include <QDir>

namespace Banana
{
class AbstractFile;
class AbstractProjectDirectory;
class Directory
	: public ObjectGroup
	, public AbstractDirectory
	, public FileNamingPolicy
{
	Q_OBJECT

	Q_PROPERTY(QString name READ getFileName SCRIPTABLE true STORED false)
	Q_PROPERTY(QString path READ getFilePath SCRIPTABLE true STORED false)
	Q_PROPERTY(bool searched READ isSearched WRITE setSearched
			SCRIPTABLE true STORED false)
	Q_PROPERTY(int searchOrder READ getSearchOrder WRITE setSearchOrder
			SCRIPTABLE true STORED false)

	Q_PROPERTY(bool userSpecific READ isUserSpecific WRITE setUserSpecific
			SCRIPTABLE true STORED false DESIGNABLE false)

public:
	Q_INVOKABLE QString getAbsoluteFilePathFor(
		const QString &path, bool search = false) const;
	Q_INVOKABLE inline QString getRelativeFilePathFor(
		const QString &path) const;
	Q_INVOKABLE QObjectList findFiles(const QStringList &filters) const;

	enum class Error
	{
		FileNotFound,
		DirectoryNotFound,
		InvalidFilePath,
		CannotReplaceDirectoryWithFile,
		CannotWriteFile,
		CannotCreateDirectory,
		FileCannotBeSavedInSpecificFormat,
		CannotBeUsedAsFileName,
		CannotReplaceOpenedFile,
		CannotReplaceFileLink
	};

	explicit Directory(const QString &name = QString());
	virtual ~Directory() override;

	Directory *getTopDirectory() const;
	Directory *getParentDirectory() const;

	template <typename T>
	T *newChild(const QString &path);

	QObject *initFileSystemObject(QObject *filesys_object, const QString &path);

	struct RegisteredFileType
	{
		const char *extension;
		const QMetaObject *fileMetaObject;
		const QMetaObject *dataMetaObject;
	};

	typedef std::vector<RegisteredFileType> RegisteredFileTypes;

	static const RegisteredFileTypes &getRegisteredFileTypes();
	static void registerFileType(const char *extension,
		const QMetaObject *fileMetaObject, const QMetaObject *dataMetaObject);
	static const QMetaObject *getFileTypeByExtension(
		const QString &filePath, const char **extensionPtr = nullptr);
	static const QMetaObject *getFileTypeByExtensionPtr(const char *extension);
	static std::vector<const char *> getFileTypeExtensions(
		const QMetaObject *metaObject);
	static const RegisteredFileType *findRegisteredFileType(
		const char *extension = nullptr,
		const QMetaObject *fileMetaObject = nullptr,
		const QMetaObject *dataMetaObject = nullptr);

	static QString getFileTypeTitle(
		const QMetaObject *metaObject, bool plural = false);
	static QString getFileFormatName(
		const char *extension, bool plural = false, bool emptyResult = false);
	static QString getFileFormatName(const QString &extension,
		bool plural = false, bool emptyResult = false);
	static QString getFileFormatNameFrom(
		const QString &filePath, bool plural = false);
	static QString getFilterForExtension(const char *extension);

	static QString getFileExtensionFromFilter(
		const QMetaObject *metaObject, const QString &filter);
	static QString getFilterForFileType(const QMetaObject *metaObject);
	static QStringList getPossibleFiltersForFileType(
		const QMetaObject *metaObject);
	static QStringList getAllPossibleFilters();
	static QStringList getFileSaveAsFilters(AbstractFile *file);

	static QString errorFormatStr(Error error);
	static QString filePathErrorStr(Error error, const QString &filepath);

	static void getFilesIn(Directory *dir, std::set<AbstractFile *> &files);
	static void getDirContents(
		Directory *dir, std::set<AbstractFileSystemObject *> &output);

	Q_INVOKABLE virtual bool rename(const QString &new_name) override;
	bool moveTo(Directory *target_dir, const QString &new_name = QString());

	inline bool isSearched() const;
	void setSearched(bool searched);
	inline int getSearchOrder() const;
	void setSearchOrder(int order);

	void setSearchOrderForAncestor(Directory *ancestor, int order);

signals:
	void dirDestroyed();
	void updatePathError(const QString &path, const QString &failed_path);

protected:
	virtual void descendantChanged(
		QObject *descendant, DescendantState state) override;
	virtual void executeUpdateFilePathError(
		const QString &path, const QString &failed_path) override;

	virtual UniqueNameScope *createNameScope(const QMetaObject *meta_object,
		Qt::CaseSensitivity sensitivity) const override;
	virtual void sortChildren(QObjectList &children) override;
	virtual void childEvent(QChildEvent *event) override;
	void findFilesRecursive(QObjectList &result,
		AbstractProjectDirectory *projectDirectory, QDir currentDir,
		const QStringList &filters) const;

private:
	int searchOrder;
	bool searched;

	static RegisteredFileTypes registeredFileTypes;
};

QString Directory::getRelativeFilePathFor(const QString &path) const
{
	return AbstractDirectory::getRelativeFilePathFor(path);
}

template <typename T>
T *Directory::newChild(const QString &path)
{
	auto result = new T;

	if (initFileSystemObject(result, path))
		return result;

	delete result;
	return nullptr;
}

bool Directory::isSearched() const
{
	return searched;
}

int Directory::getSearchOrder() const
{
	return searchOrder;
}

class RootDirectory : public Directory
{
public:
	explicit RootDirectory(const QString &path = QString());
	virtual ~RootDirectory() override;

	void setPath(const QString &path);
	virtual bool rename(const QString &new_name) override;
	virtual QString getFileName() const override;
	virtual QString getFixedName(const QString &source) const override;
};
}

Q_DECLARE_METATYPE(Banana::Directory *)
