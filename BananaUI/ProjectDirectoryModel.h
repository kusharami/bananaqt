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

#include "AbstractObjectTreeModel.h"

#include <QIcon>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <QFileIconProvider>

#include <map>
#include <functional>

namespace Banana
{

class AbstractProjectDirectory;
class Directory;
class AbstractFile;
class AbstractFileSystemObject;
class BaseFileManager;

class ProjectDirectoryModel
	: public QFileSystemModel
	, public QFileIconProvider
{
	Q_OBJECT

public:
	struct FileTypeInfo
	{
		QIcon icon;
	};

	explicit ProjectDirectoryModel(QObject *parent = nullptr);

	inline AbstractProjectDirectory *getProjectDirectory() const;
	QModelIndex setProjectDirectory(AbstractProjectDirectory *dir);

	void setFileManager(Banana::BaseFileManager *manager);
	BaseFileManager *getFileManager() const;

	virtual int columnCount(const QModelIndex &parent) const override;

	virtual QIcon icon(const QFileInfo &info) const override;

	virtual QVariant data(const QModelIndex &index, int role) const override;

	virtual bool setData(const QModelIndex &index, const QVariant &value,
						 int role = Qt::EditRole) override;

	virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action,
							  int row, int column,
							  const QModelIndex &parent) override;

	virtual Qt::DropActions supportedDragActions() const override;
	virtual Qt::DropActions supportedDropActions() const override;
	virtual Qt::DropAction convertDropAction(const QMimeData *mimeData,
											 Qt::DropAction action) const;

	void setFileTypeInfo(const QString &extension, const FileTypeInfo &info);
	void setGraphicsFileMetaObject(const QMetaObject *metaObject);

	bool canOpenFiles(const QStringList &filePaths) const;
	bool canOpenFile(AbstractFile *file) const;
	bool canOpenFile(const QMetaObject *fileMetaObject) const;
	bool canOpenFile(const QString &filePath) const;
	void openFiles(const QStringList &filePaths) const;

signals:
	void changeFilePathError(const QString &path, const QString &failed_path);
	void changeDirPathError(const QString &path, const QString &failed_path);
	void projectDirectoryChanged();
	void openFile(const QString &filePath) const;
	void locateFile(const QString &filePath) const;

private slots:
	void onProjectDirectoryDestroyed();

private:
	static QString getToolTipForFile(const QFileInfo &info);

	QModelIndex updateActiveProjectDirectory();
	void connectProjectDirectory();
	void disconnectProjectDirectory();

	std::map<QString, FileTypeInfo> fileTypesInfo;

	AbstractProjectDirectory *projectDir;
	BaseFileManager *fileManager;
	const QMetaObject *graphicsFileMetaObject;
};

AbstractProjectDirectory *ProjectDirectoryModel::getProjectDirectory() const
{
	return projectDir;
}

}
