/*******************************************************************************
Banana Qt Libraries

Copyright (c) 2017 Alexandra Cherdantseva

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

#include "ScriptableProjectFile.h"

#include "ScriptManager.h"

#include "BananaCore/Directory.h"
#include "BananaCore/Utils.h"

#include <QKeySequence>

namespace Banana
{
const QString ScriptableProjectFile::SCRIPTS_KEY =
	QStringLiteral("RegisterScripts");
const QString ScriptableProjectFile::CAPTION_KEY = QStringLiteral("Caption");
const QString ScriptableProjectFile::OBJECT_TYPE_KEY =
	QStringLiteral("ObjectType");
const QString ScriptableProjectFile::SHORTCUT_KEY = QStringLiteral("Shortcut");

ScriptableProjectFile::ScriptableProjectFile(
	const QString &name, const QString &extension)
	: AbstractProjectFile(name, extension)
	, mScriptManager(new ScriptManager(this))
{
	QObject::connect(
		mScriptManager, &ScriptManager::changed, this, &Object::modify);

	(void) QT_TRANSLATE_NOOP("Banana::ScriptableProjectFile", "mScriptManager");
}

ScriptableProjectFile::~ScriptableProjectFile()
{
	delete mScriptManager;
}

void ScriptableProjectFile::resetScriptManager()
{
	mScriptManager->clear();
}

void ScriptableProjectFile::saveData(QVariantMap &output)
{
	AbstractProjectFile::saveData(output);
	saveScriptEntries(output);
}

bool ScriptableProjectFile::loadData(const QVariantMap &input)
{
	beginLoad();
	bool ok = AbstractProjectFile::loadData(input) && loadScriptEntries(input);
	endLoad();
	return ok;
}

bool ScriptableProjectFile::loadScriptEntries(const QVariantMap &input)
{
	auto value = Utils::ValueFrom(input, SCRIPTS_KEY, QVariantList());

	if (value.type() != QVariant::List)
		return false;

	auto list = value.toList();
	value.clear();

	mScriptManager->clear();

	if (list.isEmpty())
		return true;

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

		auto vPath = Utils::ValueFrom(vmap, PATH_KEY, QString());
		if (vPath.type() != QVariant::String)
		{
			LOG_WARNING(QStringLiteral("Bad script path entry"));
			return false;
		}

		auto vShortcut = Utils::ValueFrom(vmap, SHORTCUT_KEY, QString());
		if (vShortcut.type() != QVariant::String)
		{
			LOG_WARNING(QStringLiteral("Bad script shortcut entry"));
			return false;
		}

		auto vType = Utils::ValueFrom(vmap, OBJECT_TYPE_KEY);

		QStringList types;

		switch (vType.type())
		{
			case QVariant::Invalid:
				break;

			case QVariant::String:
				types.append(vType.toString());
				break;

			case QVariant::List:
				for (const auto &item : vType.toList())
				{
					types.append(item.toString());
				}
				break;

			default:
				LOG_WARNING(QStringLiteral("Bad script object type entry"));
				return false;
		}

		ScriptCommand::MetaObjects metaObjects;

		for (const auto &typeStr : types)
		{
			auto metaObject = Utils::GetMetaObjectForClass(typeStr);

			if (nullptr == metaObject)
			{
				LOG_WARNING(
					QStringLiteral("Bad script object type: %1").arg(typeStr));
				return false;
			}
			metaObjects.insert(metaObject);
		}

		auto filePath = vPath.toString();
		if (not filePath.isEmpty())
			filePath = rootDir->getAbsoluteFilePathFor(filePath);

		mScriptManager->addScriptCommand(metaObjects, filePath,
			vCaption.toString(),
			QKeySequence::fromString(
				vShortcut.toString(), QKeySequence::PortableText));
	}

	return true;
}

void ScriptableProjectFile::saveScriptEntries(QVariantMap &output)
{
	QVariantList entries;

	auto &scriptEntries = mScriptManager->scriptEntries();
	auto rootDir = getParentDirectory();

	for (auto &entry : scriptEntries)
	{
		QVariantMap vmap;
		QVariantList objectTypes;
		for (auto metaObject : entry.metaObjects)
		{
			objectTypes.append(QLatin1String(metaObject->className()));
		}

		vmap.insert(OBJECT_TYPE_KEY, objectTypes);
		vmap.insert(PATH_KEY, rootDir->getRelativeFilePathFor(entry.filePath));
		vmap.insert(CAPTION_KEY, entry.caption);
		vmap.insert(
			SHORTCUT_KEY, entry.keySeq.toString(QKeySequence::PortableText));

		entries.append(vmap);
	}

	if (not entries.isEmpty())
	{
		output.insert(SCRIPTS_KEY, entries);
	}
}
}
