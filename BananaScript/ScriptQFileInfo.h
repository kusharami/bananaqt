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

#include "ScriptQFileDevice.h"
#include "ScriptQDir.h"

#include <QObject>
#include <QFileInfo>
#include <QScriptValue>
#include <QScriptable>
#include <QDateTime>

Q_DECLARE_METATYPE(QFileInfo *)

namespace Banana
{
class ScriptQFileInfo final
	: public QObject
	, public QScriptable
{
	Q_OBJECT

	Q_PROPERTY(QString filePath READ filePath WRITE setFilePath)
	Q_PROPERTY(QString absoluteFilePath READ absoluteFilePath)
	Q_PROPERTY(QString canonicalFilePath READ canonicalFilePath)
	Q_PROPERTY(QString fileName READ fileName)
	Q_PROPERTY(QString baseName READ baseName)
	Q_PROPERTY(QString completeBaseName READ completeBaseName)
	Q_PROPERTY(QString suffix READ suffix)
	Q_PROPERTY(QString bundleName READ bundleName)
	Q_PROPERTY(QString completeSuffix READ completeSuffix)
	Q_PROPERTY(QString symLinkTarget READ symLinkTarget)

	Q_PROPERTY(QString path READ path)
	Q_PROPERTY(QString absolutePath READ absolutePath)
	Q_PROPERTY(QString canonicalPath READ canonicalPath)

	Q_PROPERTY(QDir dir READ dir)
	Q_PROPERTY(QDir absoluteDir READ absoluteDir)

	Q_PROPERTY(bool readable READ isReadable)
	Q_PROPERTY(bool writable READ isWritable)
	Q_PROPERTY(bool executable READ isExecutable)
	Q_PROPERTY(bool hidden READ isHidden)

	Q_PROPERTY(bool relative READ isRelative)
	Q_PROPERTY(bool absolute READ isAbsolute)

	Q_PROPERTY(bool isNativePath READ isNativePath)
	Q_PROPERTY(bool isFile READ isFile)
	Q_PROPERTY(bool isDir READ isDir)
	Q_PROPERTY(bool isSymLink READ isSymLink)
	Q_PROPERTY(bool isRoot READ isRoot)
	Q_PROPERTY(bool isBundle READ isBundle)

	Q_PROPERTY(bool caching READ caching WRITE setCaching)

	Q_PROPERTY(qint64 size READ size)
	Q_PROPERTY(QString owner READ owner)
	Q_PROPERTY(uint ownerId READ ownerId)
	Q_PROPERTY(QString group READ group)
	Q_PROPERTY(uint groupId READ groupId)

	Q_PROPERTY(QDateTime created READ created)
	Q_PROPERTY(QDateTime lastModified READ lastModified)
	Q_PROPERTY(QDateTime lastRead READ lastRead)

	Q_PROPERTY(QFileDevice::Permissions permissions READ permissions)

public:
	static void Register(QScriptEngine *engine);

	explicit ScriptQFileInfo(QObject *parent);

	QString filePath() const;
	void setFilePath(const QString &file);

	QString absoluteFilePath() const;
	QString canonicalFilePath() const;
	QString fileName() const;
	QString baseName() const;
	QString completeBaseName() const;
	QString suffix() const;
	QString bundleName() const;
	QString completeSuffix() const;

	QString path() const;
	QString absolutePath() const;
	QString canonicalPath() const;
	QDir dir() const;
	QDir absoluteDir() const;

	bool isReadable() const;
	bool isWritable() const;
	bool isExecutable() const;
	bool isHidden() const;
	bool isNativePath() const;

	bool isRelative() const;
	inline bool isAbsolute() const;

	bool isFile() const;
	bool isDir() const;
	bool isSymLink() const;
	bool isRoot() const;
	bool isBundle() const;

	QString symLinkTarget() const;

	QString owner() const;
	uint ownerId() const;
	QString group() const;
	uint groupId() const;
	QFileDevice::Permissions permissions() const;

	qint64 size() const;

	QDateTime created() const;
	QDateTime lastModified() const;
	QDateTime lastRead() const;

	bool caching() const;
	void setCaching(bool on);

public slots:
	bool exists() const;
	void refresh();
	bool makeAbsolute();

	bool permission(QFileDevice::Permissions permissions) const;

	QString toString() const;

private:
	static QString className();
	QFileInfo *thisInfo() const;

	static QScriptValue Constructor(
		QScriptContext *context, QScriptEngine *engine);
	static QScriptValue ToScriptValue(
		QScriptEngine *engine, const QFileInfo &in);
	static void FromScriptValue(const QScriptValue &object, QFileInfo &out);
};

bool ScriptQFileInfo::isAbsolute() const
{
	return not isRelative();
}
}
