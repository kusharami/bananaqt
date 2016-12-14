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

#include "AbstractNameCollection.h"

#include <QDir>

namespace Banana
{

	class FileNameCollection : public AbstractNameCollection
	{
	public:
		FileNameCollection();
		FileNameCollection(const QDir &dir, const QString &fileExtension);

		inline const QDir &dir();
		inline void setDir(const QDir &value);

		inline const QString &fileExtension();
		inline void setFileExtension(const QString &value);

		virtual bool containsName(const QString &name) const override;
		virtual bool isValid() const override;

	private:
		QDir mDir;
		QString mFileExtension;
	};

	const QDir &FileNameCollection::dir()
	{
		return mDir;
	}

	void FileNameCollection::setDir(const QDir &value)
	{
		mDir = value;
	}

	const QString &FileNameCollection::fileExtension()
	{
		return mFileExtension;
	}

	void FileNameCollection::setFileExtension(const QString &value)
	{
		mFileExtension = value;
	}

}
