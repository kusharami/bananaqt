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

#include "ScriptQNetworkAccessManager.h"

#include "ScriptQByteArray.h"
#include "ScriptUtils.h"

namespace Banana
{
namespace Script
{
struct NetworkAccessManager : public QNetworkAccessManager
{
	static void RegisterEnums(QScriptEngine *engine, QScriptValue &to)
	{
		static bool convertersRegistered = false;
		if (not convertersRegistered)
		{
			convertersRegistered = true;
			QMetaType::registerConverter<Operation, qint32>();
			QMetaType::registerConverter<NetworkAccessibility, qint32>();
		}

		qScriptRegisterMetaType(engine, EnumToScriptValue<Operation>,
			EnumFromScriptValue<Operation>);

		qScriptRegisterMetaType(engine, EnumToScriptValue<NetworkAccessibility>,
			EnumFromScriptValue<NetworkAccessibility>);

		SCRIPT_REG_ENUM(to, HeadOperation);
		SCRIPT_REG_ENUM(to, GetOperation);
		SCRIPT_REG_ENUM(to, PutOperation);
		SCRIPT_REG_ENUM(to, PostOperation);
		SCRIPT_REG_ENUM(to, DeleteOperation);
		SCRIPT_REG_ENUM(to, CustomOperation);
		SCRIPT_REG_ENUM(to, UnknownOperation);
		SCRIPT_REG_ENUM(to, UnknownAccessibility);
		SCRIPT_REG_ENUM(to, NotAccessible);
		SCRIPT_REG_ENUM(to, Accessible);
	}
};
}

using namespace Script;

void ScriptQNetworkAccessManager::Register(QScriptEngine *engine)
{
	auto proto = NewQObjectPrototype<ScriptQNetworkAccessManager>(engine);
	engine->setDefaultPrototype(qMetaTypeId<QNetworkAccessManager *>(), proto);

	auto qManagerObject = engine->newFunction(Construct, proto);

	NetworkAccessManager::RegisterEnums(engine, qManagerObject);

	engine->globalObject().setProperty(
		className(), qManagerObject, STATIC_SCRIPT_VALUE);
}

ScriptQNetworkAccessManager::ScriptQNetworkAccessManager(QObject *parent)
	: QObject(parent)
{
}

void ScriptQNetworkAccessManager::setNetworkAccessible(
	QNetworkAccessManager::NetworkAccessibility accessible)
{
	auto manager = thisManager();
	if (manager)
		manager->setNetworkAccessible(accessible);
}

QNetworkAccessManager::NetworkAccessibility
ScriptQNetworkAccessManager::networkAccessible() const
{
	auto manager = thisManager();
	if (manager)
		return manager->networkAccessible();

	return QNetworkAccessManager::NetworkAccessibility(0);
}

QScriptValue ScriptQNetworkAccessManager::head(const QNetworkRequest &request)
{
	auto manager = thisManager();
	if (!manager)
		return QScriptValue::NullValue;

	return wrappedReply(manager->head(request));
}

QScriptValue ScriptQNetworkAccessManager::get(const QNetworkRequest &request)
{
	auto manager = thisManager();
	if (!manager)
		return QScriptValue::NullValue;

	return wrappedReply(manager->get(request));
}

QScriptValue ScriptQNetworkAccessManager::post(
	const QNetworkRequest &request, const QScriptValue &data)
{
	auto manager = thisManager();
	if (!manager)
		return QScriptValue::NullValue;

	if (data.isNull() || data.isUndefined())
		return wrappedReply(manager->post(request, QByteArray()));

	if (data.isQObject())
	{
		auto device = qobject_cast<QIODevice *>(data.toQObject());

		if (nullptr != device)
			return wrappedReply(manager->post(request, device));
	}

	QByteArray ba;
	ScriptQByteArray::fromScriptValue(data, ba);
	return wrappedReply(manager->post(request, ba));
}

QScriptValue ScriptQNetworkAccessManager::put(
	const QNetworkRequest &request, const QScriptValue &data)
{
	auto manager = thisManager();
	if (!manager)
		return QScriptValue::NullValue;

	if (data.isNull() || data.isUndefined())
		return wrappedReply(manager->put(request, QByteArray()));

	if (data.isQObject())
	{
		auto device = qobject_cast<QIODevice *>(data.toQObject());

		if (nullptr != device)
			return wrappedReply(manager->put(request, device));
	}

	QByteArray ba;
	ScriptQByteArray::fromScriptValue(data, ba);
	return wrappedReply(manager->put(request, ba));
}

QScriptValue ScriptQNetworkAccessManager::deleteResource(
	const QNetworkRequest &request)
{
	auto manager = thisManager();
	if (!manager)
		return QScriptValue::NullValue;

	return wrappedReply(manager->deleteResource(request));
}

void ScriptQNetworkAccessManager::connectToHostEncrypted(
	const QString &hostName, quint16 port)
{
	auto manager = thisManager();
	if (!manager)
		return;

	manager->connectToHostEncrypted(hostName, port);
}

void ScriptQNetworkAccessManager::connectToHost(
	const QString &hostName, quint16 port)
{
	auto manager = thisManager();
	if (!manager)
		return;

	manager->connectToHost(hostName, port);
}

QString ScriptQNetworkAccessManager::className()
{
	return QSTRKEY(QNetworkAccessManager);
}

QScriptValue ScriptQNetworkAccessManager::wrappedReply(QNetworkReply *reply)
{
	return WrapQObject(reply, engine());
}

QScriptValue ScriptQNetworkAccessManager::Construct(
	QScriptContext *context, QScriptEngine *engine)
{
	if (context->argumentCount() != 0)
	{
		return ThrowBadNumberOfArguments(context);
	}

	return WrapQObject(new QNetworkAccessManager, engine, context);
}

QNetworkAccessManager *ScriptQNetworkAccessManager::thisManager() const
{
	return qobject_cast<QNetworkAccessManager *>(thisObject().toQObject());
}
}
