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

#include "Directory.h"

#include "Utils.h"
#include "AbstractFile.h"
#include "Const.h"
#include "UniqueFileNameScope.h"
#include "AbstractProjectDirectory.h"

#include <set>

#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>

namespace Banana
{

	static const QString sFilterSeparator =
		#ifdef QT_WIN32
			"; "
		#else
			" "
		#endif
			;

	static const char *pFilterFmt = QT_TRANSLATE_NOOP("FileTypeFilter", "%1 (%2)");

	QString Directory::getAbsoluteFilePathFor(const QString &path, bool search) const
	{
		if (!QDir::isAbsolutePath(path))
		{
			auto first_path = AbstractDirectory::getAbsoluteFilePathFor(path);
			if (!search || QFileInfo::exists(first_path))
				return first_path;

			auto project_dir = dynamic_cast<AbstractProjectDirectory *>(getTopDirectory());

			if (nullptr != project_dir)
			{
				auto search_dirs = project_dir->getSearchDirectoryList();

				for (auto obj : search_dirs)
				{
					Q_ASSERT(nullptr != dynamic_cast<Directory *>(obj));
					auto dir = static_cast<Directory *>(obj);

					auto found_path = dir->getAbsoluteFilePathFor(path);
					if (QFileInfo::exists(found_path))
						return found_path;
				}
			}
		}

		return QDir::cleanPath(path);
	}

	Directory::Directory(const QString &name)
		: AbstractDirectory(this)
		, searched(false)
	{
		registerChildType(&Directory::staticMetaObject);
		for (auto &item : registeredFileTypes)
		{
			registerChildType(item.fileMetaObject);
		}

		setObjectName(name);
	}

	Directory *Directory::getTopDirectory() const
	{
		return dynamic_cast<Directory *>(AbstractDirectory::getTopDirectory());
	}

	Directory *Directory::getParentDirectory() const
	{
		return dynamic_cast<Directory *>(parent());
	}

	QObject *Directory::initFileSystemObject(QObject *object, const QString &path)
	{
		auto filesys_object = dynamic_cast<AbstractFileSystemObject *>(object);
		if (nullptr == filesys_object)
			return nullptr;

		QFileInfo info(getAbsoluteFilePathFor(path));

		auto dirpath = info.path();
		auto filepath = info.filePath();

		if (0 == filepath.compare(getFilePath(), Qt::CaseInsensitive))
			return nullptr;

		auto relative_dirpath = getRelativeFilePathFor(dirpath);

		Directory *dir;
		Directory *first_child_dir = nullptr;

		auto splitted = Utils::SplitPath(relative_dirpath);
		if (QDir::isRelativePath(relative_dirpath)
		&&	(splitted.isEmpty() || splitted.at(0) != ".."))
		{
			dir = this;

			for (auto it = splitted.begin(); it != splitted.end(); ++it)
			{
				auto &dir_name = *it;
				if (dir_name == ".")
					continue;

				auto found = dir->findChild<Directory *>(dir_name, Qt::FindDirectChildrenOnly);
				dir = (nullptr != found ? found : Object::create<Directory>(dir, dir_name));
				if (nullptr == first_child_dir)
					first_child_dir = dir;
			}
		} else
		{
			auto found = dynamic_cast<RootDirectory *>(findFileSystemObject(dirpath, false));
			dir = (nullptr != found ? found : Object::create<RootDirectory>(this, dirpath));
			if (nullptr == first_child_dir)
				first_child_dir = dir;
		}

		bool old_modified = false;
		auto file = dynamic_cast<AbstractFile *>(filesys_object);
		if (nullptr != file)
		{
			old_modified = file->isModified();
		}
		filesys_object->setFileName(info.fileName());
		if (nullptr == object->parent())
			modifyObject(object, false, false, true);
		object->setParent(dir);

		if (nullptr != file)
		{
			file->setModified(old_modified);
		}

		return nullptr != first_child_dir ? first_child_dir : object;
	}

	QString Directory::getFileTypeTitle(const QMetaObject *metaObject, bool plural)
	{
		return QCoreApplication::translate(plural
										   ? "FileTypeTitlePlural"
										   : "FileTypeTitle",
										   metaObject->className());
	}

	QString Directory::getFileFormatName(const char *extension,
										 bool plural,
										 bool emptyResult)
	{
		Q_ASSERT(nullptr != extension);

		if (extension[0] == 0)
			extension = " ";

		auto result = QCoreApplication::translate(plural
										   ? "FileFormatNamePlural"
										   : "FileFormatName",
										   extension);

		if (result == QString(extension))
		{
			result = emptyResult
					? QString()
					: QCoreApplication::translate(plural
									? "FileFormatNamePlural"
									: "FileFormatName",
									" ");
		}

		return result;
	}

