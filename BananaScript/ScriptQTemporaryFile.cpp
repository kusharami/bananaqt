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

#include "ScriptQTemporaryFile.h"

#include "ScriptUtils.h"

namespace Banana
{
using namespace Script;

ScriptQTemporaryFile::ScriptQTemporaryFile(QObject *parent)
	: ScriptQFile(parent)
{
}

void ScriptQTemporaryFile::Register(QScriptEngine *engine)
{
	auto proto = NewQObjectPrototype<ScriptQTemporaryFile>(engine);
	proto.setPrototype(engine->defaultPrototype(qMetaTypeId<QFile *>()));
	engine->setDefaultPrototype(qMetaTypeId<QTemporaryFile *>(), proto);

	auto qTemporaryFileObject =
		engine->newFunction(ScriptQTemporaryFile::Construct, proto);

	engine->globalObject().setProperty(
		QSTRKEY(QTemporaryFile), qTemporaryFileObject, STATIC_SCRIPT_VALUE);
}

bool ScriptQTemporaryFile::autoRemove() const
{
	auto file = thisFile();
	if (file)
		return file->autoRemove();

	return false;
}

void ScriptQTemporaryFile::setAutoRemove(bool b)
{
	auto file = thisFile();
	if (file)
		file->setAutoRemove(b);
}

QString ScriptQTemporaryFile::fileTemplate() const
{
	auto file = thisFile();
	if (file)
		return file->fileTemplate();

	return QString();
}

void ScriptQTemporaryFile::setFileTemplate(const QString &name)
{
	auto file = thisFile();
	if (file)
		file->setFileTemplate(name);
}

QString ScriptQTemporaryFile::toString() const
{
	auto file = thisFile();
	if (nullptr == file)
		return QSTRKEY(QTemporaryFile);

	auto result = file->fileName();

	if (result.isEmpty())
		return file->fileTemplate();

	return result;
}

void ScriptQTemporaryFile::close()
{
	auto file = thisFile();
	if (file && file->isOpen() && file->autoRemove())
	{
		file->remove();
	}
}

bool ScriptQTemporaryFile::open()
{
	auto file = thisFile();
	if (file)
		return file->open();

	return false;
}

QScriptValue ScriptQTemporaryFile::Construct(
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

	auto newFile = new QTemporaryFile;

	if (argumentCount == 1)
		newFile->setFileTemplate(context->argument(0).toString());

	return WrapQObject(newFile, engine, context);
}

QTemporaryFile *ScriptQTemporaryFile::thisFile() const
{
	return qobject_cast<QTemporaryFile *>(thisObject().toQObject());
}
}
