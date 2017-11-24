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

#include "AbstractFile.h"

#include "Directory.h"
#include "Utils.h"
#include "ProjectGroup.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>

namespace Banana
{
AbstractFile::AbstractFile(const QString &extension)
	: AbstractFileSystemObject(this)
	, extension(extension)
	, bindCount(0)
	, loadError(false)
	, symLink(false)
	, userSpecific(false)
	, opened(false)
	, signalsConnected(false)
	, oldParent(nullptr)
	, connectedData(nullptr)
{
	(void) QT_TRANSLATE_NOOP("ClassName", "Banana::AbstractFile");
}

AbstractFile::~AbstractFile()
{
	disconnectData(connectedData);
}

const QString &AbstractFile::getFileExtension() const
{
	return extension;
}

Directory *AbstractFile::getTopDirectory() const
{
	return dynamic_cast<Directory *>(
		AbstractFileSystemObject::getTopDirectory());
}

Directory *AbstractFile::getParentDirectory() const
{
	return dynamic_cast<Directory *>(parent());
}

bool AbstractFile::canClose()
{
	return (!isBound() && isOpen());
}

void AbstractFile::bind()
{
	bindCount++;
}

void AbstractFile::unbind(bool stayOpen)
{
	Q_ASSERT(bindCount > 0);
	bindCount--;
	if (!stayOpen && bindCount == 0)
	{
		close();
	}
}

QObject *AbstractFile::getData(bool open)
{
	if (open)
		this->open();

	return doGetData();
}

bool AbstractFile::save()
{
	bool ok = false;
	if (isOpen())
	{
		QFileInfo fileInfo(savedPath);
		if (QDir().mkpath(fileInfo.path()))
		{
			unwatchFile();

			recreateSymLinkIfNeeded(false);

			ok = saveInternal();

			if (ok)
			{
				fileInfo.refresh();
				if (fileInfo.exists() &&
					QString::compare(fileInfo.canonicalFilePath(),
						canonicalPath, Qt::CaseInsensitive) != 0)
				{
					doUpdateFilePath(false);
				}

				auto data = dynamic_cast<Object *>(doGetData());
				if (nullptr != data)
					data->setModified(false);
				setModified(false);
			}

			watchFile();
		}
	}

	return ok;
}

bool AbstractFile::create(bool open)
{
	if (!isOpen())
	{
		changeFilePath(savedPath);

		bool reused = false;
		createData(&reused);

		if (!reused)
		{
			auto object = dynamic_cast<Object *>(doGetData());

			if (nullptr != object)
				object->assign(nullptr);
		}

		loadError = false;
		opened = true;

		bool result = save();

		if (!open || !result)
		{
			destroyData();

			opened = false;
		}

		doFlagsChanged();
		if (opened)
		{
			onOpen();
		}

		return result;
	}

	return false;
}

bool AbstractFile::open()
{
	if (!isOpen())
	{
		bind();
		changeFilePath(savedPath);

		bool reused = false;
		createData(&reused);

		loadError = false;
		modified = false;

		bool error;
		if (!reused)
		{
			error = true;
			QFile file(savedPath);
			if (file.open(QIODevice::ReadOnly))
			{
				error = !doLoad(&file);
				file.close();
			}

			loadError = error;
			modified = false;
		} else
		{
			error = false;
			auto object = dynamic_cast<Object *>(doGetData());
			if (nullptr != object)
				modified = object->isModified();
		}

		doFlagsChanged();

		if (!error)
		{
			opened = true;
			onOpen();
		} else
			destroyData();

		unbind(true);
	}

	return opened;
}

bool AbstractFile::reload()
{
	if (isOpen())
	{
		bind();
		bool error = true;
		loadError = false;

		recreateSymLinkIfNeeded(true);

		QFile file(savedPath);
		if (file.open(QIODevice::ReadOnly))
		{
			error = !doLoad(&file);
			file.close();
		}

		loadError = error;

		if (error && !isBound())
		{
			modified = false;
			internalClose();
			unbind(true);
			return false;
		} else
		{
			modified = error;
			auto data = dynamic_cast<Object *>(doGetData());
			if (nullptr != data)
				data->setModified(error);

			doFlagsChanged();
			emit fileReloaded();
			unbind(true);
			return true;
		}
	}

	return open();
}

bool AbstractFile::close(bool check)
{
	if (not isOpen())
		return true;

	if (not check || canClose())
	{
		internalClose();
		return true;
	}

	return false;
}

void AbstractFile::setLoadError(bool value)
{
	loadError = value;
}

QString AbstractFile::getFilePathShort(Directory *topDirectory) const
{
	return getFilePath(getSearchedDirectory(topDirectory));
}

Directory *AbstractFile::getSearchedDirectory(Directory *topDirectory) const
{
	auto dir = dynamic_cast<Directory *>(parent());
	while (nullptr != dir)
	{
		auto dir_parent = dir->parent();
		if (dir->isSearched())
			return dir;

		dir = dynamic_cast<Directory *>(dir_parent);
	}

	if (topDirectory == nullptr)
		return getTopDirectory();

	return topDirectory;
}

void AbstractFile::tryClose()
{
	if (not isBound())
		close();
}

bool AbstractFile::rename(const QString &newName)
{
	if (open())
		return AbstractFileSystemObject::rename(newName);

	updateFileNameError(newName);
	return false;
}

bool AbstractFile::isWritable() const
{
	return isWritableFormat(getFileExtension());
}

bool AbstractFile::isUserSpecific() const
{
	return userSpecific;
}

void AbstractFile::setUserSpecific(bool yes)
{
	userSpecific = yes;
}

bool AbstractFile::saveTo(QIODevice *device)
{
	QString filepath(getCanonicalFilePath());
	bool exists = QFile::exists(filepath);
	if (isOpen())
	{
		if (!exists || isModified())
			return doSave(device);
	}

	if (exists)
	{
		QByteArray binary;
		if (Utils::LoadBinaryFromFile(binary, filepath))
		{
			return (binary.size() == device->write(binary));
		}
	}

	return false;
}

bool AbstractFile::loadFrom(QIODevice *device)
{
	bool create = !isOpen();
	if (create)
		createData();

	loadError = !doLoad(device);

	if (!loadError)
	{
		opened = true;
		modified = true;
	} else
	{
		if (create)
			destroyData();
	}

	return !loadError;
}

QString AbstractFile::getCanonicalFilePath() const
{
	if (opened)
		return canonicalPath;

	QFileInfo info(savedPath);
	if (info.exists())
		return info.canonicalFilePath();

	return savedPath;
}

bool AbstractFile::isSymLink() const
{
	if (opened)
		return symLink;

	QFileInfo info(savedPath.isEmpty() ? getFilePath() : savedPath);
	return info.isSymLink();
}

QString AbstractFile::getSymLinkTarget() const
{
	if (opened && symLink)
		return symLinkTarget;

	QFileInfo info(savedPath);
	return info.symLinkTarget();
}

bool AbstractFile::isWatched() const
{
	return false;
}

void AbstractFile::onNameChanged()
{
	updateFilePath();
}

void AbstractFile::onDataDestroyed()
{
	connectedData = nullptr;
	opened = false;
	modified = false;
	emit modifiedFlagChanged(false);
	emit flagsChanged();
}

QObject *AbstractFile::doGetData()
{
	if (!loadError)
		return this;

	return nullptr;
}

void AbstractFile::createData(bool *)
{
	connectData();
}

void AbstractFile::destroyData()
{
	disconnectData();
}

void AbstractFile::doFlagsChanged()
{
	if (!modified)
	{
		modifyObject(doGetData(), false, false, true);
	}

	Object::doFlagsChanged();
	emit flagsChanged();
}

bool AbstractFile::saveInternal()
{
	bool ok = false;
	QFile file(savedPath);
	if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
	{
		if (doSave(&file))
			ok = true;

		file.close();
	}

	return ok;
}

void AbstractFile::onOpen()
{
	watchFile();
	emit fileOpened();
}

void AbstractFile::recreateSymLinkIfNeeded(bool unwatched)
{
	if (symLink)
	{
		QFileInfo fileInfo(savedPath);
		if (fileInfo.isSymLink() || fileInfo.isFile())
		{
			if (unwatched)
				unwatchFile();

			if (Utils::DeleteFileOrLink(fileInfo) &&
				QDir().mkpath(fileInfo.path()))
				Utils::CreateSymLink(symLinkTarget, savedPath);

			if (unwatched)
				watchFile();
		}
	}
}

void AbstractFile::disconnectData(QObject *data)
{
	if (nullptr != data && data != this)
	{
		QObject::disconnect(
			data, &QObject::destroyed, this, &AbstractFile::onDataDestroyed);
	}
}

void AbstractFile::doUpdateFilePath(bool checkOldPath)
{
	QString newPath(getFilePath());
	if (!savedPath.isEmpty() && QDir::isAbsolutePath(newPath))
	{
		if (checkOldPath && savedPath == newPath)
			return;

		if (!tryChangeFilePath(newPath))
		{
			QFileInfo oldInfo(savedPath);

			setFileName(oldInfo.fileName());

			emit updateFilePathError(savedPath, newPath);
			return;
		}
	}

	changeFilePath(newPath);
}

void AbstractFile::connectData()
{
	auto data = doGetData();
	if (nullptr != data && data != this)
	{
		connectedData = data;
		QObject::connect(
			data, &QObject::destroyed, this, &AbstractFile::onDataDestroyed);
	}
}

void AbstractFile::disconnectData()
{
	disconnectData(doGetData());
	connectedData = nullptr;
}

void AbstractFile::updateFilePath(bool checkOldPath)
{
	unwatchFile();
	doUpdateFilePath(checkOldPath);
	watchFile();
}

void AbstractFile::changeFilePath(const QString &newPath)
{
	savedPath = newPath;

	QFileInfo info(newPath);
	if (info.exists())
		canonicalPath = info.canonicalFilePath();
	else
		canonicalPath = newPath;

	symLink = info.isSymLink();
	if (symLink)
	{
		symLinkTarget = info.symLinkTarget();
	}

	emit pathChanged();
}

bool AbstractFile::tryChangeFilePath(const QString &newPath)
{
	if (loadError)
		return true;

	if (!QFile::exists(savedPath))
		return true;

	if (QDir().mkpath(QFileInfo(newPath).path()) &&
		QFile::rename(savedPath, newPath))
	{
		return true;
	}

	return false;
}

void AbstractFile::executeUpdateFilePathError(
	const QString &path, const QString &failedPath)
{
	emit updateFilePathError(path, failedPath);
}

bool AbstractFile::updateFileExtension(
	const QString &fileName, QString *outExtension)
{
	QString extension;
	if (AbstractFileSystemObject::updateFileExtension(fileName, &extension))
	{
		if (extension.isEmpty())
		{
			auto directory = dynamic_cast<Directory *>(getParentDirectory());

			if (nullptr != directory &&
				metaObject() != Directory::getFileTypeByExtension(fileName))
			{
				return false;
			}
		}

		this->extension = extension;
		if (nullptr != outExtension)
			*outExtension = extension;
		return true;
	}

	return false;
}

void AbstractFile::doParentChange()
{
	Object::doParentChange();

	auto newParent = parent();

	if (oldParent != newParent)
	{
		if (signalsConnected)
		{
			if (nullptr == newParent)
			{
				QObject::disconnect(this, &QObject::objectNameChanged, this,
					&AbstractFile::onNameChanged);
				signalsConnected = false;
			}
		} else
		{
			if (nullptr != newParent)
			{
				QObject::connect(this, &QObject::objectNameChanged, this,
					&AbstractFile::onNameChanged);
				signalsConnected = true;
			}
		}
	}

	oldParent = newParent;

	updateFilePath();
}

void AbstractFile::internalClose()
{
	opened = false;
	emit fileClosed();
	destroyData();
	onDataDestroyed();
}
}
