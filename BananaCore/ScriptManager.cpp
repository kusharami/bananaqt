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

#include "AbstractProjectFile.h"

namespace Banana
{
ScriptManager::ScriptManager(AbstractProjectFile *owner, QObject *parent)
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

void ScriptManager::registerScriptFor(const QMetaObject *metaObject,
	const QString &filePath, const QString &caption)
{
	Entry entry;

	entry.metaObject = metaObject;
	entry.filePath = filePath;
	entry.caption = caption;

	mRegisteredScripts.push_back(entry);
	mOwner->setModified(true);
}

void ScriptManager::clear()
{
	if (!mRegisteredScripts.isEmpty())
	{
		mRegisteredScripts.clear();

		mOwner->setModified(true);
	}
}

void ScriptManager::setScriptEntries(const Entries &entries)
{
	if (entries != mRegisteredScripts)
	{
		mRegisteredScripts = entries;

		mOwner->setModified(true);
	}
}

ScriptManager::MetaObjects &ScriptManager::metaObjectsMutable()
{
	static MetaObjects META_OBJECTS;
	return META_OBJECTS;
}

ScriptManager::Entry::Entry()
	: metaObject(&Object::staticMetaObject)
{
}

bool ScriptManager::Entry::operator==(const Entry &other) const
{
	return metaObject == other.metaObject && filePath == other.filePath &&
		caption == other.caption;
}

ScriptManager::Entry &ScriptManager::Entry::operator=(const Entry &other)
{
	metaObject = other.metaObject;
	filePath = other.filePath;
	caption = other.caption;

	return *this;
}
}
