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

#include "ProjectDirectoryModel.h"

#include "BananaCore/Utils.h"
#include "BananaCore/AbstractProjectDirectory.h"
#include "BananaCore/BaseFileManager.h"

#include <QFileInfo>
#include <QDir>
#include <QMimeData>

namespace Banana
{
ProjectDirectoryModel::ProjectDirectoryModel(QObject *parent)
	: QFileSystemModel(parent)
	, projectDir(nullptr)
	, fileManager(nullptr)
	, graphicsFileMetaObject(nullptr)
{
	QFileIconProvider::setOptions(QFileIconProvider::DontUseCustomDirectoryIcons);
	setIconProvider(this);
	setReadOnly(false);
	setNameFilterDisables(false);

	setFilter(QDir::Dirs | QDir::Files | QDir::Readable | QDir::Writable |
		QDir::Executable | QDir::Modified | QDir::NoDotAndDotDot);
}

QModelIndex ProjectDirectoryModel::setProjectDirectory(
	AbstractProjectDirectory *dir)
{
	if (projectDir != dir)
	{
		if (nullptr != fileManager)
			fileManager->setProjectDirectory(dir);

		disconnectProjectDirectory();
		projectDir = dir;
		QModelIndex result = updateActiveProjectDirectory();
		connectProjectDirectory();

		emit projectDirectoryChanged();

		return result;
	}

	if (nullptr != projectDir)
		return index(projectDir->getFilePath());

	return QModelIndex();
}

int ProjectDirectoryModel::columnCount(const QModelIndex &) const
{
	return 1;
}

QIcon ProjectDirectoryModel::icon(const QFileInfo &info) const
{
	if (info.isFile())
	{
		QString filename(info.fileName());

		for (auto &data : fileTypesInfo)
		{
			if (filename.endsWith(data.first, Qt::CaseInsensitive))
				return data.second.icon;
		}
	}

	return QFileIconProvider::icon(info);
}

QVariant ProjectDirectoryModel::data(const QModelIndex &index, int role) const
{
	if (index.isValid())
	{
		switch (role)
		{
			case Qt::FontRole:
			{
				auto info = fileInfo(index);

				QFont font;

				info.refresh();

				if (info.isSymLink())
				{
					font.setItalic(true);

					if (!info.exists())
						font.setStrikeOut(true);
				}

				return font;
			}

			case Qt::ToolTipRole:
			{
				auto info = fileInfo(index);
				info.refresh();
				return getToolTipForFile(info);
			}

			default:
				return QFileSystemModel::data(index, role);
		}
	}

	return QVariant();
}

bool ProjectDirectoryModel::setData(
	const QModelIndex &index, const QVariant &value, int role)
{
	if (index.isValid() && role == Qt::EditRole)
	{
		Q_ASSERT(nullptr != projectDir);

		auto filePath = this->filePath(index);

		auto obj = projectDir->findFileSystemObject(filePath, false);

		auto dir = dynamic_cast<Directory *>(obj);

		if (nullptr != dir)
		{
			auto connection = QObject::connect(dir, &Directory::updatePathError,
				this, &ProjectDirectoryModel::changeDirPathError);

			bool result = dir->rename(value.toString());

			QObject::disconnect(connection);

			return result;
		}

		auto file = dynamic_cast<AbstractFile *>(obj);

		if (nullptr != file)
		{
			if (file->isOpen() || file->isBound())
			{
				QString newName(value.toString());

				bool error = false;

				auto connection = QObject::connect(file,
					&AbstractFile::updateFilePathError,
					[this, file, &error](const QString &path,
						const QString &failedPath) mutable {
						QString extension(file->getFileExtension());

						if (extension.isEmpty() ||
							failedPath.endsWith(extension, Qt::CaseInsensitive))
						{
							emit changeFilePathError(path, failedPath);
							error = true;
						}
					});

				bool result = file->rename(newName);

				QObject::disconnect(connection);

				if (result || error)
					return result;
			} else
				delete file;
		}
	}

	if (QFileSystemModel::setData(index, value, role))
	{
		if (role == Qt::EditRole)
			emit dataChanged(index, index);

		return true;
	}

	return false;
}

bool ProjectDirectoryModel::dropMimeData(const QMimeData *data,
	Qt::DropAction action, int, int, const QModelIndex &parent)
{
	if (!parent.isValid() || isReadOnly())
		return false;

	Q_ASSERT(nullptr != fileManager);

	return fileManager->processUrls(
		action, QDir(filePath(parent)), data->urls());
}

Qt::DropActions ProjectDirectoryModel::supportedDragActions() const
{
	if (nullptr == fileManager)
		return Qt::IgnoreAction;

	return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}

Qt::DropActions ProjectDirectoryModel::supportedDropActions() const
{
	return supportedDragActions();
}

Qt::DropAction ProjectDirectoryModel::convertDropAction(
	const QMimeData *, Qt::DropAction action) const
{
	return action;
}

void ProjectDirectoryModel::setFileTypeInfo(
	const QString &extension, const FileTypeInfo &info)
{
	fileTypesInfo[extension] = info;
}

void ProjectDirectoryModel::setGraphicsFileMetaObject(
	const QMetaObject *metaObject)
{
	graphicsFileMetaObject = metaObject;
}

bool ProjectDirectoryModel::canOpenFiles(const QStringList &filePaths) const
{
	for (auto &path : filePaths)
	{
		if (canOpenFile(path))
			return true;
	}

	return false;
}

bool ProjectDirectoryModel::canOpenFile(AbstractFile *file) const
{
	if (nullptr != file)
		return canOpenFile(file->metaObject());

	return false;
}

bool ProjectDirectoryModel::canOpenFile(const QMetaObject *fileMetaObject) const
{
	if (nullptr != projectDir)
		return projectDir->canBeCreated(fileMetaObject);

	return false;
}

bool ProjectDirectoryModel::canOpenFile(const QString &filePath) const
{
	return canOpenFile(Directory::getFileTypeByExtension(filePath));
}

void ProjectDirectoryModel::openFiles(const QStringList &filePaths) const
{
	for (auto &path : filePaths)
	{
		openFile(path);
	}
}

void ProjectDirectoryModel::onProjectDirectoryDestroyed()
{
	projectDir = nullptr;
	updateActiveProjectDirectory();
	emit projectDirectoryChanged();
}

QString ProjectDirectoryModel::getToolTipForFile(const QFileInfo &info)
{
	QString tooltip = QDir::toNativeSeparators(info.filePath());
	QString canonicalFilePath;

	if (info.exists())
		canonicalFilePath = info.canonicalFilePath();
	else
		canonicalFilePath = tooltip;

	if (info.isSymLink())
	{
		if (info.exists())
		{
			tooltip = tr("Links to '%1'")
						  .arg(QDir::toNativeSeparators(info.symLinkTarget()));
		} else
		{
			tooltip = tr("Symbolic link target '%1' does not exist")
						  .arg(QDir::toNativeSeparators(info.symLinkTarget()));
		}
	}

	if (info.isFile())
		return QString("%1<br><img src=\"%2\">")
			.arg(tooltip, canonicalFilePath);

	return tooltip;
}

QModelIndex ProjectDirectoryModel::updateActiveProjectDirectory()
{
	if (nullptr != projectDir)
		return setRootPath(projectDir->getFilePath());

	setRootPath(QString());
	return QModelIndex();
}

void ProjectDirectoryModel::connectProjectDirectory()
{
	if (nullptr != projectDir)
	{
		QObject::connect(projectDir, &QObject::destroyed, this,
			&ProjectDirectoryModel::onProjectDirectoryDestroyed);
	}
}

void ProjectDirectoryModel::disconnectProjectDirectory()
{
	if (nullptr != projectDir)
	{
		QObject::disconnect(projectDir, &QObject::destroyed, this,
			&ProjectDirectoryModel::onProjectDirectoryDestroyed);
	}
}

void ProjectDirectoryModel::setFileManager(BaseFileManager *manager)
{
	if (nullptr != manager)
		manager->setProjectDirectory(projectDir);

	fileManager = manager;
}

BaseFileManager *ProjectDirectoryModel::getFileManager() const
{
	return fileManager;
}
}
