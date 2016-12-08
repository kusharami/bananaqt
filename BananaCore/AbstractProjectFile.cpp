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

#include "AbstractProjectFile.h"

#include "AbstractProjectDirectory.h"
#include "Utils.h"
#include "OpenedFiles.h"
#include "Directory.h"
#include "SearchPaths.h"

#include <QFileInfo>

namespace Banana
{
	const QString AbstractProjectFile::SEARCH_PATHS_KEY = "SearchPaths";
	const QString AbstractProjectFile::IGNORED_FILES_KEY = "IgnoredFiles";
	const QString AbstractProjectFile::FILES_KEY = "Files";
	const QString AbstractProjectFile::TYPE_KEY = "Type";
	const QString AbstractProjectFile::PATH_KEY = "Path";
	const QString AbstractProjectFile::TARGET_KEY = "Target";
	const QString AbstractProjectFile::TYPE_FILE = "File";
	const QString AbstractProjectFile::TYPE_DIR = "Directory";
	const QString AbstractProjectFile::TYPE_DIR_LINK = "DirectoryLink";
	const QString AbstractProjectFile::TYPE_FILE_LINK = "FileLink";

	AbstractProjectFile::FileObjType AbstractProjectFile::getFileObjTypeFromString(const QString &str)
	{
		if (str == TYPE_DIR)
			return FileObjType::Directory;

		if (str == TYPE_DIR_LINK)
			return FileObjType::DirectoryLink;

		if (str == TYPE_FILE)
			return FileObjType::File;

		if (str == TYPE_FILE_LINK)
			return FileObjType::FileLink;

		return FileObjType::None;
	}

	AbstractProjectFile::AbstractProjectFile(const QString &name, const QString &extension)
		: VariantMapFile(extension)
		, openedFiles(nullptr)
		, searchPaths(nullptr)
	{
		setObjectName(name);

		(void) QT_TRANSLATE_NOOP("ClassName", "Banana::AbstractProjectFile");
		(void) QT_TRANSLATE_NOOP("Banana::AbstractProjectFile", "mHideIgnoredFiles");
		(void) QT_TRANSLATE_NOOP("Banana::AbstractProjectFile", "mIgnoredFilesPattern");
		(void) QT_TRANSLATE_NOOP("Banana::AbstractProjectFile", "mSearchPaths");
	}

	AbstractProjectFile::~AbstractProjectFile()
	{
		watch(false);
		delete searchPaths;
	}

	bool AbstractProjectFile::isWatched() const
	{
		if (nullptr != openedFiles)
		{
			return openedFiles->isFileWatched(this);
		}

		return false;
	}

	void AbstractProjectFile::watchFile()
	{
		watch(true);
	}

	QStringList AbstractProjectFile::getIgnoredFilesList() const
	{
		return mIgnoredFilesPattern.split(QRegExp("[\n\r]+|\\|"), QString::SkipEmptyParts);
	}

	void AbstractProjectFile::setIgnoredFilesList(const QStringList &value)
	{
		std::set<QString> join;

		QStringList newList;

		for (auto &str : value)
		{
			if (join.find(str) == join.end())
			{
				join.insert(str);
				newList.push_back(str);
			}
		}

		setIgnoredFilesPattern(newList.join('\n'));
	}

	SearchPaths *AbstractProjectFile::getSearchPaths()
	{
		if (searchPaths == nullptr)
		{
			auto project_dir = dynamic_cast<AbstractProjectDirectory *>(getTopDirectory());
			if (nullptr != project_dir)
				searchPaths = new SearchPaths(project_dir);
		}

		return searchPaths;
	}

	void AbstractProjectFile::resetSearchPaths()
	{
		getSearchPaths()->clear();
	}

	QString AbstractProjectFile::getAbsoluteFilePathFor(const QString &filepath) const
	{
		auto root_dir = dynamic_cast<Directory *>(parent());
		if (nullptr != root_dir)
			return root_dir->getAbsoluteFilePathFor(filepath);

		return QString();
	}

	void AbstractProjectFile::unwatchFile()
	{
		watch(false);
	}

