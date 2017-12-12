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

#include "ScriptManager.h"

#include "BananaCore/AbstractProjectFile.h"

#include <QFile>

namespace Banana
{
ScriptManager::ScriptManager(QObject *owner, QObject *parent)
	: QObject(parent)
	, mOwner(owner)
{
}

void ScriptManager::registerMetaObject(const QMetaObject *metaObject)
{
	if (metaObject)
	{
		metaObjectsMutable().insert(metaObject);
	}
}

void ScriptManager::addScriptCommand(const MetaObjects &metaObjects,
	const QString &filePath, const QString &caption, const QKeySequence &keySeq)
{
	mEntries.emplace_back(metaObjects, filePath, caption, keySeq);
	emit changed();
}

void ScriptManager::clear()
{
	if (not mEntries.empty())
	{
		mEntries.clear();

		emit changed();
	}
}

void ScriptManager::setScriptEntries(const Entries &entries)
{
	if (entries != mEntries)
	{
		mEntries = entries;

		emit changed();
	}
}

bool ScriptManager::hasActionsFor(const QObjectList &targets)
{
	for (const Entry &entry : mEntries)
	{
		if (entry.supportsTargets(targets))
			return true;
	}
	return false;
}

QString ScriptManager::scriptedActionsCaption()
{
	return tr("Scripted Actions");
}

QString ScriptManager::scriptedActionCaption()
{
	return tr("Scripted Action");
}

ScriptManager::MetaObjects &ScriptManager::metaObjectsMutable()
{
	static MetaObjects META_OBJECTS;
	return META_OBJECTS;
}
}