	QString Directory::getFileFormatName(const QString &extension, bool plural, bool emptyResult)
	{
		std::string ext(extension.toStdString());

		if (!ext.empty() && ext.at(0) != '.')
			ext = "." + ext;

		return getFileFormatName(ext.c_str(), plural, emptyResult);
	}

	Directory::RegisteredFileTypes Directory::registeredFileTypes;

	const Directory::RegisteredFileTypes &Directory::getRegisteredFileTypes()
	{
		return registeredFileTypes;
	}

	void Directory::registerFileType(const char *extension,
									 const QMetaObject *fileMetaObject,
									 const QMetaObject *dataMetaObject)
	{
		Q_ASSERT(nullptr != extension);
		Q_ASSERT(nullptr != fileMetaObject);
		Q_ASSERT(nullptr != dataMetaObject);

		RegisteredFileType reg;
		reg.extension = extension;
		reg.fileMetaObject = fileMetaObject;
		reg.dataMetaObject = dataMetaObject;

		for (auto &item : registeredFileTypes)
		{
			if ((0 == strcmp(extension, item.extension)))
			{
				item = reg;
				return;
			}
		}

		registeredFileTypes.push_back(reg);
	}

	const QMetaObject *Directory::getFileTypeByExtension(const QString &filePath, const char **extensionPtr)
	{
		const QMetaObject *result = nullptr;
		const char *resultExtension = nullptr;
		int longestMatch = 0;
		for (auto &item : registeredFileTypes)
		{
			QString extension(item.extension);
			if (!extension.isEmpty()
			&&	filePath.endsWith(extension, Qt::CaseInsensitive))
			{
				int extensionLength = extension.length();
				if (extensionLength > longestMatch)
				{
					longestMatch = extensionLength;
					result = item.fileMetaObject;
					resultExtension = item.extension;
				}
			}
		}

		if (nullptr != result)
		{
			if (nullptr != extensionPtr)
				*extensionPtr = resultExtension;

			return result;
		}

		// find empty extension
		for (auto &item : registeredFileTypes)
		{
			if (0 == item.extension[0])
			{
				if (nullptr != extensionPtr)
					*extensionPtr = item.extension;
				return item.fileMetaObject;
			}
		}

		if (nullptr != extensionPtr)
			*extensionPtr = nullptr;
		return nullptr;
	}

	const QMetaObject *Directory::getFileTypeByExtensionPtr(const char *extension)
	{
		for (auto &item : registeredFileTypes)
		{
			if (extension == item.extension)
				return item.fileMetaObject;
		}

		return nullptr;
	}

	std::vector<const char *> Directory::getFileTypeExtensions(const QMetaObject *metaObject)
	{
		std::vector<const char *> result;
		for (auto &item : registeredFileTypes)
		{
			if (item.fileMetaObject == metaObject)
				result.push_back(item.extension);
		}

		return result;
	}

	const Directory::RegisteredFileType *Directory::findRegisteredFileType(const char *extension,
																		   const QMetaObject *fileMetaObject,
																		   const QMetaObject *dataMetaObject)
	{
		for (auto &item : registeredFileTypes)
		{
			if (extension == item.extension
			||	fileMetaObject == item.fileMetaObject
			||	dataMetaObject == item.dataMetaObject)
			{
				return &item;
			}
		}

		return nullptr;
	}

	QString Directory::getFilterForExtension(const char *extension)
	{
		QString wildcard(extension);

		if (wildcard.isEmpty())
			wildcard = "*.*";
		else
			wildcard = QString("*") + wildcard;

		return QCoreApplication::translate("FileTypeFilter", pFilterFmt)
				.arg(getFileFormatName(extension, true), wildcard);
	}

	QString Directory::getFileExtensionFromFilter(const QMetaObject *metaObject,
												  const QString &filter)
	{
		for (auto pszExtension : getFileTypeExtensions(metaObject))
		{
			if (pszExtension[0] != 0)
			{
				if (filter.contains(pszExtension, Qt::CaseInsensitive))
					return QString(pszExtension);
			}
		}

		return QString();
	}

	QString Directory::getFilterForFileType(const QMetaObject *metaObject)
	{
		QStringList extensions;

		for (auto pszExtension : getFileTypeExtensions(metaObject))
		{
			QString extension(pszExtension);
			if (!extension.isEmpty())
			{
				extensions.push_back(QString("*") + extension);
			}
		}

		if (extensions.isEmpty())
			return getFilterForExtension(pNoExtension);

		return QCoreApplication::translate("FileTypeFilter", pFilterFmt)
				.arg(getFileTypeTitle(metaObject, true),
					 extensions.join(sFilterSeparator));
	}

