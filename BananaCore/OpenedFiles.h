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

#include "AbstractObjectGroup.h"
#include "Object.h"

#include <QFileSystemWatcher>

#include <set>
#include <map>

namespace Banana
{
class AbstractFile;
class ProjectGroup;
class OpenedFiles
	: public Object
	, public AbstractObjectGroup
{
	Q_OBJECT

public:
	explicit OpenedFiles(ProjectGroup *owner, bool noWatcher = false);
	virtual ~OpenedFiles();

	inline ProjectGroup *getOwner() const;

	bool fileIsOpened(const QString &filePath);
	QObject *getRegisteredFileData(const QString &filePath);
	void registerFile(const QString &filePath, QObject *data);
	QObject *unregisterFile(
		const QString &filePath, unsigned *refCountPtr = nullptr);
	QObject *deleteFileData(const QString &filePath);

	QObject *updateFilePath(
		const QString &oldFilePath, const QString &newFilePath);
	bool canChangeFilePath(
		const QString &oldFilePath, const QString &newFilePath);

	virtual const QObjectList &getChildren() override;
	virtual void resetChildren() override;

	bool isFileWatched(const Banana::AbstractFile *file) const;
	bool isFileWatched(const QString &filePath) const;

	void watchFile(AbstractFile *file, bool yes);
	void watch(const QString &path, bool yes);

	template <typename CLASS>
	inline void connectFilesChanged(
		CLASS *object, void (CLASS::*onFilesChanged)(const QString &))
	{
		QObject::connect(
			this, &OpenedFiles::filesChanged, object, onFilesChanged);
	}

	template <typename CLASS>
	inline void disconnectFilesChanged(
		CLASS *object, void (CLASS::*onFilesChanged)(const QString &))
	{
		QObject::disconnect(
			this, &OpenedFiles::filesChanged, object, onFilesChanged);
	}

	void clearWatcher();

signals:
	void filesChanged(const QString &path);

private slots:
	void onFileDataParentChanged();
	void onBeforeDestroy(QObject *obj);

private:
	struct Info
	{
		QObject *data;
		unsigned ref_count;
	};

	typedef std::map<QString, Info> FileMap;

	void resetWatcher(bool copy);
	void addPathInternal(const QString &path);
	void removePathInternal(const QString &path);

	FileMap::iterator findFileData(const QString &filePath);
	FileMap file_map;
	QObjectList m_children;
	QFileSystemWatcher *watcher;
	ProjectGroup *owner;
	bool noWatcher;

	friend class OpenedFilesPathGroup;
};

ProjectGroup *OpenedFiles::getOwner() const
{
	return owner;
}
}
