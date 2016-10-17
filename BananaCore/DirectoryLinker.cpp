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

#include "DirectoryLinker.h"

#include "Core.h"
#include "Directory.h"
#include "ProjectGroup.h"

#include <QFileInfo>
#include <QDir>

using namespace std::placeholders;

namespace Banana
{

	BaseDirectoryLinker::BaseDirectoryLinker()
		: projectGroup(nullptr)
		, topDirectory(nullptr)
		, directory(nullptr)
	{

	}

	BaseDirectoryLinker::~BaseDirectoryLinker()
	{
		disconnectAll();
	}

	void BaseDirectoryLinker::setDirectory(Directory *directory)
	{
		if (this->directory != directory)
		{
			disconnectAll();
			if (nullptr != directory)
			{
				topDirectory = dynamic_cast<AbstractProjectDirectory *>(directory->getTopDirectory());
				projectGroup = dynamic_cast<ProjectGroup *>(topDirectory->parent());
				Q_ASSERT(nullptr != projectGroup);

				auto project_dir = projectGroup->getActiveProjectDirectory();
				if (nullptr != project_dir && project_dir != topDirectory)
				{
					if (topDirectory == directory)
					{
						topDirectory = project_dir;
						directory = project_dir;
					} else
					{
						auto found = project_dir->findFileT<Directory>(directory->getFilePath(), false);
						if (nullptr != found)
						{
							topDirectory = project_dir;
							directory = found;
						}
					}
				}

				Q_ASSERT(nullptr != topDirectory);

				connectProjectGroup();
			} else
			{
				topDirectory = nullptr;
				projectGroup = nullptr;
			}
			this->directory = directory;
			doConnectObject(topDirectory);
			updateFileLinks(true);
			updateChildrenDirectory(dynamic_cast<QObject *>(this), directory);
		}
	}

	Directory *BaseDirectoryLinker::getDirectory() const
	{
		return directory;
	}

	void BaseDirectoryLinker::addConnectionFor(QObject *object, const QMetaObject::Connection &connection)
	{
		auto it = connections.find(object);

		if (connections.end() == it)
			it = connections.insert(std::pair<QObject *, std::vector<QMetaObject::Connection>>(object, {})).first;

		auto &vec = it->second;
		vec.push_back(connection);
	}

	QString BaseDirectoryLinker::getRelativeFilePath(const QString &path) const
	{
		if (nullptr != directory && !path.isEmpty())
			return directory->getRelativeFilePathFor(path);

		return path;
	}

	QString BaseDirectoryLinker::getAbsoluteFilePath(const QString &path) const
	{
		if (nullptr != directory && !path.isEmpty())
		{
			return directory->getAbsoluteFilePathFor(path, true);
		}

		return path;
	}

	QObject *BaseDirectoryLinker::getFileSystemItemForPath(const QString &path) const
	{
		AbstractFileSystemObject *result = nullptr;
		if (nullptr != directory && !path.isEmpty())
		{
			auto absolutePath = getAbsoluteFilePath(path);
			result = directory->findFileSystemObject(absolutePath, false);

			if (nullptr == result)
			{
				QFileInfo info(absolutePath);

				if (info.exists() && info.isFile())
					result = topDirectory->addExistingFile(info.filePath(), false);
			}
		}

		if (nullptr != result)
			return result->getObject();

		return nullptr;
	}

	void BaseDirectoryLinker::onChildObjectConnectionChanged(QObject *object, ConnectionState state)
	{
		switch (state)
		{
			case ConnectionState::Connected:
				connectObject(object);
				break;

			case ConnectionState::Disconnected:
				disconnectObject(object);
				break;

			case ConnectionState::Destroyed:
			{
				objectDestroyed(object);
			}	break;
		}
	}

	void BaseDirectoryLinker::connectObject(QObject *object)
	{
		auto old_dir = directory;
		doConnectObject(object);
		updateFileLinks(old_dir != directory);
	}

	void BaseDirectoryLinker::disconnectObject(QObject *object)
	{
		auto old_dir = directory;
		doDisconnectObject(object);
		updateFileLinks(old_dir != directory);
	}

	void BaseDirectoryLinker::objectDestroyed(QObject *object)
	{
		auto old_dir = directory;
		afterObjectDestroyed(object);
		updateFileLinks(old_dir != directory);
	}

	void BaseDirectoryLinker::connectProjectGroup()
	{
		if (nullptr != projectGroup)
		{
			addConnectionFor(projectGroup, QObject::connect(projectGroup, &QObject::destroyed,
				std::bind(&BaseDirectoryLinker::onObjectDestroyed, this, _1)));
			addConnectionFor(projectGroup, QObject::connect(projectGroup, &ProjectGroup::activeProjectDirectoryChanged,
				std::bind(&BaseDirectoryLinker::onActiveProjectDirectoryChanged, this)));
		}
	}

	void BaseDirectoryLinker::disconnectProjectGroup()
	{
		auto it = connections.find(projectGroup);

		if (connections.end() != it)
		{
			for (auto &connection : it->second)
				QObject::disconnect(connection);

			connections.erase(it);
		}

		connections.clear();

		topDirectory = nullptr;
		directory = nullptr;
	}

