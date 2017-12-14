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

#include <QObject>
#include <QKeySequence>

#include <unordered_set>

namespace Banana
{
struct ScriptCommand
{
	using MetaObjects = std::unordered_set<const QMetaObject *>;

	MetaObjects metaObjects;
	QString filePath;
	QString caption;
	QKeySequence keySeq;

	ScriptCommand();
	ScriptCommand(const MetaObjects &metaObjects, const QString &filePath,
		const QString &caption, const QKeySequence &keySeq);
	bool isValid() const;

	bool supportsTargets(const QObjectList &targets) const;

	bool operator==(const ScriptCommand &other) const;
	inline bool operator!=(const ScriptCommand &other) const;
	ScriptCommand &operator=(const ScriptCommand &other);
};

bool ScriptCommand::operator!=(const ScriptCommand &other) const
{
	return not operator==(other);
}
}
