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

#pragma once

#include "AbstractFile.h"
#include "AbstractProjectDirectory.h"

#include <QMetaObject>
#include <QDir>

#include <map>
#include <vector>
#include <functional>

namespace Banana
{
	class AbstractProjectDirectory;
	class ProjectGroup;

	enum class ConnectionState;

	class BaseDirectoryLinker
	{
	public:
		BaseDirectoryLinker();
		virtual ~BaseDirectoryLinker();

		void setDirectory(Directory *directory);
		Directory *getDirectory() const;

	protected:
		virtual void doConnectObject(QObject *object);
		virtual void doDisconnectObject(QObject *object);
		virtual void afterObjectDestroyed(QObject *object);
		virtual void updateFileLinks(bool directoryChanged = false);

		void updateDirectoryWithParents();
		void addConnectionFor(QObject *object, const QMetaObject::Connection &connection);

		template <typename FILE_T, typename ObjectType>
		inline bool updateFileWithObject(FILE_T *&file, QObject *object, QString &pathRef,
										 void (ObjectType::*onFilePathChanged)(),
										 const QMetaObject *fileMetaObject = nullptr);

		template <typename FILE_T, typename ObjectType>
		inline bool setFilePath(FILE_T *&file,
								QString &pathRef,
								const QString &newPath,
								void (ObjectType::*onFilePathChanged)(),
								const QMetaObject *fileMetaObject = nullptr);

		template <typename FILE_T, typename ObjectType>
		inline void updateFileByRef(FILE_T *&file,
									QString &pathRef,
									void (ObjectType::*onFilePathChanged)(),
									const QMetaObject *fileMetaObject = nullptr);

		template <typename FILE_T, typename ObjectType>
		inline void connectFile(FILE_T *&file, void (ObjectType::*onFilePathChanged)());

		template <typename FILE_T>
		inline void disconnectFileByRef(FILE_T *&file);

		QString getRelativeFilePath(const QString &path) const;
		QString getAbsoluteFilePath(const QString &path) const;

		QObject *getFileSystemItemForPath(const QString &path) const;
		void unbindFile(AbstractFile *file);
		void disconnectFile(AbstractFile *file);
		void disconnectAll();

		ProjectGroup *projectGroup;
		AbstractProjectDirectory *topDirectory;
		Directory *directory;

		std::map<QObject *, std::vector<QMetaObject::Connection>> connections;

	private:
		template <typename FILE_T>
		inline FILE_T *castToFile(QObject *object, const QMetaObject *fileMetaObject);

		static void updateChildrenDirectory(QObject *object, Directory *directory);
		void onActiveProjectDirectoryChanged();
		void onObjectDestroyed(QObject *object);
		void onChildObjectConnectionChanged(QObject *object, ConnectionState state);

		void connectObject(QObject *object);
		void disconnectObject(QObject *object);
		void objectDestroyed(QObject *object);
		void connectProjectGroup();
		void disconnectProjectGroup();
		void updateDirectories(AbstractProjectDirectory *topDirectory, Directory *directory);
	};

	template <typename OBJECT>
	class DirectoryLinker : public OBJECT, public BaseDirectoryLinker
	{
	protected:
		typedef DirectoryLinker Inherited;
		virtual bool assignBegin(QObject *source, bool top) override;
	};

	template <typename FILE_T>
	FILE_T *BaseDirectoryLinker::castToFile(QObject *object, const QMetaObject *fileMetaObject)
	{
		return dynamic_cast<FILE_T *>((nullptr != fileMetaObject)
									  ? fileMetaObject->cast(object)
									  : object);
	}

	template <typename FILE_T, typename ObjectType>
	void BaseDirectoryLinker::updateFileByRef(FILE_T *&file,
											  QString &pathRef,
											  void (ObjectType::*onFilePathChanged)(),
											  const QMetaObject *fileMetaObject)
	{
		auto newFile = castToFile<FILE_T>(getFileSystemItemForPath(pathRef), fileMetaObject);
		if (newFile != file)
		{
			disconnectFileByRef(file);
			file = newFile;
			connectFile(file, onFilePathChanged);
		}

		if (nullptr != file)
			pathRef = file->getFilePathShort();
	}