	QStringList Directory::getPossibleFiltersForFileType(const QMetaObject *metaObject)
	{
		QStringList result;

		auto pszExtensions = getFileTypeExtensions(metaObject);

		if (pszExtensions.size() > 1)
		{
			for (auto pszExtension : pszExtensions)
			{
				result.push_back(getFilterForExtension(pszExtension));
			}

			Utils::SortStringList(result);
		}

		result.prepend(getFilterForFileType(metaObject));

		return result;
	}

	QStringList Directory::getAllPossibleFilters()
	{
		QStringList result;

		QStringList extensions;
		std::set<const QMetaObject *> have;

		for (auto &item : registeredFileTypes)
		{
			QString extension(item.extension);
			if (!extension.isEmpty())
			{
				extensions.push_back(QString("*") + extension);

				if (have.end() == have.find(item.fileMetaObject))
				{
					result.push_back(getFilterForFileType(item.fileMetaObject));
					have.insert(item.fileMetaObject);
				}
			}
		}

		if (result.count() > 1)
		{
			Utils::SortStringList(result);

			result.prepend(
				QCoreApplication::translate("FileTypeFilter", pFilterFmt)
							.arg(tr("All supported file types"),
								 extensions.join(sFilterSeparator)));
		}

		result.push_back(getFilterForExtension(pNoExtension));

		return result;
	}

	QStringList Directory::getFileSaveAsFilters(AbstractFile *file)
	{
		Q_ASSERT(nullptr != file);

		auto metaObject = file->metaObject();

		QStringList extensions;
		QStringList result;

		for (auto pszExtension : getFileTypeExtensions(metaObject))
		{
			if (file->isWritableFormat(pszExtension))
			{
				extensions.push_back(QString("*") + QString(pszExtension));

				result.push_back(getFilterForExtension(pszExtension));
			}
		}

		if (result.count() > 1)
		{
			Utils::SortStringList(result);

			result.prepend(
				QCoreApplication::translate("FileTypeFilter", pFilterFmt)
							.arg(tr("All supported formats"),
								 extensions.join(sFilterSeparator)));
		}

		return result;
	}

	QString Directory::errorFormatStr(Directory::Error error)
	{
		switch (error)
		{
			case Error::FileNotFound:
				return tr("File '%1' is not found!");

			case Error::DirectoryNotFound:
				return tr("Directory '%1' is not found!");

			case Error::InvalidFilePath:
				return tr("'%1' is invalid file path!");

			case Error::CannotReplaceDirectoryWithFile:
				return tr("Cannot replace directory '%1' with a file!");

			case Error::CannotWriteFile:
				return tr("Unable to write %1 '%2'!");

			case Error::CannotCreateDirectory:
				return tr("Unable to create directory '%1'!");

			case Error::FileCannotBeSavedInSpecificFormat:
				return tr("%1 cannot be saved in %2 format.");

			case Error::CannotBeUsedAsFileName:
				return tr("'%1' cannot be used as %2 name. Try to use '%3'.");

			case Error::CannotReplaceOpenedFile:
				return tr("File '%1' cannot be replaced because it is open.");

			default:
				break;
		}

		return QString();
	}

	QString Directory::filePathErrorStr(Error error, const QString &filepath)
	{
		switch (error)
		{
			case Error::FileNotFound:
			case Error::DirectoryNotFound:
			case Error::InvalidFilePath:
			case Error::CannotReplaceDirectoryWithFile:
			case Error::CannotReplaceOpenedFile:
			case Error::CannotCreateDirectory:
				return errorFormatStr(error).arg(filepath);

			case Error::CannotWriteFile:
				return errorFormatStr(error).arg(getFileFormatNameFrom(filepath), filepath);

			default:
				break;
		}

		return QString();
	}

	void Directory::getFilesIn(Directory *dir, std::set<AbstractFile *> &files)
	{
		for (auto child : dir->children())
		{
			auto sub_dir = dynamic_cast<Directory *>(child);
			if (nullptr != sub_dir)
			{
				getFilesIn(sub_dir, files);
				continue;
			}

			auto file = dynamic_cast<AbstractFile *>(child);
			Q_ASSERT(nullptr != file);
			files.insert(file);
		}
	}

	void Directory::getDirContents(Directory *dir, std::set<AbstractFileSystemObject *> &output)
	{
		for (auto child : dir->children())
		{
			auto fsys = dynamic_cast<AbstractFileSystemObject *>(child);
			Q_ASSERT(nullptr != fsys);
			output.insert(fsys);

			auto sub_dir = dynamic_cast<Directory *>(child);
			if (nullptr != sub_dir)
			{
				getDirContents(sub_dir, output);
			}
		}
	}

	bool Directory::rename(const QString &new_name)
	{
		if (0 != QString::compare(objectName(), new_name, Qt::CaseSensitive))
		{
			return moveTo(dynamic_cast<Directory *>(parent()), new_name);
		}

		return true;
	}