	void BaseDirectoryLinker::updateDirectories(AbstractProjectDirectory *top_directory, Directory *directory)
	{
		disconnectAll();

		bool dir_changed = (this->directory != directory);

		this->topDirectory = top_directory;
		this->directory = directory;
		connectProjectGroup();
		doConnectObject(top_directory);
		updateFileLinks(dir_changed);
	}

	void BaseDirectoryLinker::doConnectObject(QObject *object)
	{
		Q_ASSERT(nullptr != object);

		if (object == topDirectory)
		{
			addConnectionFor(topDirectory, QObject::connect(topDirectory, &QObject::destroyed,
				std::bind(&BaseDirectoryLinker::onObjectDestroyed, this, _1)));
		}

		auto directory = dynamic_cast<Directory *>(object);
		if (nullptr != directory)
		{
			addConnectionFor(directory, QObject::connect(directory, &ObjectGroup::childObjectConnectionChanged,
				std::bind(&BaseDirectoryLinker::onChildObjectConnectionChanged, this, _1, _2)));
		}

		foreach (QObject *child, object->children())
			doConnectObject(child);
	}

	void BaseDirectoryLinker::doDisconnectObject(QObject *object)
	{
		Q_ASSERT(nullptr != object);

		if (directory != topDirectory && object == directory && directory->children().size() > 0)
		{
			disconnectAll();
			directory = topDirectory;
			connectProjectGroup();
			doConnectObject(topDirectory);
		} else
		{
			if (object == topDirectory)
			{
				disconnectAll();

				topDirectory = nullptr;
				directory = nullptr;
				connectProjectGroup();
			} else
			if (object == directory)
				directory = topDirectory;

			auto it = connections.find(object);

			if (connections.end() != it)
			{
				for (auto &connection : it->second)
					QObject::disconnect(connection);
				connections.erase(it);
			}
		}
	}

	void BaseDirectoryLinker::afterObjectDestroyed(QObject *object)
	{
		if (object == projectGroup)
		{
			connections.clear();

			projectGroup = nullptr;
			topDirectory = nullptr;
			directory = nullptr;
		} else
		if (object == topDirectory)
		{
			disconnectProjectGroup();
			connectProjectGroup();
		} else
		{
			connections.erase(object);

			if (object == directory)
				directory = topDirectory;
		}
	}

	void BaseDirectoryLinker::updateFileLinks(bool)
	{
		// do nothing
	}

	void BaseDirectoryLinker::updateDirectoryWithParents()
	{
		auto obj = dynamic_cast<QObject *>(this);
		if  (nullptr != obj)
		{
			obj = obj->parent();
			while (nullptr != obj)
			{
				auto linker = dynamic_cast<BaseDirectoryLinker *>(obj);
				if (nullptr != linker)
				{
					auto object = dynamic_cast<Object *>(obj);
					if (nullptr != object)
						object->beginLoad();
					linker->updateDirectoryWithParents();
					setDirectory(linker->directory);
					if (nullptr != object)
						object->endLoad();

					return;
				}

				obj = obj->parent();
			}
		}
	}

	void BaseDirectoryLinker::unbindFile(AbstractFile *file)
	{
		if (nullptr != file)
			file->unbind(false);
	}

	void BaseDirectoryLinker::disconnectFile(AbstractFile *file)
	{
		if (nullptr != file)
		{
			auto it = connections.find(file);
			if (connections.end() != it)
			{
				for (auto &connection : it->second)
					QObject::disconnect(connection);

				connections.erase(it);
			}

			unbindFile(file);
		}
	}

	void BaseDirectoryLinker::disconnectAll()
	{
		for (auto &item : connections)
			for (auto &connection : item.second)
				QObject::disconnect(connection);

		connections.clear();
	}

	void BaseDirectoryLinker::updateChildrenDirectory(QObject *object, Directory *directory)
	{
		if (nullptr != object)
		{
			for (auto child : object->children())
			{
				auto linker = dynamic_cast<BaseDirectoryLinker *>(child);

				if (nullptr != linker)
				{
					linker->setDirectory(directory);
				}

				updateChildrenDirectory(child, directory);
			}
		}
	}

	void BaseDirectoryLinker::onActiveProjectDirectoryChanged()
	{
		Q_ASSERT(nullptr != projectGroup);

		auto project_dir = projectGroup->getActiveProjectDirectory();

		if (nullptr != project_dir && topDirectory != project_dir)
		{
			if (nullptr != topDirectory)
			{
				if (topDirectory == directory)
				{
					updateDirectories(project_dir, project_dir);
					return;
				}

				if (nullptr != directory)
				{
					auto found = project_dir->findFileT<Directory>(directory->getFilePath(), false);

					if (nullptr != found)
						updateDirectories(project_dir, found);

					return;
				}
			}
			updateDirectories(project_dir, nullptr);
		}
	}

	void BaseDirectoryLinker::onObjectDestroyed(QObject *object)
	{
		objectDestroyed(object);
	}

}