	void AbstractProjectFile::saveData(QVariantMap &output)
	{
		VariantMapFile::saveData(output);

		QVariantList paths;

		auto root_dir = dynamic_cast<Directory *>(parent());
		saveProjectDirectory(root_dir,
							 getTopDirectory(),
							 paths);

		output.insert(FILES_KEY, paths);

		// ----
		paths.clear();

		auto ignored = getIgnoredFilesList();

		for (auto &pattern : ignored)
			paths.push_back(pattern);

		output.insert(IGNORED_FILES_KEY, paths);

		// ----
		paths.clear();

		auto dirs = getSearchPaths()->getDirectoryList();

		for (auto dir : dirs)
			paths.push_back(dir->getFilePath(root_dir));

		output.insert(SEARCH_PATHS_KEY, paths);
	}

	bool AbstractProjectFile::loadData(const QVariantMap &input)
	{
		bool ok = false;
		beginLoad();
		auto projectDir = dynamic_cast<AbstractProjectDirectory *>(getTopDirectory());
		auto rootDir = dynamic_cast<Directory *>(parent());

		if (nullptr != projectDir && projectDir->getProjectFile() == this && projectDir == rootDir)
		{
			auto siblings = projectDir->children();
			foreach (QObject *sibling, siblings)
			{
				if (sibling != this)
					delete sibling;
			}
			siblings.clear();

			if (VariantMapFile::loadData(input))
			{
				ok = true;

				auto value = Utils::ValueFrom(input, IGNORED_FILES_KEY);

				if (value.type() == QVariant::List)
				{
					auto list = value.toList();
					value.clear();

					QStringList ignored;

					for (auto &pattern : list)
					{
						if (pattern.type() == QVariant::String)
							ignored.push_back(pattern.toString());
					}

					setIgnoredFilesList(ignored);
				}

				value = Utils::ValueFrom(input, FILES_KEY);

				if (value.type() == QVariant::List)
				{
					auto list = value.toList();
					value.clear();
					struct FileInfo
					{
						QString path;
						QString target;
					};

					std::map<FileObjType, std::vector<FileInfo>> infos;

					for (auto it = list.begin(); it != list.end(); ++it)
					{
						const QVariant &itValue = *it;

						FileInfo info;
						auto type = FileObjType::None;
						switch (itValue.type())
						{
							case QVariant::Map:
							{
								QVariantMap map(itValue.toMap());

								auto v = Utils::ValueFrom(map, TYPE_KEY);
								if (v.type() != QVariant::String)
								{
									LOG_WARNING("Bad file type");
									continue;
								}
								type = getFileObjTypeFromString(v.toString());

								v = Utils::ValueFrom(map, PATH_KEY);
								if (v.type() != QVariant::String)
								{
									LOG_WARNING("Bad path");
									continue;
								}
								info.path = v.toString();

								switch (type)
								{
									case FileObjType::FileLink:
									case FileObjType::DirectoryLink:
									{
										v = Utils::ValueFrom(map, TARGET_KEY);
										if (v.type() != QVariant::String)
										{
											LOG_WARNING("Bad path");
											continue;
										}
										info.target = v.toString();
									}	break;

									default:
										break;
								}
							}	break;

							case QVariant::String:
								type = FileObjType::File;
								info.path = itValue.toString();
								break;

							default:
							{
								LOG_WARNING(QString("Unknown field type: %1").arg((int) itValue.type()));
								continue;
							}
						}

						switch (type)
						{
							case FileObjType::File:
							case FileObjType::FileLink:
							case FileObjType::Directory:
							case FileObjType::DirectoryLink:
								break;

							case FileObjType::None:
								continue;
						}

						infos[type].push_back(info);
					}

					auto it = infos.find(FileObjType::DirectoryLink);

					if (infos.end() != it)
					{
						for (auto &info : it->second)
						{
							projectDir->linkDirectory(
										rootDir->getAbsoluteFilePathFor(info.target),
										rootDir->getAbsoluteFilePathFor(info.path),
										false, false);
						}

						infos.erase(it);
					}

					for (auto &item : infos)
					{
						for (auto &info : item.second)
						{
							switch (item.first)
							{
								case FileObjType::FileLink:
									projectDir->linkFile(
												rootDir->getAbsoluteFilePathFor(info.target),
												rootDir->getAbsoluteFilePathFor(info.path),
												false, false);
									break;

								default:
									break;
							}
						}
					}
				}

				value = Utils::ValueFrom(input, SEARCH_PATHS_KEY);

				if (value.type() == QVariant::List)
				{
					auto list = value.toList();
					value.clear();

					auto searchPaths = getSearchPaths();

					int order = 0;
					for (auto &v : list)
					{
						if (v.type() != QVariant::String)
						{
							LOG_WARNING("Bad search path");
							continue;
						}

						auto path = rootDir->getAbsoluteFilePathFor(v.toString());
						if (nullptr == searchPaths->registerPath(path, order))
						{
							LOG_WARNING(QString("Unable to register search path: '%1'").arg(path));
							continue;
						}
						order++;
					}
				}
			}
		}

		endLoad();
		return ok;
	}

