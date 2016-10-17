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

#pragma once

#include "NamingPolicy.h"
#include "OpenedFiles.h"

#include <QString>
#include <QMetaObject>
#include <QFileInfo>

#include <vector>

class QObject;
class QUndoGroup;

namespace Banana
{
	class AbstractFile;

	class AbstractFileRegistrator
	{
	public:
		AbstractFileRegistrator(AbstractFile *thiz);
		virtual ~AbstractFileRegistrator();

		bool canChangeFilePath(const QString &newFilePath);

	private:
		void deleteData();
		void clearDataConnections();

	protected:
		bool isWatchedInternal() const;
		void watch(bool yes);
		bool updateFilePath(const QString &old_path, const QString &new_path);
		void createFileData(bool *reused);
		void closeFileData();
		void onSave();
		void updateData(QObject *data);
		void connectContext();
		void disconnectContext();
		void connectFileData();
		void disconnectFileData();
		void initCreateFileData();
		virtual QObject *doCreateFileData() const = 0;

		AbstractFile *thiz;
		QObject *data;
		OpenedFiles *openedFiles;
		QUndoGroup *undoGroup;
		AbstractNamingPolicy *namingPolicy;

		QMetaObject::Connection thisDestroyConnection;
		QMetaObject::Connection openedFilesConnection;
		QMetaObject::Connection undoGroupConnection;
		std::vector<QMetaObject::Connection> connections;
	};

	template <typename FILE_CLASS>
	class BaseFileRegistrator : public FILE_CLASS, public AbstractFileRegistrator
	{
	public:
		typedef BaseFileRegistrator Inherited;

		BaseFileRegistrator(const char *extension);

		virtual QString getFixedName(const QString &source) const override;
		virtual QString getNumberSeparator() const override;

		virtual bool isWatched() const override;

		virtual void unwatchFile() override;
		virtual void watchFile() override;

	protected:
		virtual void onSave() override;
		virtual void createData(bool *reused) override;
		virtual void destroyData() override;
		virtual QObject *doGetData() override { return data; }
		virtual void changeFilePath(const QString &new_path) override;
		virtual bool tryChangeFilePath(const QString &new_path) override;
	};

	template <typename FILE_CLASS, typename DATA_CLASS>
	class FileRegistrator : public BaseFileRegistrator<FILE_CLASS>
	{
	public:
		typedef FileRegistrator Inherited;

		FileRegistrator(const char *extension);

	protected:
		virtual QObject *doCreateFileData() const override;
	};

	template <typename FILE_CLASS>
	BaseFileRegistrator<FILE_CLASS>::BaseFileRegistrator(const char *extension)
		: FILE_CLASS(extension)
		, AbstractFileRegistrator(this)
	{

	}

	template <typename FILE_CLASS>
	QString BaseFileRegistrator<FILE_CLASS>::getFixedName(const QString &source) const
	{
		QString result(source);
		if (nullptr != namingPolicy)
			result = namingPolicy->getFixedName(result);

		return FILE_CLASS::getFixedName(result);
	}

	template <typename FILE_CLASS>
	QString BaseFileRegistrator<FILE_CLASS>::getNumberSeparator() const
	{
		if (nullptr != namingPolicy)
			return namingPolicy->getNumberSeparator();

		return FILE_CLASS::getNumberSeparator();
	}

	template <typename FILE_CLASS>
	bool BaseFileRegistrator<FILE_CLASS>::isWatched() const
	{
		return AbstractFileRegistrator::isWatchedInternal();
	}

	template <typename FILE_CLASS>
	void BaseFileRegistrator<FILE_CLASS>::unwatchFile()
	{
		watch(false);
	}

	template <typename FILE_CLASS>
	void BaseFileRegistrator<FILE_CLASS>::watchFile()
	{
		watch(true);
	}

	template <typename FILE_CLASS>
	void BaseFileRegistrator<FILE_CLASS>::onSave()
	{
		AbstractFileRegistrator::onSave();
		FILE_CLASS::onSave();
	}

	template <typename FILE_CLASS>
	void BaseFileRegistrator<FILE_CLASS>::createData(bool *reused)
	{
		createFileData(reused);
		FILE_CLASS::createData(reused);
	}

	template <typename FILE_CLASS>
	void BaseFileRegistrator<FILE_CLASS>::destroyData()
	{
		closeFileData();
		FILE_CLASS::destroyData();
	}

	template <typename FILE_CLASS>
	void BaseFileRegistrator<FILE_CLASS>::changeFilePath(const QString &new_path)
	{
		if (nullptr != data)
		{
			QString oldCanonicalPath(FILE_CLASS::canonical_path);
			FILE_CLASS::changeFilePath(new_path);
			AbstractFileRegistrator::updateFilePath(oldCanonicalPath, FILE_CLASS::canonical_path);
			data->setObjectName(QFileInfo(FILE_CLASS::canonical_path).baseName());
		} else
		{
			FILE_CLASS::changeFilePath(new_path);
		}
	}

	template <typename FILE_CLASS>
	bool BaseFileRegistrator<FILE_CLASS>::tryChangeFilePath(const QString &new_path)
	{
		bool ok = true;
		if (nullptr != data)
		{
			QFileInfo info(new_path);
			QString canonicalPath(info.exists() ? info.canonicalFilePath() : new_path);

			ok = AbstractFileRegistrator::canChangeFilePath(canonicalPath);
		}

		return ok && FILE_CLASS::tryChangeFilePath(new_path);
	}

	template <typename FILE_CLASS, typename DATA_CLASS>
	FileRegistrator<FILE_CLASS, DATA_CLASS>::FileRegistrator(const char *extension)
		: BaseFileRegistrator<FILE_CLASS>(extension)
	{

	}

	template <typename FILE_CLASS, typename DATA_CLASS>
	QObject *FileRegistrator<FILE_CLASS, DATA_CLASS>::doCreateFileData() const
	{
		return new DATA_CLASS;
	}
}