	template <typename FILE_T>
	void BaseDirectoryLinker::disconnectFileByRef(FILE_T *&file)
	{
		disconnectFile(file);
		file = nullptr;
	}

	template <typename FILE_T, typename ObjectType>
	bool BaseDirectoryLinker::setFilePath(FILE_T *&file,
										  QString &pathRef,
										  const QString &newPath,
										  void (ObjectType::*onFilePathChanged)(),
										  const QMetaObject *fileMetaObject)
	{
		auto thiz = dynamic_cast<ObjectType *>(this);
		Q_ASSERT(nullptr != thiz);

		auto oldFile = file;
		auto oldPath = pathRef;
		pathRef = newPath;

		updateFileByRef(file, pathRef, onFilePathChanged, fileMetaObject);

		if (oldFile != file || pathRef != oldPath)
		{
			thiz->setModified(true);

			return true;
		}

		return false;
	}

	template <typename FILE_T, typename ObjectType>
	void BaseDirectoryLinker::connectFile(FILE_T *&file,
										  void (ObjectType::*onFilePathChanged)())
	{
		if (nullptr != file)
		{
			file->bind();
			file->open();

			auto thiz = dynamic_cast<ObjectType *>(this);
			Q_ASSERT(nullptr != thiz);

			addConnectionFor(file, QObject::connect(file, &QObject::destroyed,
													std::bind(&BaseDirectoryLinker::objectDestroyed, this, file)));

			addConnectionFor(file, QObject::connect(file, &AbstractFile::pathChanged, thiz, onFilePathChanged));
			addConnectionFor(file, QObject::connect(topDirectory,
													&AbstractProjectDirectory::changedSearchPaths,
													thiz, onFilePathChanged));

			auto updateFileLinksCB = std::bind(&BaseDirectoryLinker::updateFileLinks, thiz, false);

			addConnectionFor(file, QObject::connect(file, &AbstractFile::fileOpened, updateFileLinksCB));
			addConnectionFor(file, QObject::connect(file, &AbstractFile::fileReloaded, updateFileLinksCB));
			addConnectionFor(file, QObject::connect(file, &AbstractFile::fileClosed, updateFileLinksCB));
			addConnectionFor(file, QObject::connect(file, &AbstractFile::dataChanged, updateFileLinksCB));
		}
	}

	template <typename FILE_T, typename ObjectType>
	bool BaseDirectoryLinker::updateFileWithObject(FILE_T *&file,
												   QObject *object,
												   QString &pathPef,
												   void (ObjectType::*onFilePathChanged)(),
												   const QMetaObject *fileMetaObject)
	{
		if (nullptr == file)
		{
			auto newFile = castToFile<FILE_T>(object, fileMetaObject);
			if (nullptr != newFile)
			{
				auto shortPath = newFile->getFilePathShort();
				bool ok = (0 == pathPef.compare(shortPath, Qt::CaseInsensitive));

				if (!ok)
				{
					ok = (0 == QString::compare(topDirectory->getAbsoluteFilePathFor(pathPef, true),
												newFile->getFilePath(), Qt::CaseInsensitive));
				}

				if (ok)
				{
					file = newFile;
					pathPef = shortPath;
					connectFile(file, onFilePathChanged);

					return true;
				}
			}
		}

		return false;
	}

	template <typename OBJECT>
	bool DirectoryLinker<OBJECT>::assignBegin(QObject *source, bool top)
	{
		if (top || nullptr == this->parent())
		{
			auto sourceLinker = dynamic_cast<BaseDirectoryLinker *>(source);
			if (nullptr != sourceLinker)
				setDirectory(sourceLinker->getDirectory());
		}

		updateDirectoryWithParents();

		if (nullptr != source && nullptr == source->parent())
		{
			auto sourceLinker = dynamic_cast<BaseDirectoryLinker *>(source);
			if (nullptr != sourceLinker)
				sourceLinker->setDirectory(getDirectory());
		}

		return OBJECT::assignBegin(source, top);
	}
}