	void AbstractProjectFile::doUpdateFilePath(bool check_oldpath)
	{
		openedFiles = nullptr;
		auto projectDir = dynamic_cast<AbstractProjectDirectory *>(getTopDirectory());
		if (nullptr != projectDir)
		{
			openedFiles = projectDir->getOpenedFiles();
		}

		VariantMapFile::doUpdateFilePath(check_oldpath);
	}

	void AbstractProjectFile::watch(bool yes)
	{
		if (nullptr != openedFiles)
		{
			if (!canonicalPath.isEmpty())
				openedFiles->watchFile(this, yes);
		}
	}

	void AbstractProjectFile::saveProjectDirectory(Directory *root_dir,
												   Directory *dir,
												   QVariantList &output) const
	{
		Q_ASSERT(nullptr != root_dir);
		Q_ASSERT(nullptr != dir);

		for (auto child : dir->children())
		{
			dir = dynamic_cast<Directory *>(child);
			if (nullptr != dir)
			{
				if (nullptr == dynamic_cast<RootDirectory *>(child))
				{
					auto relative_path = dir->getFilePath(root_dir);
					QFileInfo info(dir->getFilePath());

					if (info.isSymLink())
					{
						QVariantMap map;
						map.insert(TYPE_KEY, TYPE_DIR_LINK);
						map.insert(PATH_KEY, relative_path);
						map.insert(TARGET_KEY, root_dir->getRelativeFilePathFor(info.symLinkTarget()));

						output.push_back(map);
					}
				}

				saveProjectDirectory(root_dir, dir, output);
			} else
			{
				auto file = dynamic_cast<AbstractFile *>(child);
				if (nullptr != file && file != this)
				{
					auto relative_filepath = file->getFilePath(root_dir);
					if (file->isSymLink())
					{
						QVariantMap map;
						map.insert(TYPE_KEY, TYPE_FILE_LINK);
						map.insert(PATH_KEY, relative_filepath);
						map.insert(TARGET_KEY, root_dir->getRelativeFilePathFor(file->getSymLinkTarget()));

						output.push_back(map);
					}
				}
			}
		}
	}

	void AbstractProjectFile::doSetHideIgnoredFiles(bool value)
	{
		auto oldValue = mHideIgnoredFiles;
		mHideIgnoredFiles = value;
		PUSH_UNDO_COMMAND(HideIgnoredFiles, oldValue);
		emit changedHideIgnoredFiles();
		setModified(true);
	}

	void AbstractProjectFile::resetHideIgnoredFiles()
	{
		setHideIgnoredFiles(true);
	}

	void AbstractProjectFile::doSetIgnoredFilesPattern(const QString &value)
	{
		auto oldValue = mIgnoredFilesPattern;
		mIgnoredFilesPattern = value;
		PUSH_UNDO_COMMAND(IgnoredFilesPattern, oldValue);
		emit changedIgnoredFilesPattern();
		setModified(true);
	}

	void AbstractProjectFile::resetIgnoredFilesPattern()
	{
		setIgnoredFilesPattern(QString());
	}

}
