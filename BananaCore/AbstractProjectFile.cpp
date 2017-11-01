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

#include "AbstractProjectFile.h"

#include "AbstractProjectDirectory.h"
#include "Utils.h"
#include "OpenedFiles.h"
#include "Directory.h"
#include "SearchPaths.h"
#include "Config.h"

#include <QJsonDocument>
#include <QFileInfo>
#include <QDir>
#include <QFileDialog>

namespace Banana
{
const QString AbstractProjectFile::SEARCH_PATHS_KEY =
	QStringLiteral("SearchPaths");
const QString AbstractProjectFile::IGNORED_FILES_KEY =
	QStringLiteral("IgnoredFiles");
const QString AbstractProjectFile::FILES_KEY = QStringLiteral("Files");
const QString AbstractProjectFile::TYPE_KEY = QStringLiteral("Type");
const QString AbstractProjectFile::PATH_KEY = QStringLiteral("Path");
const QString AbstractProjectFile::TARGET_KEY = QStringLiteral("Target");
const QString AbstractProjectFile::TYPE_FILE = QStringLiteral("File");
const QString AbstractProjectFile::TYPE_DIR = QStringLiteral("Directory");
const QString AbstractProjectFile::TYPE_DIR_LINK =
	QStringLiteral("DirectoryLink");
const QString AbstractProjectFile::TYPE_FILE_LINK = QStringLiteral("FileLink");
const QString AbstractProjectFile::USER_SPECIFIC_KEY =
	QStringLiteral("UserSpecific");
const QString AbstractProjectFile::USER_PATHS_KEY = QStringLiteral("UserPaths");

AbstractProjectFile::FileObjType AbstractProjectFile::getFileObjTypeFromString(
	const QString &str)
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

AbstractProjectFile::AbstractProjectFile(
	const QString &name, const QString &extension)
	: VariantMapFile(extension)
	, openedFiles(nullptr)
	, searchPaths(nullptr)
{
	setObjectName(name);

	(void) QT_TRANSLATE_NOOP("ClassName", "Banana::AbstractProjectFile");
	(void) QT_TRANSLATE_NOOP(
		"Banana::AbstractProjectFile", "mHideIgnoredFiles");
	(void) QT_TRANSLATE_NOOP(
		"Banana::AbstractProjectFile", "mIgnoredFilesPattern");
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
	return mIgnoredFilesPattern.split(
		QRegExp("[\n\r]+|\\|"), QString::SkipEmptyParts);
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
		auto projectDir =
			dynamic_cast<AbstractProjectDirectory *>(getTopDirectory());
		if (nullptr != projectDir)
			searchPaths = new SearchPaths(projectDir);
	}

	return searchPaths;
}

void AbstractProjectFile::resetSearchPaths()
{
	getSearchPaths()->clear();
}

QString AbstractProjectFile::getAbsoluteFilePathFor(
	const QString &filepath) const
{
	auto root_dir = dynamic_cast<Directory *>(parent());
	if (nullptr != root_dir)
		return root_dir->getAbsoluteFilePathFor(filepath);

	return QString();
}

QString AbstractProjectFile::getUserSettingsPath() const
{
	return getFilePath() + ".user";
}

void AbstractProjectFile::setUserSpecificPathFor(
	AbstractFileSystemObject *file, bool user)
{
	Q_ASSERT(nullptr != file);
	Q_ASSERT(file->isSymLink());

	auto relativePath = file->getFilePath(getParentDirectory());

	if (user)
		userPaths.insert(relativePath, file->getSymLinkTarget());
	else
		userPaths.remove(relativePath);

	file->setUserSpecific(user);

	setModified(true);
}

void AbstractProjectFile::loadUserSettings()
{
	QJsonDocument doc;

	userPaths = QJsonObject();

	if (Utils::LoadJsonFromFile(doc, getUserSettingsPath()))
	{
		if (doc.isObject())
		{
			auto userSettings = doc.object();

			auto val = userSettings.value(USER_PATHS_KEY);

			if (val.isObject())
			{
				userPaths = val.toObject();
			}
		}
	}
}

