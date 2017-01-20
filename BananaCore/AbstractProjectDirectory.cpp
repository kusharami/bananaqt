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

#include "AbstractProjectDirectory.h"

#include "AbstractProjectFile.h"
#include "ProjectGroup.h"
#include "IProjectGroupDelegate.h"
#include "OpenedFiles.h"
#include "Utils.h"
#include "UndoStack.h"

#include <QDir>
#include <QFileInfo>
#include <QChildEvent>
#include <QDebug>
#include <QRegExp>
#include <QUndoGroup>

namespace Banana
{

AbstractProjectDirectory::AbstractProjectDirectory(const QString &path)
	: RootDirectory(path)
	, projectFile(nullptr)
	, nfRememberAnswer(Answer::Unknown)
	, afRememberAnswer(Answer::Unknown)
{
}

void AbstractProjectDirectory::setParent(QObject *parent)
{
	RootDirectory::setParent(parent);

	auto projectGroup = getProjectGroup();
	Q_ASSERT(nullptr != projectGroup);

	if (nullptr == projectFile)
	{
		projectFile = createProjectFile();
		projectFile->bind();
	}
}

QString AbstractProjectDirectory::getFilePath(
		const AbstractDirectory *relativeTo) const
{
	if (nullptr != relativeTo)
	{
		if (relativeTo == this)
			return QString();

		return QDir(relativeTo->getFilePath()).relativeFilePath(objectName());
	}

	return objectName();
}

QString AbstractProjectDirectory::getFileName() const
{
	return QFileInfo(objectName()).fileName();
}

QString AbstractProjectDirectory::getFixedName(const QString &source) const
{
	QString result(source);

	if (result.isEmpty())
		result = FileNamingPolicy::getFixedName(result);

	return RootDirectory::getFixedName(result);
}

std::vector<const char *> AbstractProjectDirectory::getFileExtensions(
		bool toCreate) const
{
	std::vector<const char *> result;
	for (auto &item : getRegisteredFileTypes())
	{
		if (!toCreate || canBeCreated(item.fileMetaObject))
			result.push_back(item.extension);
	}

	return result;
}

AbstractProjectFile *AbstractProjectDirectory::getProjectFile() const
{
	return projectFile;
}

AbstractFile *AbstractProjectDirectory::newFile(
		const QString &path, bool first, bool multiple)
{
	AbstractFile *resultFile = nullptr;
	auto delegate = getProjectGroupDelegate();
	auto projectGroup = getProjectGroup();

	bool silent = (nullptr == delegate || projectGroup->isSilent());

	Answer *rememberAnswer = (multiple && !silent
							   ? &this->nfRememberAnswer : nullptr);

	if (first && !silent)
		this->nfRememberAnswer = Answer::Unknown;

	if (nullptr != rememberAnswer && *rememberAnswer == Answer::Abort)
		return nullptr;

	QString absolutePath(getAbsoluteFilePathFor(path));
	if (!QFileInfo(absolutePath).isDir())
	{
		const QMetaObject *fileType;
		if (Utils::CheckFilePath(path))
		{
			if (isFileReplaceAllowed(absolutePath, rememberAnswer, &fileType))
			{
				resultFile = dynamic_cast<AbstractFile *>(
								  findFileSystemObject(absolutePath, false));
				bool found = (nullptr != resultFile);
				if (!found)
				{
					resultFile = dynamic_cast<AbstractFile *>(
									  fileType->newInstance());
					Q_ASSERT(nullptr != resultFile);

					auto first_child = initFileSystemObject(
										   resultFile, absolutePath);
					Q_ASSERT(nullptr != first_child);
				}

				if (!resultFile->create())
				{
					if (!found)
						delete resultFile;
					resultFile = nullptr;

					filePathError(Error::CannotWriteFile, absolutePath);
				}
			}
		} else
			filePathError(Error::InvalidFilePath, absolutePath);
	} else
	{
		filePathError(Error::CannotReplaceDirectoryWithFile, absolutePath);
	}

	return resultFile;
}

AbstractFile *AbstractProjectDirectory::addFile(
		const QString &path, bool mustExist, bool verbose)
{
	AbstractFile *resultFile = nullptr;

	QFileInfo fileInfo(getAbsoluteFilePathFor(path));
	bool isDir = fileInfo.isDir();

	if (!isDir || !mustExist)
	{
		isDir = false;
		if (Utils::CheckFilePath(path))
		{
			auto object = findFileSystemObject(path, false);
			resultFile = dynamic_cast<AbstractFile *>(object);

			if (nullptr != resultFile)
			{
				// do nothing
			} else
			if (nullptr == object)
			{	 // Should not have a directory with the same name
				if (!mustExist || fileInfo.exists())
				{
					auto fileType = getFileTypeByExtension(path);
					Q_ASSERT(nullptr != fileType);

					resultFile = dynamic_cast<AbstractFile *>(
									 fileType->newInstance());
					if (nullptr == resultFile)
						return nullptr;

					auto firstChild = initFileSystemObject(resultFile, path);
					Q_ASSERT(nullptr != firstChild);
				} else
				if (verbose)
					filePathError(Error::FileNotFound, path);
			} else
				isDir = true;
		} else
		if (verbose)
			filePathError(Error::InvalidFilePath, path);
	}

	if (isDir && verbose)
	{
		filePathError(Error::CannotReplaceDirectoryWithFile, path);
	}

	return resultFile;
}

AbstractFile *AbstractProjectDirectory::linkFile(QString targetFilePath,
												 QString linkFilePath,
												 bool mustExist,
												 bool verbose)
{
	QFileInfo target(getAbsoluteFilePathFor(targetFilePath));
	QFileInfo link(getAbsoluteFilePathFor(linkFilePath));

	if (mustExist && !target.isFile())
		return nullptr;

	targetFilePath = target.filePath();
	linkFilePath = link.filePath();

	if (0 == QString::compare(
			targetFilePath, linkFilePath, Qt::CaseInsensitive))
		return addFile(linkFilePath, mustExist, verbose);

	if (link.isFile())
	{
		if (target.exists())
		{
			if (0 == QString::compare(target.canonicalFilePath(),
									  link.canonicalFilePath(),
									  Qt::CaseInsensitive))
			{
				return addExistingFile(linkFilePath, verbose);
			}
		}
	} else
	{
		bool prepared = false;
		if (link.isSymLink())
		{
			prepared = Utils::DeleteFileOrLink(link);
		} else
			prepared = QDir().mkpath(link.path());

		if (prepared && Utils::CreateSymLink(targetFilePath, linkFilePath))
		{
			return addExistingFile(linkFilePath, verbose);
		}
	}

	if (!mustExist)
	{
		return addFile(linkFilePath, false, verbose);
	}

	if (verbose)
		filePathError(Error::CannotWriteFile, linkFilePath);

	return nullptr;
}

Directory *AbstractProjectDirectory::addSearchPath(
		const QString &path, int order, bool verbose)
{
	auto result = dynamic_cast<Directory *>(findFileSystemObject(path, false));
	if (nullptr == result)
		result = addDirectory(path, false, verbose);

	if (nullptr != result)
		result->setSearchOrderForAncestor(this, order);

	return result;
}

Directory *AbstractProjectDirectory::addDirectory(
		const QString &path, bool mustExist, bool verbose)
{
	if (mustExist)
	{
		QDir dir(path);
		if (!dir.exists())
		{
			if (verbose)
				filePathError(Error::DirectoryNotFound, path);
			return nullptr;
		}
	}

	auto dir = new Directory;
	if (nullptr == initFileSystemObject(dir, path))
	{
		delete dir;
		if (verbose)
			filePathError(Error::CannotCreateDirectory, path);
		return nullptr;
	}

	return dir;
}

Directory *AbstractProjectDirectory::linkDirectory(QString targetPath,
												   QString linkPath,
												   bool mustExist,
												   bool verbose)
{
	QFileInfo target(getAbsoluteFilePathFor(targetPath));
	QFileInfo link(getAbsoluteFilePathFor(linkPath));

	if (mustExist && !target.isDir())
		return nullptr;

	targetPath = target.filePath();
	linkPath = link.filePath();

	if (0 == QString::compare(targetPath, linkPath, Qt::CaseInsensitive))
		return addDirectory(linkPath, mustExist, verbose);

	if (link.isDir())
	{
		if (target.exists())
		{
			if (0 == QString::compare(target.canonicalFilePath(),
									  link.canonicalFilePath(),
									  Qt::CaseInsensitive))
			{
				return addDirectory(linkPath, true, verbose);
			}
		}
	} else
	{
		bool prepared = false;
		if (link.isSymLink())
		{
			prepared = Utils::DeleteFileOrLink(link);
		} else
			prepared = QDir().mkpath(link.path());

		if (prepared && Utils::CreateSymLink(targetPath, linkPath))
		{
			return addDirectory(linkPath, true, verbose);
		}
	}

	if (!mustExist)
	{
		return addDirectory(linkPath, false, verbose);
	}

	if (verbose)
		filePathError(Error::CannotWriteFile, linkPath);

	return nullptr;
}

AbstractFile *AbstractProjectDirectory::addExistingFile(
		const QString &path, bool verbose)
{
	return addFile(path, true, verbose);
}

AbstractFile *AbstractProjectDirectory::linkExistingFile(
		const QString &target, const QString &link, bool verbose)
{
	return linkFile(target, link, true, verbose);
}

Directory *AbstractProjectDirectory::findDirectory(const QString &path)
{
	return findFileT<Directory>(path, false);
}

bool AbstractProjectDirectory::saveFile(AbstractFile *file)
{
	if (file->isOpen() && !file->save())
	{
		filePathError(Error::CannotWriteFile, file->getFilePath());
		return false;
	}

	return true;
}

bool AbstractProjectDirectory::validateFileSaveAsPath(
		AbstractFile *file,
		QString &newFilePath,
		const QString &selectedFilter) const
{
	auto metaObject = file->metaObject();

	QFileInfo newInfo(newFilePath);
	QString currentExtension(file->getFileExtension());
	QString givenName(newInfo.fileName());
	if (newFilePath.endsWith(currentExtension, Qt::CaseInsensitive))
	{
		givenName = stripExtension(givenName, currentExtension);
	} else
	{
		const char *newExtension;

		if (metaObject == getFileTypeByExtension(newFilePath, &newExtension))
		{
			givenName = stripExtension(givenName, newExtension);
		} else
		if (newExtension == nullptr || 0 == newExtension[0])
		{
			if (!selectedFilter.isEmpty())
				currentExtension = getFileExtensionFromFilter(metaObject, selectedFilter);

			givenName = newInfo.fileName();
			newFilePath += currentExtension;
		} else
		{
			errorMessage(errorFormatStr(Error::FileCannotBeSavedInSpecificFormat)
					.arg(getFileTypeTitle(metaObject),
						 getFileFormatName(newExtension)));

			return false;
		}
	}

	QString correctName(file->getFixedName(givenName));

	if (0 != givenName.compare(correctName, Qt::CaseSensitive))
	{
		errorMessage(errorFormatStr(Error::CannotBeUsedAsFileName)
					 .arg(givenName, getFileTypeTitle(file->metaObject()),
					 correctName));

		return false;
	}

	return true;
}

bool AbstractProjectDirectory::saveFileAs(AbstractFile *file,
										  const QString &filePath,
										  const QString &selectedFilter,
										  bool verbose,
										  bool first,
										  bool multiple)
{
	auto delegate = getProjectGroupDelegate();
	auto projectGroup = getProjectGroup();

	bool silent = !verbose && (nullptr == delegate || projectGroup->isSilent());

	Answer *rememberAnswer = (multiple && !silent
							  ? &this->nfRememberAnswer : nullptr);

	if (first && !silent)
		this->nfRememberAnswer = Answer::Unknown;

	if (nullptr != rememberAnswer && *rememberAnswer == Answer::Abort)
		return false;

	QString newFilePath(filePath);
	if (!newFilePath.isEmpty()
	&&	validateFileSaveAsPath(file, newFilePath, selectedFilter)
	&&	isFileReplaceAllowed(newFilePath, rememberAnswer, nullptr))
	{
		auto found = findFileSystemObject(newFilePath, false);
		auto resultFile = dynamic_cast<AbstractFile *>(found);

		if (nullptr != resultFile)
		{
			if (found == file)
			{
				saveFile(file);
				return true;
			}

			delete resultFile;
		}

		bool ok = true;

		auto oldFilePath = file->getFilePath();

		QFileInfo info(newFilePath);
		if ((file->isOpen() || QFile::exists(oldFilePath))
		&&	(info.exists() || info.isSymLink()))
		{
			ok = Utils::DeleteFileOrLink(info);
		}

		if (ok)
		{
			auto connection = QObject::connect(
								  file, &AbstractFile::updateFilePathError,
			[&ok](const QString &, const QString &) mutable
			{
				ok = false;
			});

			auto oldCanonicalPath = file->getCanonicalFilePath();

			file->unwatchFile();

			file->setLoadError(true);
			auto resultDir = initFileSystemObject(file, newFilePath);

			QObject::disconnect(connection);

			if (ok)
			{
				Q_ASSERT(nullptr != resultDir);

				auto cfile = dynamic_cast<AbstractFile *>(
								 findFileSystemObject(oldCanonicalPath, true));
				if (nullptr != cfile)
				{
					cfile->watchFile();
				}
			} else
			{
				initFileSystemObject(file, oldFilePath);
				file->watchFile();
			}

			file->setLoadError(false);
		}

		if (ok)
		{
			saveFile(file);
		}

		return ok;
	}

	return false;
}

AbstractFile *AbstractProjectDirectory::findFile(const QString &path)
{
	return findFileT<AbstractFile>(path, false);
}

IProjectGroupDelegate *AbstractProjectDirectory::getProjectGroupDelegate() const
{
	auto project_group = getProjectGroup();

	if (nullptr != project_group)
		return project_group->getDelegate();

	return nullptr;
}

ProjectGroup *AbstractProjectDirectory::getProjectGroup() const
{
	return dynamic_cast<ProjectGroup *>(parent());
}

OpenedFiles *AbstractProjectDirectory::getOpenedFiles() const
{
	auto projectGroup = getProjectGroup();
	if (nullptr != projectGroup)
	{
		return projectGroup->getOpenedFiles();
	}

	return nullptr;
}

QObjectList AbstractProjectDirectory::findFiles(const QString &regExp)
{
	QObjectList result;

	internalFindFiles(this, QRegExp(regExp, Qt::CaseInsensitive), result);

	return result;
}

QObjectList AbstractProjectDirectory::getSearchDirectoryList() const
{
	QObjectList result;

	fillSearchDirListFrom(this, result);

	std::sort(result.begin(), result.end(), [](QObject *a, QObject *b) -> bool
	{
		auto dir1 = static_cast<Directory *>(a);
		auto dir2 = static_cast<Directory *>(b);
		return (dir1->getSearchOrder() < dir2->getSearchOrder());
	});

	return result;
}

void AbstractProjectDirectory::clearSearchPaths()
{
	clearSearchPaths(this);
}

void AbstractProjectDirectory::saveAllFiles()
{
	QList<AbstractFile *> not_saved;
	saveAllFiles(this, not_saved);
	if (not_saved.count() > 0)
	{
		if (not_saved.count() == 1)
			filePathError(Error::CannotWriteFile,
						  not_saved.at(0)->getFilePath());
		else
			errorMessage(Directory::tr("Some files were unable to be saved!"));
	}
}

AbstractProjectFile *AbstractProjectDirectory::createProjectFile()
{
	auto name = QDir(getFilePath()).dirName();
	auto result = getProjectFileMetaObject()->newInstance(Q_ARG(QString, name));
	if (nullptr != result)
	{
		auto file = dynamic_cast<AbstractProjectFile *>(result);
		if (nullptr == file)
		{
			delete result;
			return nullptr;
		}

		result->setParent(this);
		return file;
	}

	return nullptr;
}

bool AbstractProjectDirectory::isFileReplaceAllowed(
		const QString &absolutePath,
		Answer *rememberAnswer,
		const QMetaObject **outFileType)
{
	if (QFileInfo(absolutePath).isDir())
		return false;

	auto file_type = getFileTypeByExtension(absolutePath);
	Q_ASSERT(nullptr != file_type);

	if (nullptr != outFileType)
		*outFileType = file_type;

	auto project_group = getProjectGroup();
	Q_ASSERT(nullptr != project_group);

	auto found = findFileSystemObject(absolutePath, true);

	bool foundSameType = false;

	if (nullptr != found)
	{
		auto foundFile = dynamic_cast<AbstractFile *>(found);
		if (nullptr != foundFile)
		{
			if (0 != QString::compare(foundFile->getFilePath(),
									  absolutePath,
									  Qt::CaseInsensitive))
			{
				filePathError(Error::CannotWriteFile, absolutePath);
				return false;
			}

			if (project_group->getOpenedFiles()->fileIsOpened(
					foundFile->getCanonicalFilePath()))
			{
				filePathError(Error::CannotReplaceOpenedFile, absolutePath);
				return false;
			}

			foundSameType = (file_type == foundFile->metaObject());
		}

		auto found_dir = dynamic_cast<Directory *>(found);
		if (nullptr != found_dir)
		{
			filePathError(Error::CannotReplaceDirectoryWithFile, absolutePath);
			return false;
		}
	}

	if (foundSameType || QFile::exists(absolutePath))
	{
		auto delegate = getProjectGroupDelegate();
		Q_ASSERT(nullptr != delegate);
		switch (delegate->shouldReplaceFile(absolutePath, rememberAnswer))
		{
			case Answer::Yes:
			case Answer::YesToAll:
				break;

			default:
				return false;
		}
	}

	return true;
}

void AbstractProjectDirectory::internalFindFiles(
		Directory *directory, const QRegExp &regExp, QObjectList &dest)
{
	for (auto child : directory->children())
	{
		auto dir = dynamic_cast<Directory *>(child);
		if (nullptr != dir)
		{
			internalFindFiles(dir, regExp, dest);
			continue;
		}

		auto file = dynamic_cast<AbstractFile *>(child);
		if (nullptr != file)
		{
			if (regExp.exactMatch(file->getFilePath()))
				dest.push_back(file);
		}
	}
}

void AbstractProjectDirectory::saveAllFiles(
		Directory *directory, QList<AbstractFile *> &notSaved)
{
	for (auto child : directory->getChildren())
	{
		auto dir = dynamic_cast<Directory *>(child);
		if (nullptr != dir)
			saveAllFiles(dir, notSaved);
		else
		{
			auto file = dynamic_cast<AbstractFile *>(child);
			if (nullptr != file
			&&	file->isOpen()
			&&	file->isModified()
			&&	!file->save())
			{
				notSaved.push_back(file);
			}
		}
	}
}

QString AbstractProjectDirectory::stripExtension(
		const QString &fileName, const QString &extension)
{
	return QString(fileName.data(), fileName.length() - extension.length());
}

void AbstractProjectDirectory::filePathError(
		Error error, const QString &filePath)
{
	errorMessage(filePathErrorStr(error, filePath));
}

void AbstractProjectDirectory::errorMessage(const QString &message) const
{
	auto delegate = getProjectGroupDelegate();
	auto project_group = getProjectGroup();

	if (nullptr != delegate && !project_group->isSilent())
	{
		delegate->errorMessage(message);
	} else
	{
		qDebug() << message;
	}
}

void AbstractProjectDirectory::fillSearchDirListFrom(
		const Directory *dir, QObjectList &list) const
{
	Q_ASSERT(nullptr != dir);

	for (auto obj : dir->children())
	{
		auto dir = dynamic_cast<Directory *>(obj);
		if (nullptr != dir)
		{
			if (dir->isSearched())
				list.push_back(dir);
			fillSearchDirListFrom(dir, list);
		}
	}
}

void AbstractProjectDirectory::clearSearchPaths(Directory *dir)
{
	Q_ASSERT(nullptr != dir);

	for (auto obj : dir->children())
	{
		dir = dynamic_cast<Directory *>(obj);
		if (nullptr != dir)
		{
			dir->setSearched(false);
			clearSearchPaths(dir);
		}
	}
}

void AbstractProjectDirectory::onProjectFileModified()
{
	Q_ASSERT(nullptr != projectFile);
	if (!projectFile->isLoading())
		projectFile->setModified(true);
}

void AbstractProjectDirectory::onLinkDestroyed()
{
	if (nullptr != projectFile && !projectFile->isLoading())
		projectFile->setModified(true);
}

void AbstractProjectDirectory::descendantChanged(
		QObject *descendant, DescendantState state)
{
	auto fsys = dynamic_cast<AbstractFileSystemObject *>(descendant);

	if (nullptr != fsys)
	{
		bool symLink = fsys->isSymLink();

		switch (state)
		{
			case DescendantState::Added:
				if (symLink)
				{
					QObject::connect(
								descendant,
								&QObject::destroyed,
								this,
								&AbstractProjectDirectory::onLinkDestroyed);
				}
				break;

			case DescendantState::Removed:
				QObject::disconnect(descendant,
									&QObject::destroyed,
									this,
									&AbstractProjectDirectory::onLinkDestroyed);
				break;

			default:
				break;
		}

		if (nullptr != projectFile && !projectFile->isLoading())
		{
			if (state == DescendantState::SearchPathsChanged)
				emit changedSearchPaths();
			else if (!symLink)
				return;

			projectFile->setModified(true);
		}
	}
}

}
