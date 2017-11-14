/*******************************************************************************
Banana Qt Libraries

Copyright (c) 2016-2017 Alexandra Cherdantseva

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
#include "ScriptManager.h"
#include "Config.h"
#include "ProjectGroup.h"
#include "IProjectGroupDelegate.h"

#include <QJsonDocument>
#include <QFileInfo>
#include <QDir>

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
const QString AbstractProjectFile::SCRIPTS_KEY =
	QStringLiteral("RegisterScripts");
const QString AbstractProjectFile::CAPTION_KEY = QStringLiteral("Caption");
const QString AbstractProjectFile::OBJECT_TYPE_KEY =
	QStringLiteral("ObjectType");

struct AbstractProjectFile::FileInfo
{
	QString path;
	QString target;
	bool userSpecific = false;
};

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

bool AbstractProjectFile::finalizeFileInfo(
	FileObjType type, FileInfo &info, const QVariantMap &map)
{
	switch (type)
	{
		case FileObjType::FileLink:
		case FileObjType::DirectoryLink:
		{
			if (info.userSpecific)
			{
				bool dir = (type == FileObjType::DirectoryLink);
				if (not fetchUserSpecificPath(dir, info.path, info.target))
				{
					return false;
				}
			} else
			{
				auto v = Utils::ValueFrom(map, TARGET_KEY);
				if (v.type() != QVariant::String)
				{
					LOG_WARNING(QStringLiteral("Bad path"));
					return false;
				}
				info.target = v.toString();
			}
			break;
		}

		case FileObjType::File:
		case FileObjType::Directory:
			break;

		default:
			return false;
	}

	return true;
}

AbstractProjectFile::AbstractProjectFile(
	const QString &name, const QString &extension)
	: VariantMapFile(extension)
	, openedFiles(nullptr)
	, searchPaths(nullptr)
	, scriptManager(nullptr)
{
	setObjectName(name);

	(void) QT_TRANSLATE_NOOP("ClassName", "Banana::AbstractProjectFile");
	(void) QT_TRANSLATE_NOOP(
		"Banana::AbstractProjectFile", "mHideIgnoredFiles");
	(void) QT_TRANSLATE_NOOP(
		"Banana::AbstractProjectFile", "mIgnoredFilesPattern");
	(void) QT_TRANSLATE_NOOP("Banana::AbstractProjectFile", "mSearchPaths");
	(void) QT_TRANSLATE_NOOP("Banana::AbstractProjectFile", "mScriptManager");
}

AbstractProjectFile::~AbstractProjectFile()
{
	watch(false);
	delete searchPaths;
	delete scriptManager;
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

ScriptManager *AbstractProjectFile::getScriptManager()
{
	if (scriptManager == nullptr)
	{
		scriptManager = new ScriptManager(this);
	}

	return scriptManager;
}

void AbstractProjectFile::resetScriptManager()
{
	getScriptManager()->clear();
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

	auto projectDir =
		dynamic_cast<AbstractProjectDirectory *>(getTopDirectory());
	if (nullptr == projectDir)
		return false;

	auto projectGroup = projectDir->getProjectGroup();
	if (nullptr == projectGroup)
		return false;

	auto delegate = projectGroup->getDelegate();

	QFileInfo fileInfo(out);

	if (dir)
	{
		if (fileInfo.isDir())
			return true;

		if (nullptr == delegate)
			return false;

		out = delegate->fetchDirPath(
			tr("Select directory for '%1'")
				.arg(QDir::toNativeSeparators(relativePath)),
			out.isEmpty() ? QFileInfo(getFilePath()).path() : out);
	} else
	{
		if (fileInfo.isFile())
			return true;

		if (nullptr == delegate)
			return false;

		out = delegate->fetchFilePath(
			tr("Select file for '%1'")
				.arg(QDir::toNativeSeparators(relativePath)),
			out.isEmpty() ? QFileInfo(getFilePath()).path() : out,
			Directory::getFilterForExtension(""));
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

void AbstractProjectFile::saveFileEntries(QVariantMap &output)
{
	QVariantList entries;

	saveProjectDirectory(getParentDirectory(), getTopDirectory(), entries);

	if (not entries.isEmpty())
	{
		output.insert(FILES_KEY, entries);
	}
}

bool AbstractProjectFile::loadIgnoredFileEntries(const QVariantMap &input)
{
	auto value = Utils::ValueFrom(input, IGNORED_FILES_KEY, QVariantList());

	if (value.type() != QVariant::List)
		return false;

	auto list = value.toList();
	value.clear();

	QStringList ignored;

	for (const auto &pattern : list)
	{
		if (pattern.type() != QVariant::String)
		{
			LOG_WARNING(QStringLiteral("Bad ignored file entry"));
			return false;
		}

		ignored.push_back(pattern.toString());
	}

	setIgnoredFilesList(ignored);

	return true;
}

void AbstractProjectFile::saveIgnoredFileEntries(QVariantMap &output)
{
	QVariantList entries;

	auto ignored = getIgnoredFilesList();

	for (const auto &pattern : ignored)
	{
		entries.push_back(pattern);
	}

	if (not entries.isEmpty())
	{
		output.insert(IGNORED_FILES_KEY, entries);
	}
}

void AbstractProjectFile::saveSearchPathEntries(QVariantMap &output)
{
	QVariantList entries;

	auto dirs = getSearchPaths()->getDirectoryList();

	auto rootDir = getParentDirectory();

	for (auto dir : dirs)
	{
		entries.push_back(dir->getFilePath(rootDir));
	}

	if (not entries.isEmpty())
	{
		output.insert(SEARCH_PATHS_KEY, entries);
	}
}

bool AbstractProjectFile::loadScriptEntries(const QVariantMap &input)
{
	auto value = Utils::ValueFrom(input, SCRIPTS_KEY, QVariantList());

	if (value.type() != QVariant::List)
		return false;

	auto list = value.toList();
	value.clear();

	if (list.isEmpty() && nullptr == scriptManager)
		return true;

	auto scriptManager = getScriptManager();
	auto rootDir = getParentDirectory();

	for (const auto &value : list)
	{
		if (value.type() != QVariant::Map)
		{
			LOG_WARNING(QStringLiteral("Bad script entry"));
			return false;
		}

		auto vmap = value.toMap();

		auto vCaption = Utils::ValueFrom(vmap, CAPTION_KEY, QString());

		if (vCaption.type() != QVariant::String)
		{
			LOG_WARNING(QStringLiteral("Bad script caption entry"));
			return false;
		}

		auto vPath = Utils::ValueFrom(vmap, PATH_KEY);

		QString pathStr;

		if (vPath.type() == QVariant::String)
			pathStr = vPath.toString();

		if (pathStr.isEmpty())
		{
			LOG_WARNING(QStringLiteral("Bad script path entry"));
			return false;
		}

		auto vType = Utils::ValueFrom(vmap, OBJECT_TYPE_KEY);

		QString typeStr;

		if (vType.type() == QVariant::String)
			typeStr = vType.toString();

		auto metaObject = Utils::GetMetaObjectForClass(typeStr);

		if (nullptr == metaObject)
		{
			LOG_WARNING(QStringLiteral("Bad script object type entry"));
			return false;
		}

		auto filePath = rootDir->getAbsoluteFilePathFor(pathStr);

		scriptManager->registerScriptFor(
			metaObject, filePath, vCaption.toString());
	}

	return true;
}

void AbstractProjectFile::saveScriptEntries(QVariantMap &output)
{
	if (scriptManager == nullptr)
		return;

	QVariantList entries;

	auto &scriptEntries = getScriptManager()->scriptEntries();
	auto rootDir = getParentDirectory();

	for (auto &entry : scriptEntries)
	{
		QVariantMap vmap;

		vmap.insert(OBJECT_TYPE_KEY, entry.metaObject->className());
		vmap.insert(PATH_KEY, rootDir->getRelativeFilePathFor(entry.filePath));
		vmap.insert(CAPTION_KEY, entry.caption);

		entries.append(vmap);
	}

	if (not entries.isEmpty())
	{
		output.insert(SCRIPTS_KEY, entries);
	}
}

void AbstractProjectFile::saveData(QVariantMap &output)
{
	VariantMapFile::saveData(output);

	saveFileEntries(output);
	saveIgnoredFileEntries(output);
	saveSearchPathEntries(output);
	saveScriptEntries(output);
	saveUserSettings();
}

bool AbstractProjectFile::loadFileEntries(
	AbstractProjectDirectory *projectDir, const QVariantMap &input)
{
	auto value = Utils::ValueFrom(input, FILES_KEY, QVariantList());

	if (value.type() != QVariant::List)
		return false;

	auto list = value.toList();
	value.clear();

	std::map<FileObjType, std::vector<FileInfo>> infos;

	for (const auto &itValue : list)
	{
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
					return false;
				}
				type = getFileObjTypeFromString(v.toString());

				v = Utils::ValueFrom(map, PATH_KEY);
				if (v.type() != QVariant::String)
				{
					LOG_WARNING("Bad path");
					return false;
				}
				info.path = v.toString();

				v = Utils::ValueFrom(map, USER_SPECIFIC_KEY, false);
				if (v.type() != QVariant::Bool)
				{
					LOG_WARNING("Bad user specific");
					return false;
				}

				info.userSpecific = v.toBool();

				if (not finalizeFileInfo(type, info, map))
					return false;

				break;
			}

			case QVariant::String:
				type = FileObjType::File;
				info.path = itValue.toString();
				break;

			default:
			{
				LOG_WARNING(QStringLiteral("Bad field type"));
				return false;
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
				return false;
		}

		infos[type].push_back(info);
	}

	auto rootDir = getParentDirectory();
	auto it = infos.find(FileObjType::DirectoryLink);

	if (infos.end() != it)
	{
		for (const auto &info : it->second)
		{
			auto dir = projectDir->linkDirectory(
				rootDir->getAbsoluteFilePathFor(info.target),
				rootDir->getAbsoluteFilePathFor(info.path), false, false);

			if (dir)
			{
				dir->setUserSpecific(info.userSpecific);
			}
		}

		infos.erase(it);
	}

	for (const auto &item : infos)
	{
		for (const auto &info : item.second)
		{
			switch (item.first)
			{
				case FileObjType::FileLink:
				{
					auto file = projectDir->linkFile(
						rootDir->getAbsoluteFilePathFor(info.target),
						rootDir->getAbsoluteFilePathFor(info.path), false,
						false);

					if (file)
					{
						file->setUserSpecific(info.userSpecific);
					}
					break;
				}

				default:
					break;
			}
		}
	}

	return true;
}

bool AbstractProjectFile::loadSearchPathEntries(const QVariantMap &input)
{
	auto value = Utils::ValueFrom(input, SEARCH_PATHS_KEY, QVariantList());

	if (value.type() != QVariant::List)
		return false;

	auto list = value.toList();
	value.clear();

	auto searchPaths = getSearchPaths();

	auto rootDir = getParentDirectory();

	int order = 0;
	for (const auto &v : list)
	{
		if (v.type() != QVariant::String)
		{
			LOG_WARNING(QStringLiteral("Bad search path"));
			return false;
		}

		auto path = rootDir->getAbsoluteFilePathFor(v.toString());
		if (nullptr == searchPaths->registerPath(path, order))
		{
			LOG_WARNING(QStringLiteral("Unable to register search path: '%1'")
							.arg(path));
			continue;
		}
		order++;
	}
	return true;
}

bool AbstractProjectFile::loadData(const QVariantMap &input)
{
	bool ok = false;
	beginLoad();
	auto projectDir =
		dynamic_cast<AbstractProjectDirectory *>(getTopDirectory());

	if (nullptr != projectDir && projectDir->getProjectFile() == this &&
		projectDir == getParentDirectory())
	{
		auto siblings = projectDir->children();
		for (auto sibling : siblings)
		{
			if (sibling != this)
				delete sibling;
		}
		siblings.clear();

		delete scriptManager;
		scriptManager = nullptr;

		if (VariantMapFile::loadData(input))
		{
			loadUserSettings();

			ok = loadIgnoredFileEntries(input) && //
				loadFileEntries(projectDir, input) && //
				loadSearchPathEntries(input) && //
				loadScriptEntries(input);
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
				if (file->isSymLink() && file->isUserSpecific())
				{
					QVariantMap map;
					map.insert(TYPE_KEY, TYPE_FILE_LINK);
					map.insert(PATH_KEY, relative_filepath);

					map.insert(USER_SPECIFIC_KEY, true);

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
