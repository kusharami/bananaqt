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

#include "ScriptQFile.h"

#include "ScriptUtils.h"
#include "BananaCore/Utils.h"

namespace Banana
{
using namespace Script;

QScriptValue ScriptQFile::Register(QScriptEngine *engine)
{
	auto proto = NewQObjectPrototype<ScriptQFile>(engine);
	proto.setPrototype(engine->defaultPrototype(qMetaTypeId<QFileDevice *>()));
	engine->setDefaultPrototype(qMetaTypeId<QFile *>(), proto);

	auto qFileObject = engine->newFunction(ScriptQFile::Construct, proto);

	qFileObject.setProperty(
		QSTRKEY(exists), engine->newFunction(exists), STATIC_SCRIPT_VALUE);
	qFileObject.setProperty(
		QSTRKEY(remove), engine->newFunction(remove), STATIC_SCRIPT_VALUE);
	qFileObject.setProperty(
		QSTRKEY(rename), engine->newFunction(rename), STATIC_SCRIPT_VALUE);
	qFileObject.setProperty(
		QSTRKEY(copy), engine->newFunction(copy), STATIC_SCRIPT_VALUE);

	engine->globalObject().setProperty(
		className(), qFileObject, STATIC_SCRIPT_VALUE);

	return proto;
}

ScriptQFile::ScriptQFile(QObject *parent)
	: ScriptQFileDevice(parent)
{
}

void ScriptQFile::setFilePath(const QString &value)
{
	auto file = thisFile();
	if (file)
		file->setFileName(value);
}

QString ScriptQFile::symLinkTarget() const
{
	auto file = thisFile();
	if (file)
		return file->symLinkTarget();

	return QString();
}

QString ScriptQFile::toString() const
{
	if (nullptr == thisFile())
		return className();

	return ScriptQFileDevice::toString();
}

bool ScriptQFile::exists()
{
	auto file = thisFile();
	if (file)
	{
		if (file->fileName().isEmpty())
			return false;

		return file->exists();
	}

	return false;
}

bool ScriptQFile::remove()
{
	auto file = thisFile();
	if (file)
		return file->remove();

	return false;
}

bool ScriptQFile::rename(const QString &newFilePath)
{
	auto file = thisFile();
	if (file)
		return file->rename(newFilePath);

	return false;
}

bool ScriptQFile::copy(const QString &newFilePath)
{
	auto file = thisFile();
	if (file)
		return file->copy(newFilePath);

	return false;
}

bool ScriptQFile::link(const QString &linkPath)
{
	auto file = thisFile();
	if (file)
		return Utils::CreateSymLink(file->fileName(), linkPath);

	return false;
}

QString ScriptQFile::className()
{
	return QSTRKEY(QFile);
}

QScriptValue ScriptQFile::exists(QScriptContext *context, QScriptEngine *engine)
{
	if (context->argumentCount() != 1)
		return ThrowBadNumberOfArguments(context);

	return QScriptValue(engine, QFile::exists(context->argument(0).toString()));
}

QScriptValue ScriptQFile::remove(QScriptContext *context, QScriptEngine *engine)
{
	if (context->argumentCount() != 1)
		return ThrowBadNumberOfArguments(context);

	return QScriptValue(engine, QFile::remove(context->argument(0).toString()));
}

QScriptValue ScriptQFile::rename(QScriptContext *context, QScriptEngine *engine)
{
	if (context->argumentCount() != 2)
		return ThrowBadNumberOfArguments(context);

	return QScriptValue(engine,
		QFile::rename(
			context->argument(0).toString(), context->argument(1).toString()));
}

QScriptValue ScriptQFile::copy(QScriptContext *context, QScriptEngine *engine)
{
	if (context->argumentCount() != 2)
		return ThrowBadNumberOfArguments(context);

	return QScriptValue(engine,
		QFile::copy(
			context->argument(0).toString(), context->argument(1).toString()));
}

QScriptValue ScriptQFile::Construct(
	QScriptContext *context, QScriptEngine *engine)
{
	int argumentCount = context->argumentCount();

	switch (argumentCount)
	{
		case 0:
		case 1:
			break;

		default:
			return ThrowBadNumberOfArguments(context);
	}

	auto newFile = new QFile;

	if (argumentCount == 1)
		newFile->setFileName(context->argument(0).toString());

	return ConstructQObject(newFile, context, engine);
}

QFile *ScriptQFile::thisFile() const
{
	return qobject_cast<QFile *>(thisObject().toQObject());
}
}