	bool Directory::moveTo(Directory *target_dir, const QString &new_name)
	{
		bool ok = false;
		if (metaObject() == &Directory::staticMetaObject)
		{
			auto new_dir = new Directory(new_name.isEmpty() ? objectName() : new_name);
			new_dir->setParent(target_dir);

			QFileInfo dest_info(FileNamingPolicy::uniqueFilePath(QDir(target_dir->getFilePath()),
																 new_dir->objectName()));

			delete new_dir;

			std::set<AbstractFile *> files;
			getFilesIn(this, files);

			for (auto file : files)
			{
				file->unwatchFile();
			}

			QDir dir(getFilePath());
			if (!dir.exists() || QDir().rename(dir.path(), dest_info.filePath()))
			{
				setParent(target_dir);
				setObjectName(dest_info.fileName());

				for (auto file : files)
				{
					file->load_error = true;
					file->updateFilePath();
					file->load_error = false;
				}

				ok = true;
			} else
				updateFileNameError(new_name);

			for (auto file : files)
			{
				file->watchFile();
			}
		}

		return ok;
	}

	void Directory::setSearched(bool searched)
	{
		if (this->searched != searched)
		{
			this->searched = searched;
			descendantChanged(this, DescendantState::SearchPathsChanged);
		}
	}

	void Directory::setSearchOrder(int order)
	{
		if (searchOrder != order)
		{
			searchOrder = order;
			descendantChanged(this, DescendantState::SearchPathsChanged);
		}
	}

	void Directory::setSearchOrderForAncestor(Directory *ancestor, int order)
	{
		if (!searched || searchOrder != order)
		{
			searched = true;
			searchOrder = order;
			Q_ASSERT(nullptr != ancestor);
			ancestor->descendantChanged(this, DescendantState::SearchPathsChanged);
			setModified(true);
		}
	}

	QString Directory::getFileFormatNameFrom(const QString &filePath, bool plural)
	{
		QString result;
		int idx = 0;
		while (result.isEmpty())
		{
			idx = filePath.lastIndexOf('.', idx - 1);
			if (idx < 0)
				break;

			result = getFileFormatName(QString(&filePath.data()[idx]), plural, true);
		}

		if (result.isEmpty())
			return getFileFormatName(result, plural, false);

		return result;
	}

	void Directory::executeUpdateFilePathError(const QString &path, const QString &failed_path)
	{
		emit updatePathError(path, failed_path);
	}

	UniqueNameScope *Directory::createNameScope(const QMetaObject *meta_object,
												Qt::CaseSensitivity sensitivity) const
	{
		Q_ASSERT(Qt::CaseInsensitive == sensitivity);
		Q_UNUSED(sensitivity);

		return new UniqueFileNameScope(meta_object);
	}

	void Directory::sortChildren(QObjectList &children)
	{
		std::sort(children.begin(), children.end(),
		[](QObject *a, QObject *b) -> bool
		{
			auto ad = dynamic_cast<AbstractDirectory *>(a);
			auto bd = dynamic_cast<AbstractDirectory *>(b);

			if (nullptr != ad)
			{
				if (nullptr != bd)
					return ad->getFileName() < bd->getFileName();

				return true;
			}

			auto af = dynamic_cast<AbstractFile *>(a);
			auto bf = dynamic_cast<AbstractFile *>(b);

			if (nullptr != af && nullptr != bf)
				return QString::compare(af->getFileName(), bf->getFileName(), Qt::CaseInsensitive) < 0;

			return false;
		});
	}

	void Directory::childEvent(QChildEvent *event)
	{
		if (event->added())
		{
			descendantChanged(event->child(), DescendantState::Added);
		} else
		if (event->removed())
		{
			descendantChanged(event->child(), DescendantState::Removed);
		}

		ObjectGroup::childEvent(event);
	}

	void Directory::descendantChanged(QObject *descendant, DescendantState state)
	{
		AbstractDirectory::descendantChanged(descendant, state);

		setModified(true);
		doFlagsChanged();
	}

	RootDirectory::RootDirectory(const QString &path)
		: Directory(path)
	{

	}

	void RootDirectory::setPath(const QString &path)
	{
		setObjectName(path);
	}

	bool RootDirectory::rename(const QString &)
	{
		return false;
	}

	QString RootDirectory::getFileName() const
	{
		auto parent = dynamic_cast<AbstractDirectory *>(this->parent());
		if (nullptr != parent)
			return QDir(parent->getFilePath()).relativeFilePath(objectName());

		return objectName();
	}

	QString RootDirectory::getFixedName(const QString &source) const
	{
		auto parent = getParentDirectory();
		if (nullptr != parent)
			return QDir::cleanPath(QDir(parent->getFilePath()).absoluteFilePath(source));

		return QDir(source).path();
	}

}
