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

#include "ScriptQSaveFile.h"

#include "ScriptUtils.h"

namespace Banana
{
using namespace Script;

ScriptQSaveFile::ScriptQSaveFile(QObject *parent)
	: ScriptQFileDevice(parent)
{
}

void ScriptQSaveFile::Register(QScriptEngine *engine)
{
	auto proto = NewQObjectPrototype<ScriptQSaveFile>(engine);
	proto.setPrototype(engine->defaultPrototype(qMetaTypeId<QFileDevice *>()));
	engine->setDefaultPrototype(qMetaTypeId<QSaveFile *>(), proto);

	auto qSaveFileObject =
		engine->newFunction(ScriptQSaveFile::Construct, proto);

	engine->globalObject().setProperty(
		QSTRKEY(QSaveFile), qSaveFileObject, STATIC_SCRIPT_VALUE);
}

void ScriptQSaveFile::setFilePath(const QString &value)
{
	auto file = thisFile();
	if (file)
		file->setFileName(value);
}

void ScriptQSaveFile::setDirectWriteFallback(bool enabled)
{
	auto file = thisFile();
	if (file)
		file->setDirectWriteFallback(enabled);
}

bool ScriptQSaveFile::directWriteFallback() const
{
	auto file = thisFile();
	if (file)
		return file->directWriteFallback();

	return false;
}

QString ScriptQSaveFile::toString() const
{
	if (nullptr == thisFile())
		return QSTRKEY(QSaveFile);

	return ScriptQFileDevice::toString();
}

void ScriptQSaveFile::close()
{
	context()->throwError(tr("QSaveFile.close is not allowed."));
}

bool ScriptQSaveFile::commit()
{
	auto file = thisFile();
	if (file)
		return file->commit();

	return false;
}

void ScriptQSaveFile::cancelWriting()
{
	auto file = thisFile();
	if (file)
		file->cancelWriting();
}

QScriptValue ScriptQSaveFile::Construct(
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

	auto newFile = new QSaveFile;

	if (argumentCount == 1)
		newFile->setFileName(context->argument(0).toString());

	return WrapQObject(newFile, engine, context);
}

QSaveFile *ScriptQSaveFile::thisFile() const
{
	return qobject_cast<QSaveFile *>(thisObject().toQObject());
}
}
