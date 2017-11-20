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

#include "AbstractDirectory.h"

#include "Utils.h"
#include "AbstractFile.h"

#include <QDir>

namespace Banana
{
AbstractDirectory::AbstractDirectory(QObject *thiz)
	: AbstractFileSystemObject(thiz)
	, userSpecific(false)
{
}

QString AbstractDirectory::getAbsoluteFilePathFor(const QString &path) const
{
	if (QDir::isAbsolutePath(path))
		return QDir::cleanPath(path);

	return QDir::cleanPath(QDir(getFilePath()).absoluteFilePath(path));
}

QString AbstractDirectory::getRelativeFilePathFor(const QString &path) const
{
	if (QDir::isRelativePath(path))
		return path;

	return QDir(getFilePath()).relativeFilePath(path);
}

const QString &AbstractDirectory::getFileExtension() const
{
	static const QString dummy;
	return dummy;
}

QString AbstractDirectory::getFileName() const
{
	return thiz->objectName();
}

AbstractFileSystemObject *AbstractDirectory::findFileSystemObject(
	const QString &path, bool linked)
{
	AbstractFileSystemObject *result = nullptr;
	if (!path.isEmpty())
	{
		auto top_dir = getTopDirectory();
		if (nullptr != top_dir)
		{
			auto absolute_path = getAbsoluteFilePathFor(path);
			result = top_dir->internalFind(absolute_path, false);

			if (nullptr == result && linked)
			{
				QFileInfo info(absolute_path);
				if (info.exists())
				{
					result =
						top_dir->internalFind(info.canonicalFilePath(), true);
				}
			}
		}
	}

	return result;
}

bool AbstractDirectory::isUserSpecific() const
{
	return userSpecific;
}

void AbstractDirectory::setUserSpecific(bool yes)
{
	userSpecific = yes;
}

void AbstractDirectory::descendantChanged(
	QObject *descendant, DescendantState state)
{
	auto parent = getParentDirectory();
	if (nullptr != parent)
		parent->descendantChanged(descendant, state);
}

AbstractFileSystemObject *AbstractDirectory::internalFind(
	const QString &path, bool canonical)
{
	if (canonical)
	{
		if (0 == path.compare(getCanonicalFilePath(), Qt::CaseInsensitive))
		{
			return this;
		}
	} else
	{
		if (0 == path.compare(getFilePath(), Qt::CaseInsensitive))
		{
			return this;
		}
	}

	for (auto child : thiz->children())
	{
		auto file = dynamic_cast<AbstractFile *>(child);
		if (nullptr != file)
		{
			if (canonical)
			{
				if (path.compare(
						file->getCanonicalFilePath(), Qt::CaseInsensitive) == 0)
				{
					return file;
				}
			} else
			{
				if (0 == path.compare(file->savedPath, Qt::CaseInsensitive))
				{
					return file;
				}
			}
		}
	}

	for (auto child : thiz->children())
	{
		auto dir = dynamic_cast<AbstractDirectory *>(child);
		if (nullptr != dir)
		{
			auto result = dir->internalFind(path, canonical);
			if (nullptr != result)
				return result;
		}
	}

	return nullptr;
}
}