void AbstractProjectFile::saveUserSettings()
{
	QJsonObject userSettings;

	userSettings.insert(USER_PATHS_KEY, userPaths);

	QJsonDocument doc;
	doc.setObject(userSettings);

	Utils::SaveJsonToFile(doc, getUserSettingsPath());
}

bool AbstractProjectFile::fetchUserSpecificPath(
	bool dir, const QString &relativePath, QString &out)
{
	auto val = userPaths.value(relativePath);
	out = val.isString() ? val.toString() : QString();

	QFileInfo fileInfo(out);

	if (dir)
	{
		if (fileInfo.isDir())
			return true;

		out = QFileDialog::getExistingDirectory(nullptr,
			tr("Select directory for '%1'")
				.arg(QDir::toNativeSeparators(relativePath)),
			out.isEmpty() ? QFileInfo(getFilePath()).path() : out,
			QFileDialog::ShowDirsOnly | FILE_DIALOG_FLAGS |
				QFileDialog::DontResolveSymlinks);

	} else
	{
		if (fileInfo.isFile())
			return true;

		out = QFileDialog::getOpenFileName(nullptr,
			tr("Select file for '%1'").arg(relativePath),
			out.isEmpty() ? QFileInfo(getFilePath()).path() : out,
			Directory::getFilterForExtension(""), nullptr,
			QFileDialog::DontResolveSymlinks | FILE_DIALOG_FLAGS);
	}

	if (not out.isEmpty())
	{
		userPaths.insert(relativePath, out);
		return true;
	}

	return false;
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
	saveProjectDirectory(root_dir, getTopDirectory(), paths);

	output.insert(FILES_KEY, paths);

	// ----
	paths.clear();

	auto ignored = getIgnoredFilesList();

	for (auto &pattern : ignored)
	{
		paths.push_back(pattern);
	}

	output.insert(IGNORED_FILES_KEY, paths);

	// ----
	paths.clear();

	auto dirs = getSearchPaths()->getDirectoryList();

	for (auto dir : dirs)
	{
		paths.push_back(dir->getFilePath(root_dir));
	}

	output.insert(SEARCH_PATHS_KEY, paths);

	saveUserSettings();
}

