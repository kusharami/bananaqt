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

#include "ScriptCommand.h"

namespace Banana
{
ScriptCommand::ScriptCommand() {}

ScriptCommand::ScriptCommand(const MetaObjects &metaObjects,
	const QString &filePath, const QString &caption, const QKeySequence &keySeq)
	: metaObjects(metaObjects)
	, filePath(filePath)
	, caption(caption)
	, keySeq(keySeq)
{
}

bool ScriptCommand::isValid() const
{
	return not caption.isEmpty() && not filePath.isEmpty();
}

bool ScriptCommand::supportsTargets(const QObjectList &targets) const
{
	if (metaObjects.empty() || not isValid())
		return false;

	for (auto target : targets)
	{
		for (auto metaObject : metaObjects)
		{
			Q_ASSERT(nullptr != metaObject);
			if (metaObject->cast(target))
				return true;
		}
	}

	return false;
}

bool ScriptCommand::operator==(const ScriptCommand &other) const
{
	return metaObjects == other.metaObjects && filePath == other.filePath &&
		caption == other.caption && keySeq == other.keySeq;
}

ScriptCommand &ScriptCommand::operator=(const ScriptCommand &other)
{
	metaObjects = other.metaObjects;
	filePath = other.filePath;
	caption = other.caption;
	keySeq = other.keySeq;

	return *this;
}
}
