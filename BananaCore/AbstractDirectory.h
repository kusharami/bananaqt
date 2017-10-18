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

#pragma once

#include "AbstractFileSystemObject.h"

#include <vector>

namespace Banana
{
enum class DescendantState
{
	Added,
	Removed,
	Renamed,
	SearchPathsChanged
};

class AbstractDirectory : public AbstractFileSystemObject
{
public:
	AbstractDirectory(QObject *thiz);

	QString getAbsoluteFilePathFor(const QString &path) const;
	QString getRelativeFilePathFor(const QString &path) const;

	virtual const QString &getFileExtension() const override;
	virtual QString getFileName() const override;

	AbstractFileSystemObject *findFileSystemObject(
		const QString &path, bool linked);

	template <typename T>
	inline T *findFileT(const QString &path, bool linked = false);

protected:
	friend class AbstractFileSystemObject;

	virtual void descendantChanged(QObject *descendant, DescendantState state);

private:
	AbstractFileSystemObject *internalFind(const QString &path, bool canonical);
};

template <typename T>
T *AbstractDirectory::findFileT(const QString &path, bool linked)
{
	return dynamic_cast<T *>(findFileSystemObject(path, linked));
}
}
