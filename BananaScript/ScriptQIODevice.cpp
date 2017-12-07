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

#include "ScriptQIODevice.h"

#include <QScriptValue>
#include <QScriptContext>
#include <QScriptEngine>

#include "ScriptUtils.h"

namespace Banana
{
namespace Script
{
struct IODevice : public QIODevice
{
	static void RegisterEnums(QScriptEngine *engine, QScriptValue &to)
	{
		static bool convertersRegistered = false;
		if (!convertersRegistered)
		{
			convertersRegistered = true;
			QMetaType::registerConverter<OpenMode, qint32>();
		}

		qScriptRegisterMetaType(
			engine, EnumToScriptValue<OpenMode>, EnumFromScriptValue<OpenMode>);

		// QIODevice::OpenMode
		SCRIPT_REG_ENUM(to, NotOpen);
		SCRIPT_REG_ENUM(to, ReadOnly);
		SCRIPT_REG_ENUM(to, WriteOnly);
		SCRIPT_REG_ENUM(to, ReadWrite);
		SCRIPT_REG_ENUM(to, Append);
		SCRIPT_REG_ENUM(to, Truncate);
		SCRIPT_REG_ENUM(to, Text);
		SCRIPT_REG_ENUM(to, Unbuffered);
	}
};
}

QScriptValue ScriptQIODevice::Register(QScriptEngine *engine)
{
	auto proto = Script::NewQObjectPrototype<ScriptQIODevice>(engine);
	engine->setDefaultPrototype(qMetaTypeId<QIODevice *>(), proto);

	auto qIODeviceObject = engine->newObject();
	qIODeviceObject.setProperty(QSTRKEY(prototype), proto, STATIC_SCRIPT_VALUE);

	Script::IODevice::RegisterEnums(engine, qIODeviceObject);

	engine->globalObject().setProperty(
		QSTRKEY(QIODevice), qIODeviceObject, STATIC_SCRIPT_VALUE);

	return qIODeviceObject;
}

ScriptQIODevice::ScriptQIODevice(QObject *parent)
	: QObject(parent)
{
}

QString ScriptQIODevice::errorString() const
{
	auto device = thisDevice();
	if (device)
		return device->errorString();

	return QString();
}

bool ScriptQIODevice::isTextModeEnabled() const
{
	auto device = thisDevice();
	if (device)
		return device->isTextModeEnabled();

	return false;
}

void ScriptQIODevice::setTextModeEnabled(bool value)
{
	auto device = thisDevice();
	if (device)
		device->setTextModeEnabled(value);
}

bool ScriptQIODevice::isSequential() const
{
	auto device = thisDevice();
	if (device)
		return device->isSequential();

	return false;
}

bool ScriptQIODevice::isOpen() const
{
	auto device = thisDevice();
	if (device)
		return device->isOpen();

	return false;
}

bool ScriptQIODevice::isReadable() const
{
	auto device = thisDevice();
	if (device)
		return device->isReadable();

	return false;
}

bool ScriptQIODevice::isWritable() const
{
	auto device = thisDevice();
	if (device)
		return device->isWritable();

	return false;
}

bool ScriptQIODevice::atEnd() const
{
	auto device = thisDevice();
	if (device)
		return device->atEnd();

	return false;
}

bool ScriptQIODevice::isTransactionStarted() const
{
	auto device = thisDevice();
	if (device)
		return device->isTransactionStarted();

	return false;
}

qint64 ScriptQIODevice::bytesAvailable() const
{
	auto device = thisDevice();
	if (device)
		return device->bytesAvailable();

	return -1;
}

qint64 ScriptQIODevice::bytesToWrite() const
{
	auto device = thisDevice();
	if (device)
		return device->bytesToWrite();

	return -1;
}

qint64 ScriptQIODevice::size() const
{
	auto device = thisDevice();
	if (device)
		return device->size();

	return -1;
}

qint64 ScriptQIODevice::pos() const
{
	auto device = thisDevice();
	if (device)
		return device->pos();

	return -1;
}

void ScriptQIODevice::setPos(qint64 pos)
{
	auto device = thisDevice();
	if (device && not device->seek(pos))
	{
		context()->throwError(tr("Unable to set position."));
	}
}

QIODevice::OpenMode ScriptQIODevice::openMode() const
{
	auto device = thisDevice();
	if (device)
		return device->openMode();

	return QIODevice::NotOpen;
}

bool ScriptQIODevice::open(QIODevice::OpenMode mode)
{
	auto device = thisDevice();
	if (device)
		return device->open(mode);

	return false;
}

void ScriptQIODevice::close()
{
	auto device = thisDevice();
	if (device)
		device->close();
}

bool ScriptQIODevice::seek(qint64 offset)
{
	auto device = thisDevice();
	if (device)
		return device->seek(offset);

	return false;
}

bool ScriptQIODevice::reset()
{
	auto device = thisDevice();
	if (device)
		return device->reset();

	return false;
}

void ScriptQIODevice::startTransaction()
{
	auto device = thisDevice();
	if (device)
		device->startTransaction();
}

void ScriptQIODevice::commitTransaction()
{
	auto device = thisDevice();
	if (device)
		device->commitTransaction();
}

void ScriptQIODevice::rollbackTransaction()
{
	auto device = thisDevice();
	if (device)
		device->rollbackTransaction();
}

QByteArray ScriptQIODevice::peek(int maxlen)
{
	auto device = thisDevice();
	if (device)
		return device->peek(maxlen);

	return QByteArray();
}

QByteArray ScriptQIODevice::read(int maxlen)
{
	auto device = thisDevice();
	if (device)
		return device->read(maxlen);

	return QByteArray();
}

QByteArray ScriptQIODevice::readAll()
{
	auto device = thisDevice();
	if (device)
		return device->readAll();

	return QByteArray();
}

QByteArray ScriptQIODevice::readLine(qint64 maxLen)
{
	auto device = thisDevice();
	if (device)
		return device->readLine(maxLen);

	return QByteArray();
}

bool ScriptQIODevice::canReadLine() const
{
	auto device = thisDevice();
	if (device)
		return device->canReadLine();

	return false;
}

int ScriptQIODevice::write(const QByteArray &data)
{
	auto device = thisDevice();
	if (device)
		return int(device->write(data));

	return -1;
}

QString ScriptQIODevice::toString() const
{
	auto device = thisDevice();
	if (device)
		return QString::fromLatin1(device->metaObject()->className());

	return QSTRKEY(QIODevice);
}

QIODevice *ScriptQIODevice::thisDevice() const
{
	return qobject_cast<QIODevice *>(thisObject().toQObject());
}
}
