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

#include "ScriptQFileInfo.h"

#include "ScriptUtils.h"

namespace Banana
{
using namespace Script;

void ScriptQFileInfo::Register(QScriptEngine *engine)
{
	qScriptRegisterMetaType(engine, ScriptQFileInfo::ToScriptValue,
		ScriptQFileInfo::FromScriptValue);

	auto proto = NewQObjectPrototype<ScriptQFileInfo>(engine);
	auto ctor = engine->newFunction(Constructor, proto);
	ctor.setData(proto);

	engine->globalObject().setProperty(className(), ctor, STATIC_SCRIPT_VALUE);
}

ScriptQFileInfo::ScriptQFileInfo(QObject *parent)
	: QObject(parent)
{
}

QString ScriptQFileInfo::filePath() const
{
	auto info = thisInfo();
	if (info)
		return info->filePath();

	return QString();
}

void ScriptQFileInfo::setFilePath(const QString &file)
{
	auto info = thisInfo();
	if (info)
		info->setFile(file);
}

QString ScriptQFileInfo::absoluteFilePath() const
{
	auto info = thisInfo();
	if (info)
		return info->absoluteFilePath();

	return QString();
}

QString ScriptQFileInfo::canonicalFilePath() const
{
	auto info = thisInfo();
	if (info)
		return info->canonicalFilePath();

	return QString();
}

QString ScriptQFileInfo::fileName() const
{
	auto info = thisInfo();
	if (info)
		return info->fileName();

	return QString();
}

QString ScriptQFileInfo::baseName() const
{
	auto info = thisInfo();
	if (info)
		return info->baseName();

	return QString();
}

QString ScriptQFileInfo::completeBaseName() const
{
	auto info = thisInfo();
	if (info)
		return info->completeBaseName();

	return QString();
}

QString ScriptQFileInfo::suffix() const
{
	auto info = thisInfo();
	if (info)
		return info->suffix();

	return QString();
}

QString ScriptQFileInfo::bundleName() const
{
	auto info = thisInfo();
	if (info)
		return info->bundleName();

	return QString();
}

QString ScriptQFileInfo::completeSuffix() const
{
	auto info = thisInfo();
	if (info)
		return info->completeSuffix();

	return QString();
}

QString ScriptQFileInfo::path() const
{
	auto info = thisInfo();
	if (info)
		return info->path();

	return QString();
}

QString ScriptQFileInfo::absolutePath() const
{
	auto info = thisInfo();
	if (info)
		return info->absolutePath();

	return QString();
}

QString ScriptQFileInfo::canonicalPath() const
{
	auto info = thisInfo();
	if (info)
		return info->canonicalPath();

	return QString();
}

QDir ScriptQFileInfo::dir() const
{
	auto info = thisInfo();
	if (info)
		return info->dir();

	return QDir();
}

QDir ScriptQFileInfo::absoluteDir() const
{
	auto info = thisInfo();
	if (info)
		return info->absoluteDir();

	return QDir();
}

bool ScriptQFileInfo::isReadable() const
{
	auto info = thisInfo();
	if (info)
		return info->isReadable();

	return false;
}

bool ScriptQFileInfo::isWritable() const
{
	auto info = thisInfo();
	if (info)
		return info->isWritable();

	return false;
}

bool ScriptQFileInfo::isExecutable() const
{
	auto info = thisInfo();
	if (info)
		return info->isExecutable();

	return false;
}

bool ScriptQFileInfo::isHidden() const
{
	auto info = thisInfo();
	if (info)
		return info->isHidden();

	return false;
}

bool ScriptQFileInfo::isNativePath() const
{
	auto info = thisInfo();
	if (info)
		return info->isNativePath();

	return false;
}

bool ScriptQFileInfo::isRelative() const
{
	auto info = thisInfo();
	if (info)
		return info->isRelative();

	return true;
}

bool ScriptQFileInfo::isFile() const
{
	auto info = thisInfo();
	if (info)
		return info->isFile();

	return false;
}

bool ScriptQFileInfo::isDir() const
{
	auto info = thisInfo();
	if (info)
		return info->isDir();

	return false;
}

bool ScriptQFileInfo::isSymLink() const
{
	auto info = thisInfo();
	if (info)
		return info->isSymLink();

	return false;
}

bool ScriptQFileInfo::isRoot() const
{
	auto info = thisInfo();
	if (info)
		return info->isRoot();

	return false;
}

bool ScriptQFileInfo::isBundle() const
{
	auto info = thisInfo();
	if (info)
		return info->isBundle();

	return false;
}

QString ScriptQFileInfo::symLinkTarget() const
{
	auto info = thisInfo();
	if (info)
		return info->symLinkTarget();

	return QString();
}

QString ScriptQFileInfo::owner() const
{
	auto info = thisInfo();
	if (info)
		return info->owner();

	return QString();
}

uint ScriptQFileInfo::ownerId() const
{
	auto info = thisInfo();
	if (info)
		return info->ownerId();

	return 0;
}

QString ScriptQFileInfo::group() const
{
	auto info = thisInfo();
	if (info)
		return info->group();

	return QString();
}

uint ScriptQFileInfo::groupId() const
{
	auto info = thisInfo();
	if (info)
		return info->groupId();

	return 0;
}

QFileDevice::Permissions ScriptQFileInfo::permissions() const
{
	auto info = thisInfo();
	if (info)
		return info->permissions();

	return QFileDevice::Permissions();
}

qint64 ScriptQFileInfo::size() const
{
	auto info = thisInfo();
	if (info)
		return info->size();

	return -1;
}

QDateTime ScriptQFileInfo::created() const
{
	auto info = thisInfo();
	if (info)
		return info->created();

	return QDateTime();
}

QDateTime ScriptQFileInfo::lastModified() const
{
	auto info = thisInfo();
	if (info)
		return info->lastModified();

	return QDateTime();
}

QDateTime ScriptQFileInfo::lastRead() const
{
	auto info = thisInfo();
	if (info)
		return info->lastRead();

	return QDateTime();
}

bool ScriptQFileInfo::caching() const
{
	auto info = thisInfo();
	if (info)
		return info->caching();

	return false;
}

void ScriptQFileInfo::setCaching(bool on)
{
	auto info = thisInfo();
	if (info)
		info->setCaching(on);
}

bool ScriptQFileInfo::exists() const
{
	auto info = thisInfo();
	if (info)
		return info->exists();

	return false;
}

void ScriptQFileInfo::refresh()
{
	auto info = thisInfo();
	if (info)
		info->refresh();
}

bool ScriptQFileInfo::makeAbsolute()
{
	auto info = thisInfo();
	if (info)
		return info->makeAbsolute();

	return false;
}

bool ScriptQFileInfo::permission(QFileDevice::Permissions permissions) const
{
	auto info = thisInfo();
	if (info)
		return info->permission(permissions);

	return false;
}

QString ScriptQFileInfo::toString() const
{
	auto info = thisInfo();
	if (info)
	{
		return info->filePath();
	}

	return className();
}

QString ScriptQFileInfo::className()
{
	return QSTRKEY(QFileInfo);
}

QFileInfo *ScriptQFileInfo::thisInfo() const
{
	return qscriptvalue_cast<QFileInfo *>(thisObject().data());
}

QScriptValue ScriptQFileInfo::Constructor(
	QScriptContext *context, QScriptEngine *engine)
{
	QFileInfo fileInfo;
	switch (context->argumentCount())
	{
		case 1:
			FromScriptValue(context->argument(0), fileInfo);
			break;

		case 0:
			break;

		default:
			return ThrowBadNumberOfArguments(context);
	}

	return ToScriptValue(engine, fileInfo);
}

QScriptValue ScriptQFileInfo::ToScriptValue(
	QScriptEngine *engine, const QFileInfo &in)
{
	auto ctor = engine->globalObject().property(className());

	auto result = engine->newObject();
	result.setData(engine->newVariant(QVariant::fromValue(in)));
	result.setPrototype(ctor.data());
	return result;
}

void ScriptQFileInfo::FromScriptValue(
	const QScriptValue &object, QFileInfo &out)
{
	auto fi = qscriptvalue_cast<QFileInfo *>(object.data());
	out = fi ? *fi : QFileInfo(object.toString());
}
}
