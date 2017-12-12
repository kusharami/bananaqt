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

#include "ScriptQFileDevice.h"

#include <QScriptValue>
#include <QScriptContext>
#include <QScriptEngine>

#include "ScriptUtils.h"

namespace Banana
{
namespace Script
{
struct FileDevice : public QFileDevice
{
	static void RegisterEnums(QScriptEngine *engine, QScriptValue &to)
	{
		static bool convertersRegistered = false;
		if (!convertersRegistered)
		{
			convertersRegistered = true;
			QMetaType::registerConverter<Permissions, qint32>();
			QMetaType::registerConverter<FileError, qint32>();
		}

		qScriptRegisterMetaType(engine, EnumToScriptValue<Permissions>,
			EnumFromScriptValue<Permissions>);

		qScriptRegisterMetaType(engine, EnumToScriptValue<FileError>,
			EnumFromScriptValue<FileError>);

		// QFileDevice::Permissions
		SCRIPT_REG_ENUM(to, ReadOwner);
		SCRIPT_REG_ENUM(to, ReadUser);
		SCRIPT_REG_ENUM(to, ReadGroup);
		SCRIPT_REG_ENUM(to, ReadOther);
		SCRIPT_REG_ENUM(to, WriteOwner);
		SCRIPT_REG_ENUM(to, WriteUser);
		SCRIPT_REG_ENUM(to, WriteGroup);
		SCRIPT_REG_ENUM(to, WriteOther);
		SCRIPT_REG_ENUM(to, ExeOwner);
		SCRIPT_REG_ENUM(to, ExeUser);
		SCRIPT_REG_ENUM(to, ExeGroup);
		SCRIPT_REG_ENUM(to, ExeOther);

		// QFileDevice::FileError
		SCRIPT_REG_ENUM(to, NoError);
		SCRIPT_REG_ENUM(to, ReadError);
		SCRIPT_REG_ENUM(to, WriteError);
		SCRIPT_REG_ENUM(to, FatalError);
		SCRIPT_REG_ENUM(to, ResourceError);
		SCRIPT_REG_ENUM(to, OpenError);
		SCRIPT_REG_ENUM(to, AbortError);
		SCRIPT_REG_ENUM(to, TimeOutError);
		SCRIPT_REG_ENUM(to, UnspecifiedError);
		SCRIPT_REG_ENUM(to, RemoveError);
		SCRIPT_REG_ENUM(to, RenameError);
		SCRIPT_REG_ENUM(to, PositionError);
		SCRIPT_REG_ENUM(to, ResizeError);
		SCRIPT_REG_ENUM(to, PermissionsError);
		SCRIPT_REG_ENUM(to, CopyError);
	}
};
}

void ScriptQFileDevice::Register(QScriptEngine *engine)
{
	auto proto = Script::NewQObjectPrototype<ScriptQFileDevice>(engine);
	proto.setPrototype(engine->defaultPrototype(qMetaTypeId<QIODevice *>()));
	engine->setDefaultPrototype(qMetaTypeId<QFileDevice *>(), proto);

	auto qFileDeviceObject = engine->newObject();
	qFileDeviceObject.setProperty(
		QSTRKEY(prototype), proto, STATIC_SCRIPT_VALUE);

	Script::FileDevice::RegisterEnums(engine, qFileDeviceObject);

	engine->globalObject().setProperty(
		className(), qFileDeviceObject, STATIC_SCRIPT_VALUE);
}

ScriptQFileDevice::ScriptQFileDevice(QObject *parent)
	: ScriptQIODevice(parent)
{
}

QFileDevice::FileError ScriptQFileDevice::error() const
{
	auto device = thisDevice();
	if (device)
		return device->error();

	return QFileDevice::UnspecifiedError;
}

QFileDevice::Permissions ScriptQFileDevice::permissions() const
{
	auto device = thisDevice();
	if (device)
		return device->permissions();

	return QFileDevice::Permissions();
}

void ScriptQFileDevice::setPermissions(QFileDevice::Permissions value)
{
	auto device = thisDevice();
	if (device && not device->setPermissions(value))
	{
		context()->throwError(tr("Unable to change file permissions."));
	}
}

void ScriptQFileDevice::setSize(qint64 size)
{
	if (not resize(size))
	{
		context()->throwError(tr("Unable to resize file."));
	}
}

QString ScriptQFileDevice::filePath() const
{
	auto device = thisDevice();
	if (device)
		return device->fileName();

	return QString();
}

bool ScriptQFileDevice::resize(qint64 newSize)
{
	auto device = thisDevice();
	if (device)
		return device->resize(newSize);

	return false;
}

bool ScriptQFileDevice::flush()
{
	auto device = thisDevice();
	if (device)
		return device->flush();

	return false;
}

void ScriptQFileDevice::unsetError()
{
	auto device = thisDevice();
	if (device)
		device->unsetError();
}

QString ScriptQFileDevice::className()
{
	return QSTRKEY(QFileDevice);
}

QString ScriptQFileDevice::toString() const
{
	auto device = thisDevice();
	if (nullptr == device)
		return className();

	return device->fileName();
}

QFileDevice *ScriptQFileDevice::thisDevice() const
{
	return qobject_cast<QFileDevice *>(thisObject().toQObject());
}
}
