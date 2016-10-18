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

#include "FileRegistrator.h"

#include "ProjectGroup.h"
#include "AbstractFile.h"
#include "AbstractDirectory.h"
#include "PropertyDef.h"

#include <QDir>

namespace Banana
{

	AbstractFileRegistrator::AbstractFileRegistrator(AbstractFile *thiz)
		: thiz(thiz)
		, data(nullptr)
		, openedFiles(nullptr)
		, namingPolicy(nullptr)
	{
	}

	AbstractFileRegistrator::~AbstractFileRegistrator()
	{
		closeFileData();
	}

	void AbstractFileRegistrator::createFileData(bool *reused)
	{
		disconnectFileData();

		initCreateFileData();
		Q_ASSERT(nullptr != openedFiles);

		data = openedFiles->getRegisteredFileData(thiz->canonical_path);

		if (nullptr != reused)
			*reused = (nullptr != data);

		if (nullptr == data)
			data = doCreateFileData();

		Q_ASSERT(nullptr != data);

		openedFiles->registerFile(thiz->canonical_path, data);

		data->setObjectName(QFileInfo(thiz->canonical_path).baseName());

		connectFileData();
	}

	void AbstractFileRegistrator::closeFileData()
	{
		if (nullptr != data)
		{
			disconnectFileData();

			deleteData();

			disconnectContext();
		}
	}

	bool AbstractFileRegistrator::canChangeFilePath(const QString &newFilePath)
	{
		if (nullptr != data)
		{
			if (nullptr != openedFiles)
				return openedFiles->canChangeFilePath(thiz->canonical_path, newFilePath);
		}

		return true;
	}

	void AbstractFileRegistrator::deleteData()
	{
		auto toDelete = data;
		data = nullptr;
		if (nullptr != openedFiles && nullptr != toDelete)
		{
			auto deleteResult = openedFiles->deleteFileData(thiz->canonical_path);
			Q_ASSERT(deleteResult == toDelete);
			Q_UNUSED(deleteResult);
		}
	}

	bool AbstractFileRegistrator::updateFilePath(const QString &old_path, const QString &new_path)
	{
		if (nullptr != data)
		{
			if (nullptr != openedFiles)
			{
				auto new_data = openedFiles->updateFilePath(old_path, new_path);

				if (nullptr == new_data)
					return false;

				updateData(new_data);
			}
		}

		return true;
	}

	void AbstractFileRegistrator::clearDataConnections()
	{
		for (auto &connection : connections)
			QObject::disconnect(connection);

		connections.clear();
	}

	bool AbstractFileRegistrator::isWatchedInternal() const
	{
		if (nullptr != openedFiles)
		{
			return openedFiles->isFileWatched(thiz);
		}

		return false;
	}

	void AbstractFileRegistrator::watch(bool yes)
	{
		if (nullptr != openedFiles)
		{
			openedFiles->watchFile(thiz, yes);
		}
	}

	void AbstractFileRegistrator::onSave()
	{
		auto data = dynamic_cast<Object *>(this->data);
		if (nullptr != data)
			data->setModified(false);
	}

	void AbstractFileRegistrator::updateData(QObject *data)
	{
		if (data != this->data)
		{
			thiz->disconnectData();
			disconnectFileData();

			this->data = data;

			thiz->connectData();
			connectFileData();

			emit thiz->dataChanged();
		}
	}

	void AbstractFileRegistrator::connectContext()
	{
		if (nullptr != openedFiles)
		{
			openedFilesConnection = QObject::connect(openedFiles, &QObject::destroyed, [this]()
			{
				openedFiles = nullptr;
			});
		}
	}

	void AbstractFileRegistrator::disconnectContext()
	{
		if (nullptr != openedFiles)
		{
			QObject::disconnect(openedFilesConnection);
			openedFiles = nullptr;
		}
	}

	void AbstractFileRegistrator::initCreateFileData()
	{
		disconnectContext();

		auto object = thiz->parent();

		while (nullptr != object)
		{
			auto group = dynamic_cast<ProjectGroup *>(object);

			if (nullptr != group)
			{
				openedFiles = group->getOpenedFiles();
				connectContext();
				break;
			}

			object = object->parent();
		}
	}

	void AbstractFileRegistrator::connectFileData()
	{
		auto data = this->data;
		if (nullptr != data)
		{
			thisDestroyConnection = QObject::connect(thiz, &QObject::destroyed, [this]()
			{
				deleteData();
			});

			connections.push_back(QObject::connect(data, &QObject::destroyed, [this]()
			{
				QObject::disconnect(thisDestroyConnection);
				connections.clear();
				this->data = nullptr;
			}));

			auto obj = dynamic_cast<Object *>(data);
			if (nullptr != obj)
			{
				connections.push_back(QObject::connect(obj, &Object::modifiedFlagChanged, [this](bool modified)
				{
					thiz->setModified(modified);
				}));
			}
		}
	}

	void AbstractFileRegistrator::disconnectFileData()
	{
		if (nullptr != data)
		{
			QObject::disconnect(thisDestroyConnection);

			clearDataConnections();
		}
	}

}
