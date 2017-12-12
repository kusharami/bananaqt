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

#pragma once

#include "ScriptCommand.h"

#include <vector>

namespace Banana
{
class ScriptRunner;

class ScriptManager : public QObject
{
	Q_OBJECT

public:
	using MetaObjects = ScriptCommand::MetaObjects;
	using Entry = ScriptCommand;

	using Entries = std::vector<Entry>;

	explicit ScriptManager(QObject *owner, QObject *parent = nullptr);

	inline QObject *owner() const;

	static inline const MetaObjects &metaObjects();
	static void registerMetaObject(const QMetaObject *metaObject);

	Q_INVOKABLE void addScriptCommand(const MetaObjects &metaObjects,
		const QString &filePath, const QString &caption,
		const QKeySequence &keySeq);
	Q_INVOKABLE void clear();

	inline const Entries &scriptEntries() const;
	void setScriptEntries(const Entries &entries);

	bool hasActionsFor(const QObjectList &targets);

	static QString scriptedActionsCaption();
	static QString scriptedActionCaption();

signals:
	void changed();

private:
	static MetaObjects &metaObjectsMutable();

	QObject *mOwner;
	Entries mEntries;
};

QObject *ScriptManager::owner() const
{
	return mOwner;
}

const ScriptManager::MetaObjects &ScriptManager::metaObjects()
{
	return metaObjectsMutable();
}

const ScriptManager::Entries &ScriptManager::scriptEntries() const
{
	return mEntries;
}
}

Q_DECLARE_METATYPE(Banana::ScriptManager *)
