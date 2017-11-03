#include "ScriptManager.h"

namespace Banana
{
ScriptManager::ScriptManager(QObject *parent)
	: QObject(parent)
{
}

void ScriptManager::registerScriptFor(const QMetaObject *metaObject,
	const QString &filePath, const QString &caption)
{
	Entry entry;

	entry.metaObject = metaObject;
	entry.filePath = filePath;
	entry.caption = caption;

	mRegisteredScripts.push_back(entry);
}

void ScriptManager::clear()
{
	mRegisteredScripts.clear();
}
}
