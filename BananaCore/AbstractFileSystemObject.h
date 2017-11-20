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
#include <QStringList>

class QObject;

namespace Banana
{
class AbstractDirectory;
class AbstractFileSystemObject
{
public:
	AbstractFileSystemObject(QObject *thiz);
	virtual ~AbstractFileSystemObject();

	inline QObject *getObject() const;

	virtual bool isSymLink() const;
	virtual QString getSymLinkTarget() const;

	AbstractDirectory *getTopDirectory() const;
	AbstractDirectory *getParentDirectory() const;
	virtual QString getFilePath(
		const AbstractDirectory *relative_to = nullptr) const;
	virtual QString getCanonicalFilePath() const;
	virtual QString getFileName() const;
	QString getBaseName() const;

	virtual bool rename(const QString &new_name);
	virtual bool setFileName(const QString &value);

	virtual bool isUserSpecific() const = 0;
	virtual void setUserSpecific(bool user) = 0;
	virtual const QString &getFileExtension() const = 0;

	virtual QStringList getSupportedExtensions() const;

	virtual bool isWritableFormat(const QString &extension) const;

protected:
	virtual bool updateFileExtension(const QString &filename, QString *outExt);
	virtual void updateFileNameError(const QString &failedName);
	virtual void executeUpdateFilePathError(
		const QString &path, const QString &failed_path) = 0;

	AbstractFileSystemObject *internalFind(
		const QString &path, bool canonical) const;

	QObject *thiz;
};

QObject *AbstractFileSystemObject::getObject() const
{
	return thiz;
}
}
