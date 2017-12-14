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
#include <QDir>
#include <QScriptValue>
#include <QScriptable>

Q_DECLARE_METATYPE(QDir)
Q_DECLARE_METATYPE(QDir *)
Q_DECLARE_METATYPE(QDir::Filters)
Q_DECLARE_METATYPE(QDir::SortFlags)

namespace Banana
{
class ScriptQDir final
	: public QObject
	, public QScriptable
{
	Q_OBJECT

	Q_PROPERTY(QString path READ path WRITE setPath)
	Q_PROPERTY(QString absolutePath READ absolutePath)
	Q_PROPERTY(QString canonicalPath READ canonicalPath)

	Q_PROPERTY(QString dirName READ dirName)

	Q_PROPERTY(bool readable READ isReadable)
	Q_PROPERTY(bool relative READ isRelative)
	Q_PROPERTY(bool absolute READ isAbsolute)
	Q_PROPERTY(bool isRoot READ isRoot)

public:
	static void Register(QScriptEngine *engine);

	explicit ScriptQDir(QObject *parent);

	void setPath(const QString &path);
	QString path() const;
	QString absolutePath() const;
	QString canonicalPath() const;

	QString dirName() const;

	bool isReadable() const;
	bool isRoot() const;

	bool isRelative() const;
	inline bool isAbsolute() const;

public slots:
	bool exists() const;
	bool exists(const QString &name) const;
	bool mkdir(const QString &dirName) const;
	bool rmdir(const QString &dirName) const;
	bool mkpath(const QString &dirPath) const;
	bool rmpath(const QString &dirPath) const;

	bool removeRecursively();
	bool makeAbsolute();

	bool cd(const QString &dirName);
	bool cdUp();

	QString filePath(const QString &fileName) const;
	QString absoluteFilePath(const QString &fileName) const;
	QString relativeFilePath(const QString &fileName) const;

	bool remove(const QString &fileName);
	bool rename(const QString &oldName, const QString &newName);

	QScriptValue entryList(const QScriptValue &nameFilters,
		QDir::Filters filters = QDir::NoFilter,
		QDir::SortFlags sort = QDir::NoSort) const;

	void refresh();

	bool equals(const QDir &other) const;

	QString toString() const;

private:
	QDir *thisDir() const;

	static QString className();

	static QScriptValue listSeparator(
		QScriptContext *context, QScriptEngine *engine);
	static QScriptValue separator(
		QScriptContext *context, QScriptEngine *engine);

	static QScriptValue current(QScriptContext *context, QScriptEngine *engine);
	static QScriptValue currentPath(
		QScriptContext *context, QScriptEngine *engine);
	static QScriptValue home(QScriptContext *context, QScriptEngine *engine);
	static QScriptValue homePath(
		QScriptContext *context, QScriptEngine *engine);
	static QScriptValue root(QScriptContext *context, QScriptEngine *engine);
	static QScriptValue rootPath(
		QScriptContext *context, QScriptEngine *engine);
	static QScriptValue temp(QScriptContext *context, QScriptEngine *engine);
	static QScriptValue tempPath(
		QScriptContext *context, QScriptEngine *engine);

	static QScriptValue cleanPath(
		QScriptContext *context, QScriptEngine *engine);
	static QScriptValue exists(QScriptContext *context, QScriptEngine *engine);
	static QScriptValue remove(QScriptContext *context, QScriptEngine *engine);
	static QScriptValue rename(QScriptContext *context, QScriptEngine *engine);

	static QScriptValue toNativeSeparators(
		QScriptContext *context, QScriptEngine *engine);
	static QScriptValue fromNativeSeparators(
		QScriptContext *context, QScriptEngine *engine);

	static QScriptValue Constructor(
		QScriptContext *context, QScriptEngine *engine);
	static QScriptValue ToScriptValue(QScriptEngine *engine, const QDir &in);
	static void FromScriptValue(const QScriptValue &object, QDir &out);
};

bool ScriptQDir::isAbsolute() const
{
	return not isRelative();
}
}
