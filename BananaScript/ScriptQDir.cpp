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

#include "ScriptQDir.h"

#include "ScriptUtils.h"

namespace Banana
{
using namespace Script;

namespace Script
{
struct Dir : public QDir
{
	static void RegisterEnums(QScriptEngine *engine, QScriptValue &to)
	{
		static bool convertersRegistered = false;
		if (not convertersRegistered)
		{
			convertersRegistered = true;
			QMetaType::registerConverter<Filters, qint32>();
			QMetaType::registerConverter<SortFlags, qint32>();
		}

		qScriptRegisterMetaType(
			engine, EnumToScriptValue<Filters>, EnumFromScriptValue<Filters>);

		qScriptRegisterMetaType(engine, EnumToScriptValue<SortFlags>,
			EnumFromScriptValue<SortFlags>);

		SCRIPT_REG_ENUM(to, Dirs);
		SCRIPT_REG_ENUM(to, Files);
		SCRIPT_REG_ENUM(to, Drives);
		SCRIPT_REG_ENUM(to, NoSymLinks);
		SCRIPT_REG_ENUM(to, AllEntries);
		SCRIPT_REG_ENUM(to, Readable);
		SCRIPT_REG_ENUM(to, Writable);
		SCRIPT_REG_ENUM(to, Executable);
		SCRIPT_REG_ENUM(to, Modified);
		SCRIPT_REG_ENUM(to, Hidden);
		SCRIPT_REG_ENUM(to, System);
		SCRIPT_REG_ENUM(to, AllDirs);
		SCRIPT_REG_ENUM(to, CaseSensitive);
		SCRIPT_REG_ENUM(to, NoDot);
		SCRIPT_REG_ENUM(to, NoDotDot);
		SCRIPT_REG_ENUM(to, NoDotAndDotDot);
		SCRIPT_REG_ENUM(to, NoFilter);
		to.setProperty(
			QSTRKEY(NoDots), (int) NoDotAndDotDot, STATIC_SCRIPT_VALUE);

		SCRIPT_REG_ENUM(to, Name);
		SCRIPT_REG_ENUM(to, Time);
		SCRIPT_REG_ENUM(to, Size);
		SCRIPT_REG_ENUM(to, Type);
		SCRIPT_REG_ENUM(to, Unsorted);
		SCRIPT_REG_ENUM(to, DirsFirst);
		SCRIPT_REG_ENUM(to, DirsLast);
		SCRIPT_REG_ENUM(to, Reversed);
		SCRIPT_REG_ENUM(to, IgnoreCase);
		SCRIPT_REG_ENUM(to, LocaleAware);
		SCRIPT_REG_ENUM(to, NoSort);
	}
};
}

void ScriptQDir::Register(QScriptEngine *engine)
{
	qScriptRegisterMetaType(engine, ToScriptValue, FromScriptValue);

	auto proto = NewQObjectPrototype<ScriptQDir>(engine);

	auto qDir = engine->newFunction(Constructor, proto);
	qDir.setData(proto);

	Dir::RegisterEnums(engine, qDir);

	qDir.setProperty(QSTRKEY(listSeparator), engine->newFunction(listSeparator),
		STATIC_SCRIPT_VALUE | QScriptValue::PropertyGetter);
	qDir.setProperty(QSTRKEY(separator), engine->newFunction(separator),
		STATIC_SCRIPT_VALUE | QScriptValue::PropertyGetter);
	qDir.setProperty(
		QSTRKEY(current), engine->newFunction(current), STATIC_SCRIPT_VALUE);
	qDir.setProperty(QSTRKEY(currentPath), engine->newFunction(currentPath),
		STATIC_SCRIPT_VALUE | QScriptValue::PropertyGetter);
	qDir.setProperty(
		QSTRKEY(home), engine->newFunction(home), STATIC_SCRIPT_VALUE);
	qDir.setProperty(QSTRKEY(homePath), engine->newFunction(homePath),
		STATIC_SCRIPT_VALUE | QScriptValue::PropertyGetter);
	qDir.setProperty(
		QSTRKEY(root), engine->newFunction(root), STATIC_SCRIPT_VALUE);
	qDir.setProperty(QSTRKEY(rootPath), engine->newFunction(rootPath),
		STATIC_SCRIPT_VALUE | QScriptValue::PropertyGetter);
	qDir.setProperty(
		QSTRKEY(temp), engine->newFunction(temp), STATIC_SCRIPT_VALUE);
	qDir.setProperty(QSTRKEY(tempPath), engine->newFunction(tempPath),
		STATIC_SCRIPT_VALUE | QScriptValue::PropertyGetter);
	qDir.setProperty(QSTRKEY(cleanPath), engine->newFunction(cleanPath),
		STATIC_SCRIPT_VALUE);
	qDir.setProperty(
		QSTRKEY(exists), engine->newFunction(exists), STATIC_SCRIPT_VALUE);
	qDir.setProperty(
		QSTRKEY(remove), engine->newFunction(remove), STATIC_SCRIPT_VALUE);
	qDir.setProperty(
		QSTRKEY(rename), engine->newFunction(rename), STATIC_SCRIPT_VALUE);
	qDir.setProperty(QSTRKEY(toNativeSeparators),
		engine->newFunction(toNativeSeparators), STATIC_SCRIPT_VALUE);
	qDir.setProperty(QSTRKEY(fromNativeSeparators),
		engine->newFunction(fromNativeSeparators), STATIC_SCRIPT_VALUE);

	engine->globalObject().setProperty(className(), qDir, STATIC_SCRIPT_VALUE);
}

ScriptQDir::ScriptQDir(QObject *parent)
	: QObject(parent)
{
}

void ScriptQDir::setPath(const QString &path)
{
	auto dir = thisDir();
	if (dir)
		dir->setPath(path);
}

QString ScriptQDir::path() const
{
	auto dir = thisDir();
	if (dir)
		return dir->path();

	return QString();
}

QString ScriptQDir::absolutePath() const
{
	auto dir = thisDir();
	if (dir)
		return dir->absolutePath();

	return QString();
}

QString ScriptQDir::canonicalPath() const
{
	auto dir = thisDir();
	if (dir)
		return dir->canonicalPath();

	return QString();
}

QString ScriptQDir::dirName() const
{
	auto dir = thisDir();
	if (dir)
		return dir->dirName();

	return QString();
}

bool ScriptQDir::isReadable() const
{
	auto dir = thisDir();
	if (dir)
		return dir->isReadable();

	return false;
}

bool ScriptQDir::isRoot() const
{
	auto dir = thisDir();
	if (dir)
		return dir->isRoot();

	return false;
}

bool ScriptQDir::isRelative() const
{
	auto dir = thisDir();
	if (dir)
		return dir->isRelative();

	return true;
}

bool ScriptQDir::exists() const
{
	auto dir = thisDir();
	if (dir)
		return dir->exists();

	return false;
}

bool ScriptQDir::exists(const QString &name) const
{
	auto dir = thisDir();
	if (dir)
		return dir->exists(name);

	return false;
}

bool ScriptQDir::mkdir(const QString &dirName) const
{
	auto dir = thisDir();
	if (dir)
		return dir->mkdir(dirName);

	return false;
}

bool ScriptQDir::rmdir(const QString &dirName) const
{
	auto dir = thisDir();
	if (dir)
		return dir->rmdir(dirName);

	return false;
}

bool ScriptQDir::mkpath(const QString &dirPath) const
{
	auto dir = thisDir();
	if (dir)
		return dir->mkpath(dirPath);

	return false;
}

bool ScriptQDir::rmpath(const QString &dirPath) const
{
	auto dir = thisDir();
	if (dir)
		return dir->rmpath(dirPath);

	return false;
}

bool ScriptQDir::removeRecursively()
{
	auto dir = thisDir();
	if (dir)
		return dir->removeRecursively();

	return false;
}

bool ScriptQDir::makeAbsolute()
{
	auto dir = thisDir();
	if (dir)
		return dir->makeAbsolute();

	return false;
}

bool ScriptQDir::cd(const QString &dirName)
{
	auto dir = thisDir();
	if (dir)
		return dir->cd(dirName);

	return false;
}

bool ScriptQDir::cdUp()
{
	auto dir = thisDir();
	if (dir)
		return dir->cdUp();

	return false;
}

QString ScriptQDir::filePath(const QString &fileName) const
{
	auto dir = thisDir();
	if (dir)
		return dir->filePath(fileName);

	return QString();
}

QString ScriptQDir::absoluteFilePath(const QString &fileName) const
{
	auto dir = thisDir();
	if (dir)
		return dir->absoluteFilePath(fileName);

	return QString();
}

QString ScriptQDir::relativeFilePath(const QString &fileName) const
{
	auto dir = thisDir();
	if (dir)
		return dir->relativeFilePath(fileName);

	return QString();
}

bool ScriptQDir::remove(const QString &fileName)
{
	auto dir = thisDir();
	if (dir)
		return dir->remove(fileName);

	return false;
}

bool ScriptQDir::rename(const QString &oldName, const QString &newName)
{
	auto dir = thisDir();
	if (dir)
		return dir->rename(oldName, newName);

	return false;
}

QScriptValue ScriptQDir::entryList(const QScriptValue &nameFilters,
	QDir::Filters filters, QDir::SortFlags sort) const
{
	QScriptValue result;
	auto dir = thisDir();
	if (!dir)
		return result;

	auto engine = nameFilters.engine();
	Q_ASSERT(nullptr != engine);

	if (!nameFilters.isArray())
		return ThrowIncompatibleArgumentType(engine->currentContext(), 1);

	QStringList list;

	int count = nameFilters.property(QSTRKEY(length)).toInt32();
	for (int i = 0; i < count; i++)
	{
		list.push_back(nameFilters.property(i).toString());
	}

	list = dir->entryList(list, filters, sort);

	count = list.count();
	result = engine->newArray(uint(list.count()));
	for (int i = 0; i < count; i++)
	{
		result.setProperty(quint32(i), list.at(i));
	}

	return result;
}

void ScriptQDir::refresh()
{
	auto dir = thisDir();
	if (dir)
		dir->refresh();
}

bool ScriptQDir::equals(const QDir &other) const
{
	auto dir = thisDir();
	if (dir)
		return *dir == other;

	return false;
}

QString ScriptQDir::toString() const
{
	auto dir = thisDir();
	if (dir)
	{
		return dir->path();
	}

	return className();
}

QDir *ScriptQDir::thisDir() const
{
	return qscriptvalue_cast<QDir *>(thisObject().data());
}

QString ScriptQDir::className()
{
	return QSTRKEY(QDir);
}

QScriptValue ScriptQDir::listSeparator(
	QScriptContext *context, QScriptEngine *engine)
{
	if (context->argumentCount() != 0)
		return ThrowBadNumberOfArguments(context);

	return QScriptValue(engine, QString(QDir::listSeparator()));
}

QScriptValue ScriptQDir::separator(
	QScriptContext *context, QScriptEngine *engine)
{
	if (context->argumentCount() != 0)
		return ThrowBadNumberOfArguments(context);

	return QScriptValue(engine, QString(QDir::separator()));
}

QScriptValue ScriptQDir::current(QScriptContext *context, QScriptEngine *engine)
{
	if (context->argumentCount() != 0)
		return ThrowBadNumberOfArguments(context);

	return ToScriptValue(engine, QDir::current());
}

QScriptValue ScriptQDir::currentPath(
	QScriptContext *context, QScriptEngine *engine)
{
	if (context->argumentCount() != 0)
		return ThrowBadNumberOfArguments(context);

	return QScriptValue(engine, QDir::currentPath());
}

QScriptValue ScriptQDir::home(QScriptContext *context, QScriptEngine *engine)
{
	if (context->argumentCount() != 0)
		return ThrowBadNumberOfArguments(context);

	return ToScriptValue(engine, QDir::home());
}

QScriptValue ScriptQDir::homePath(
	QScriptContext *context, QScriptEngine *engine)
{
	if (context->argumentCount() != 0)
		return ThrowBadNumberOfArguments(context);

	return QScriptValue(engine, QDir::homePath());
}

QScriptValue ScriptQDir::root(QScriptContext *context, QScriptEngine *engine)
{
	if (context->argumentCount() != 0)
		return ThrowBadNumberOfArguments(context);

	return ToScriptValue(engine, QDir::root());
}

QScriptValue ScriptQDir::rootPath(
	QScriptContext *context, QScriptEngine *engine)
{
	if (context->argumentCount() != 0)
		return ThrowBadNumberOfArguments(context);

	return QScriptValue(engine, QDir::rootPath());
}

QScriptValue ScriptQDir::temp(QScriptContext *context, QScriptEngine *engine)
{
	if (context->argumentCount() != 0)
		return ThrowBadNumberOfArguments(context);

	return ToScriptValue(engine, QDir::temp());
}

QScriptValue ScriptQDir::tempPath(
	QScriptContext *context, QScriptEngine *engine)
{
	if (context->argumentCount() != 0)
		return ThrowBadNumberOfArguments(context);

	return QScriptValue(engine, QDir::tempPath());
}

QScriptValue ScriptQDir::exists(QScriptContext *context, QScriptEngine *engine)
{
	if (context->argumentCount() != 1)
		return ThrowBadNumberOfArguments(context);

	return QScriptValue(engine, QDir(context->argument(0).toString()).exists());
}

QScriptValue ScriptQDir::remove(QScriptContext *context, QScriptEngine *engine)
{
	if (context->argumentCount() != 1)
		return ThrowBadNumberOfArguments(context);

	return QScriptValue(
		engine, QDir(context->argument(0).toString()).removeRecursively());
}

QScriptValue ScriptQDir::rename(QScriptContext *context, QScriptEngine *engine)
{
	if (context->argumentCount() != 2)
		return ThrowBadNumberOfArguments(context);

	return QScriptValue(engine,
		QDir().rename(
			context->argument(0).toString(), context->argument(1).toString()));
}

QScriptValue ScriptQDir::cleanPath(
	QScriptContext *context, QScriptEngine *engine)
{
	if (context->argumentCount() != 1)
		return ThrowBadNumberOfArguments(context);

	return QScriptValue(
		engine, QDir::cleanPath(context->argument(0).toString()));
}

QScriptValue ScriptQDir::toNativeSeparators(
	QScriptContext *context, QScriptEngine *engine)
{
	if (context->argumentCount() != 1)
		return ThrowBadNumberOfArguments(context);

	return QScriptValue(
		engine, QDir::toNativeSeparators(context->argument(0).toString()));
}

QScriptValue ScriptQDir::fromNativeSeparators(
	QScriptContext *context, QScriptEngine *engine)
{
	if (context->argumentCount() != 1)
		return ThrowBadNumberOfArguments(context);

	return QScriptValue(
		engine, QDir::fromNativeSeparators(context->argument(0).toString()));
}

QScriptValue ScriptQDir::Constructor(
	QScriptContext *context, QScriptEngine *engine)
{
	QDir dir;
	switch (context->argumentCount())
	{
		case 1:
			FromScriptValue(context->argument(0), dir);
			break;

		case 0:
			break;

		default:
			return ThrowBadNumberOfArguments(context);
	}

	return ToScriptValue(engine, dir);
}

QScriptValue ScriptQDir::ToScriptValue(QScriptEngine *engine, const QDir &in)
{
	auto ctor = engine->globalObject().property(className());

	auto result = engine->newObject();
	result.setData(engine->newVariant(QVariant::fromValue(in)));
	result.setPrototype(ctor.data());
	return result;
}

void ScriptQDir::FromScriptValue(const QScriptValue &object, QDir &out)
{
	auto d = qscriptvalue_cast<QDir *>(object.data());
	out = d ? *d : QDir(object.toString());
}
}
