/*
 * Banana Qt Libraries
 *
 * Copyright (c) 2016 Alexandra Cherdantseva
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
		, bind_count(0)
		, load_error(false)
		, symlink(false)
		, is_open(false)
		, signals_connected(false)
		, old_parent(nullptr)
		, connected_data(nullptr)
	{
	}

	AbstractFile::~AbstractFile()
	{
		disconnectData(connected_data);
	}

	const QString &AbstractFile::getFileExtension() const
	{
		return extension;
	}

	Directory *AbstractFile::getTopDirectory() const
	{
		return dynamic_cast<Directory *>(AbstractFileSystemObject::getTopDirectory());
	}

	Directory *AbstractFile::getParentDirectory() const
	{
		return dynamic_cast<Directory *>(parent());
	}

	bool AbstractFile::isOpen() const
	{
		return is_open;
	}

	bool AbstractFile::canClose()
	{
		return (!isBound() && isOpen());
	}

	void AbstractFile::bind()
	{
		bind_count++;
	}

	void AbstractFile::unbind(bool stay_open)
	{
		Q_ASSERT(bind_count > 0);
		bind_count--;
		if (!stay_open && bind_count == 0)
			close();
	}

	bool AbstractFile::isBound() const
	{
		return (bind_count > 0);
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
			QFileInfo info(saved_path);
			if (QDir().mkpath(info.path()))
			{
				unwatchFile();

				if (symlink)
				{
					if (info.isSymLink() || !info.isDir())
					{
						QFile::remove(saved_path);
						Utils::CreateSymLink(symlink_target, saved_path);
					}
				}

				ok = saveInternal();

				if (ok)
				{
					info.refresh();
					if (info.exists()
					&&	0 != QString::compare(info.canonicalFilePath(), canonical_path, Qt::CaseInsensitive))
					{
						doUpdateFilePath(false);
					}

					onSave();
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
			changeFilePath(saved_path);

			bool reused = false;
			createData(&reused);

			if (!reused)
			{
				auto object = dynamic_cast<Object *>(doGetData());

				if (nullptr != object)
					object->assign(nullptr);
			}

			load_error = false;
			is_open = true;

			bool result = save();

			if (!open || !result)
			{
				destroyData();

				is_open = false;
			}

			doFlagsChanged();
			if (is_open)
			{
				opened();
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
			changeFilePath(saved_path);

			bool reused = false;
			createData(&reused);

			load_error = false;
			modified = false;

			bool error;
			if (!reused)
			{
				error = true;
				QFile file(saved_path);
				if (file.open(QIODevice::ReadOnly))
				{
					error = !doLoad(&file);
					file.close();
				}

				load_error = error;
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
				is_open = true;
				opened();
			} else
				destroyData();

			unbind(true);
		}

		return is_open;
	}

	bool AbstractFile::reload()
	{
		if (isOpen())
		{
			bind();
			bool error = true;
			load_error = false;

			if (symlink)
			{
				QFileInfo info(saved_path);
				if (info.isSymLink() || !info.isDir())
				{
					unwatchFile();

					QFile::remove(saved_path);

					if (QDir().mkpath(info.path()))
						Utils::CreateSymLink(symlink_target, saved_path);

					watchFile();
				}
			}

			QFile file(saved_path);
			if (file.open(QIODevice::ReadOnly))
			{
				error = !doLoad(&file);
				file.close();
			}

			load_error = error;

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

	void AbstractFile::close()
	{
		if (isOpen())
			internalClose();
	}

	void AbstractFile::setLoadError(bool value)
	{
		load_error = value;
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

	bool AbstractFile::rename(const QString &new_name)
	{
		if (open())
			return AbstractFileSystemObject::rename(new_name);

		updateFileNameError(new_name);
		return false;
	}

	bool AbstractFile::isWritable() const
	{
		return isWritableFormat(getFileExtension());
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

		load_error = !doLoad(device);

		if (!load_error)
		{
			is_open = true;
			modified = true;
		} else
		{
			if (create)
				destroyData();
		}

		return !load_error;
	}

	QString AbstractFile::getCanonicalFilePath() const
	{
		if (is_open)
			return canonical_path;

		QFileInfo info(saved_path);
		if (info.exists())
			return info.canonicalFilePath();

		return saved_path;
	}

	bool AbstractFile::isSymLink() const
	{
		if (is_open)
			return symlink;

		QFileInfo info(saved_path.isEmpty() ? getFilePath() : saved_path);
		return info.isSymLink();
	}

	QString AbstractFile::getSymLinkTarget() const
	{
		if (is_open && symlink)
			return symlink_target;

		QFileInfo info(saved_path);
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
		connected_data = nullptr;
		is_open = false;
		modified = false;
		emit modifiedFlagChanged(false);
		emit flagsChanged();
	}

	QObject *AbstractFile::doGetData()
	{
		if (!load_error)
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

	void AbstractFile::onSave()
	{
		auto data = dynamic_cast<Object *>(doGetData());
		if (nullptr != data)
			data->setModified(false);
		else
			setModified(false);
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
		QFile file(saved_path);
		if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
		{
			if (doSave(&file))
				ok = true;

			file.close();
		}

		return ok;
	}

	void AbstractFile::opened()
	{
		watchFile();
		emit fileOpened();
	}

	void AbstractFile::disconnectData(QObject *data)
	{
		if (nullptr != data && data != this)
			QObject::disconnect(data, &QObject::destroyed, this, &AbstractFile::onDataDestroyed);
	}

	void AbstractFile::doUpdateFilePath(bool check_oldpath)
	{
		QString new_path(getFilePath());
		if (!saved_path.isEmpty() && QDir::isAbsolutePath(new_path))
		{
			if (check_oldpath && saved_path == new_path)
				return;

			if (!tryChangeFilePath(new_path))
			{
				QFileInfo old_info(saved_path);

				setFileName(old_info.fileName());

				emit updateFilePathError(saved_path, new_path);
				return;
			}
		}

		changeFilePath(new_path);
	}

	void AbstractFile::connectData()
	{
		auto data = doGetData();
		if (nullptr != data && data != this)
		{
			connected_data = data;
			QObject::connect(data, &QObject::destroyed, this, &AbstractFile::onDataDestroyed);
		}
	}

	void AbstractFile::disconnectData()
	{
		disconnectData(doGetData());
		connected_data = nullptr;
	}

	void AbstractFile::updateFilePath(bool check_oldpath)
	{
		unwatchFile();
		doUpdateFilePath(check_oldpath);
		watchFile();
	}

	void AbstractFile::changeFilePath(const QString &new_path)
	{
		saved_path = new_path;

		QFileInfo info(new_path);
		if (info.exists())
			canonical_path = info.canonicalFilePath();
		else
			canonical_path = new_path;

		symlink = info.isSymLink();
		if (symlink)
		{
			symlink_target = info.symLinkTarget();
		}

		emit pathChanged();
	}

	bool AbstractFile::tryChangeFilePath(const QString &new_path)
	{
		if (load_error)
			return true;

		if (!QFile::exists(saved_path))
			return true;

		if (QDir().mkpath(QFileInfo(new_path).path())
		&&	QFile::rename(saved_path, new_path))
		{
			return true;
		}

		return false;
	}

	void AbstractFile::executeUpdateFilePathError(const QString &path, const QString &failed_path)
	{
		emit updateFilePathError(path, failed_path);
	}

	bool AbstractFile::updateFileExtension(const QString &filename, QString *out_ext)
	{
		QString extension;
		if (AbstractFileSystemObject::updateFileExtension(filename, &extension))
		{
			if (extension.isEmpty())
			{
				auto directory = dynamic_cast<Directory *>(getParentDirectory());

				if (nullptr != directory
				&&	metaObject() != Directory::getFileTypeByExtension(filename))
				{
					return false;
				}
			}

			this->extension = extension;
			if (nullptr != out_ext)
				*out_ext = extension;
			return true;
		}

		return false;
	}

	void AbstractFile::doParentChange()
	{
		Object::doParentChange();

		auto new_parent = parent();

		if (old_parent != new_parent)
		{
			if (signals_connected)
			{
				if (nullptr == new_parent)
				{
					QObject::disconnect(this, &QObject::objectNameChanged, this, &AbstractFile::onNameChanged);
					signals_connected = false;
				}
			} else
			{
				if (nullptr != new_parent)
				{
					QObject::connect(this, &QObject::objectNameChanged, this, &AbstractFile::onNameChanged);
					signals_connected = true;
				}
			}
		}

		old_parent = new_parent;

		updateFilePath();
	}

	void AbstractFile::internalClose()
	{
		is_open = false;
		destroyData();
		onDataDestroyed();
		emit fileClosed();
	}

}
