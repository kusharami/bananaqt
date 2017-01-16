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

#include <QString>

class QDir;
class QFileInfo;

namespace Banana
{

	struct AbstractNamingPolicy
	{
		virtual ~AbstractNamingPolicy();

		virtual QString getFixedName(const QString &source) const = 0;
		virtual QString getNumberSeparator() const = 0;

		QString getNameWithoutNumber(const QString &name, quint32 *number_ptr = nullptr) const;
	};

	class IdentifierNamingPolicy : public AbstractNamingPolicy
	{
	public:
		virtual QString getFixedName(const QString &source) const override;
		virtual QString getNumberSeparator() const override;
	};

	class FileNamingPolicy : public AbstractNamingPolicy
	{
	public:
		virtual QString getFixedName(const QString &source) const override;
		virtual QString getNumberSeparator() const override;

		QString getUniqueFilePath(const QDir &parent_dir, const QString &base_name, QString suffix) const;

		static QString uniqueFilePath(const QString &parent_path, const QString &base_name, const QString &suffix = QString());
		static QString uniqueFilePath(const QString &parent_path, const QFileInfo &info);
		static QString uniqueFilePath(const QDir &parent_dir, const QString &base_name, const QString &suffix = QString());
		static QString uniqueFilePath(const QDir &parent_dir, const QFileInfo &info);
	};
}