bool AbstractProjectFile::loadData(const QVariantMap &input)
{
	bool ok = false;
	beginLoad();
	auto projectDir =
		dynamic_cast<AbstractProjectDirectory *>(getTopDirectory());
	auto rootDir = dynamic_cast<Directory *>(parent());

	if (nullptr != projectDir && projectDir->getProjectFile() == this &&
		projectDir == rootDir)
	{
		auto siblings = projectDir->children();
		for (auto sibling : siblings)
		{
			if (sibling != this)
				delete sibling;
		}
		siblings.clear();

		if (VariantMapFile::loadData(input))
		{
			loadUserSettings();

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
					bool userSpecific = false;
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

							v = Utils::ValueFrom(map, USER_SPECIFIC_KEY, false);
							if (v.type() != QVariant::Bool)
							{
								LOG_WARNING("Bad user specific");
								continue;
							}

							info.userSpecific = v.toBool();

							switch (type)
							{
								case FileObjType::FileLink:
								case FileObjType::DirectoryLink:
								{
									if (info.userSpecific)
									{
										if (not fetchUserSpecificPath(type ==
													FileObjType::DirectoryLink,
												info.path, info.target))
										{
											ok = false;
											break;
										}
									} else
									{
										v = Utils::ValueFrom(map, TARGET_KEY);
										if (v.type() != QVariant::String)
										{
											LOG_WARNING("Bad path");
											continue;
										}
										info.target = v.toString();
									}
									break;
								}

								default:
									break;
							}

							break;
						}

						case QVariant::String:
							type = FileObjType::File;
							info.path = itValue.toString();
							break;

						default:
						{
							LOG_WARNING(QStringLiteral("Unknown field type: %1")
											.arg((int) itValue.type()));
							continue;
						}
					}

					if (not ok)
					{
						break;
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

				if (ok)
				{
					auto it = infos.find(FileObjType::DirectoryLink);

					if (infos.end() != it)
					{
						for (auto &info : it->second)
						{
							auto dir = projectDir->linkDirectory(
								rootDir->getAbsoluteFilePathFor(info.target),
								rootDir->getAbsoluteFilePathFor(info.path),
								false, false);

							if (dir)
							{
								dir->setUserSpecific(info.userSpecific);
							}
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
								{
									auto file = projectDir->linkFile(
										rootDir->getAbsoluteFilePathFor(
											info.target),
										rootDir->getAbsoluteFilePathFor(
											info.path),
										false, false);

									if (file)
									{
										file->setUserSpecific(
											info.userSpecific);
									}
									break;
								}

								default:
									break;
							}
						}
					}
				}
			}

			if (ok)
			{
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
							LOG_WARNING(QStringLiteral("Bad search path"));
							continue;
						}

						auto path =
							rootDir->getAbsoluteFilePathFor(v.toString());
						if (nullptr == searchPaths->registerPath(path, order))
						{
							LOG_WARNING(QStringLiteral(
								"Unable to register search path: '%1'")
											.arg(path));
							continue;
						}
						order++;
					}
				}
			}
		}
	}

	endLoad();

	if (ok)
	{
		saveUserSettings();
	}

	return ok;
}

void AbstractProjectFile::doUpdateFilePath(bool checkOldPath)
{
	openedFiles = nullptr;
	auto projectDir =
		dynamic_cast<AbstractProjectDirectory *>(getTopDirectory());
	if (nullptr != projectDir)
	{
		openedFiles = projectDir->getOpenedFiles();
	}

	VariantMapFile::doUpdateFilePath(checkOldPath);
}

void AbstractProjectFile::watch(bool yes)
{
	if (nullptr != openedFiles)
	{
		if (!canonicalPath.isEmpty())
			openedFiles->watchFile(this, yes);
	}
}

void AbstractProjectFile::saveProjectDirectory(
	Directory *rootDir, Directory *dir, QVariantList &output) const
{
	Q_ASSERT(nullptr != rootDir);
	Q_ASSERT(nullptr != dir);

	for (auto child : dir->children())
	{
		dir = dynamic_cast<Directory *>(child);
		if (nullptr != dir)
		{
			if (nullptr == dynamic_cast<RootDirectory *>(child))
			{
				auto relative_path = dir->getFilePath(rootDir);
				QFileInfo info(dir->getFilePath());

				if (info.isSymLink())
				{
					QVariantMap map;
					map.insert(TYPE_KEY, TYPE_DIR_LINK);
					map.insert(PATH_KEY, relative_path);

					if (dir->isUserSpecific())
					{
						map.insert(USER_SPECIFIC_KEY, true);
					} else
					{
						map.insert(TARGET_KEY,
							rootDir->getRelativeFilePathFor(
								info.symLinkTarget()));
					}

					output.push_back(map);
				}
			}

			saveProjectDirectory(rootDir, dir, output);
		} else
		{
			auto file = dynamic_cast<AbstractFile *>(child);
			if (nullptr != file && file != this)
			{
				auto relative_filepath = file->getFilePath(rootDir);
				if (file->isSymLink())
				{
					QVariantMap map;
					map.insert(TYPE_KEY, TYPE_FILE_LINK);
					map.insert(PATH_KEY, relative_filepath);

					if (file->isUserSpecific())
					{
						map.insert(USER_SPECIFIC_KEY, true);
					} else
					{
						map.insert(TARGET_KEY,
							rootDir->getRelativeFilePathFor(
								file->getSymLinkTarget()));
					}

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
