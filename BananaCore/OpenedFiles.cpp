/*
 * MIT License
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

#include "OpenedFiles.h"

#include "AbstractFile.h"
#include "DirectoryLinker.h"

#include <QFileInfo>
#include <QDir>
#include <QChildEvent>

namespace Banana
{

	OpenedFiles::OpenedFiles(ProjectGroup *owner)
		: watcher(nullptr)
		, owner(owner)
	{
		resetWatcher(false);
	}

	OpenedFiles::~OpenedFiles()
	{
		delete watcher;
	}

	bool OpenedFiles::fileIsOpened(const QString &filePath)
	{
		return nullptr != getRegisteredFileData(filePath);
	}

	QObject *OpenedFiles::getRegisteredFileData(const QString &filePath)
	{
		auto it = findFileData(filePath);

		if (file_map.end() != it)
			return it->second.data;

		return nullptr;
	}

	void OpenedFiles::registerFile(const QString &filePath, QObject *data)
	{
		auto it = findFileData(filePath);

		if (file_map.end() == it)
		{
			it = file_map.insert(std::pair<QString, Info>(filePath, { data, 0 })).first;
			addPathInternal(filePath);
		}

		Q_ASSERT(it->second.data == data);

		if (it->second.ref_count == 0)
			data->setParent(this);

		it->second.ref_count++;
	}

	QObject *OpenedFiles::unregisterFile(const QString &filePath, unsigned *ref_count_ptr)
	{
		auto it = findFileData(filePath);

		QObject *result = nullptr;

		if (nullptr != ref_count_ptr)
			*ref_count_ptr = 0;

		if (file_map.end() != it)
		{
			result = it->second.data;
			if (0 == --it->second.ref_count)
			{
				removePathInternal(filePath);
				file_map.erase(it);
			} else if (nullptr != ref_count_ptr)
				*ref_count_ptr = it->second.ref_count;
		}

		return result;
	}

	QObject *OpenedFiles::deleteFileData(const QString &filePath)
	{
		unsigned ref_count = 0;

		auto data = unregisterFile(filePath, &ref_count);
		if (ref_count == 0)
			delete data;

		return data;
	}

	QObject *OpenedFiles::updateFilePath(const QString &oldFilePath, const QString &newFilePath)
	{
		auto it = findFileData(oldFilePath);

		if (file_map.end() == it)
			return nullptr;

		auto data = it->second.data;

		if (0 == QString::compare(oldFilePath, newFilePath, Qt::CaseInsensitive))
			return data;

		Q_ASSERT(nullptr != data);

		unsigned ref_count = 0;

		auto unregistered = unregisterFile(oldFilePath, &ref_count);
		Q_ASSERT(data == unregistered);
		Q_UNUSED(unregistered);

		if (ref_count == 0)
		{
			it = findFileData(newFilePath);

			if (file_map.end() == it)
			{
				registerFile(newFilePath, data);
				return data;
			}
		} else
		{
			auto new_data = data->metaObject()->newInstance();

			new_data->setObjectName(data->objectName());

			registerFile(newFilePath, new_data);

			auto object = dynamic_cast<Object *>(new_data);
			if (nullptr != object)
			{
				auto src = dynamic_cast<Object *>(data);
				bool old_modified = src->isModified();

				object->assign(src);

				object->setModified(old_modified);
			} else
			{
				QVariantMap vmap;
				Object::saveContents(object, vmap);
				Object::loadContents(vmap, new_data, true);
			}

			return new_data;
		}

		return nullptr;
	}

	bool OpenedFiles::canChangeFilePath(const QString &oldFilePath, const QString &newFilePath)
	{
		if (0 == QString::compare(oldFilePath, newFilePath, Qt::CaseInsensitive))
			return true;

		auto old_data = getRegisteredFileData(oldFilePath);
		auto found_data = getRegisteredFileData(newFilePath);

		if (found_data == old_data)
			return true;

		if (nullptr == found_data)
			return true;

		return false;
	}

	const QObjectList &OpenedFiles::getChildren()
	{
		if (m_children.empty())
		{
			for (auto child : children())
			{
				auto object = dynamic_cast<Object *>(child);
				if (nullptr == object || !object->isDeleted())
					m_children.push_back(child);
			}
		}

		return m_children;
	}

	void OpenedFiles::resetChildren()
	{
		m_children.clear();
	}

	bool OpenedFiles::isFileWatched(const AbstractFile *file) const
	{
		Q_ASSERT(nullptr != file);

		if (file->isSymLink())
			return isFileWatched(file->getFilePath());

		return isFileWatched(file->getCanonicalFilePath());
	}

	bool OpenedFiles::isFileWatched(const QString &filePath) const
	{
		QFileInfo info(filePath);

		if (info.isFile() || (info.isSymLink() && !info.isDir()))
		{
			for (auto &it_path : watcher->files())
			{
				if (0 == QString::compare(it_path, filePath, Qt::CaseInsensitive))
					return true;
			}
		}

		return false;
	}

	void OpenedFiles::watchFile(AbstractFile *file, bool yes)
	{
		if (file->isSymLink())
		{
			watch(file->getFilePath(), yes);
		} else
		if (yes)
		{
			addPathInternal(QFileInfo(file->getFilePath()).canonicalPath());
		}

		watch(file->getCanonicalFilePath(), yes);
	}

	void OpenedFiles::watch(const QString &path, bool yes)
	{
		if (!path.isEmpty())
		{
			if (yes)
			{
				addPathInternal(path);
			} else
			{
				removePathInternal(path);
			}
		}
	}

	void OpenedFiles::clearWatcher()
	{
		resetWatcher(false);
	}

	void OpenedFiles::onFileDataDestroyed()
	{
		resetChildren();
	}

	void OpenedFiles::childEvent(QChildEvent *event)
	{
		Object::childEvent(event);

		if (event->added() || event->removed())
		{
			if (event->added())
			{
				QObject::connect(event->child(), &QObject::destroyed,
								 this, &OpenedFiles::onFileDataDestroyed);
			} else
			if (event->removed())
			{
				QObject::disconnect(event->child(), &QObject::destroyed,
								 this, &OpenedFiles::onFileDataDestroyed);
			}
			resetChildren();
		}
	}

	void OpenedFiles::resetWatcher(bool copy)
	{
		QStringList files;

		if (copy)
			files = watcher->files();
		delete watcher;

		watcher = new QFileSystemWatcher;

		if (copy)
		{
			for (auto &path : files)
			{
				addPathInternal(path);
			}
		}

		QObject::connect(watcher, &QFileSystemWatcher::fileChanged,
						 this, &OpenedFiles::filesChanged);
		QObject::connect(watcher, &QFileSystemWatcher::directoryChanged,
						 this, &OpenedFiles::filesChanged);
	}

	void OpenedFiles::removePathInternal(const QString &path)
	{
		if (!watcher->removePath(path))
		{
			auto files = watcher->files();
			resetWatcher(false);

			for (auto &p : files)
			{
				if (0 != QString::compare(path, p, Qt::CaseInsensitive))
					addPathInternal(p);
			}
		}
	}

	void OpenedFiles::addPathInternal(const QString &path)
	{
		QFileInfo info(path);

		if (info.exists() || info.isSymLink())
		{
			if (watcher->addPath(path))
			{
				if (!info.isRoot())
					addPathInternal(QDir(info.path()).canonicalPath());
			}
		}
	}

	OpenedFiles::FileMap::iterator OpenedFiles::findFileData(const QString &filePath)
	{
		auto it = file_map.find(filePath);

		if (file_map.end() != it)
			return it;

		return file_map.end();
	}

}
