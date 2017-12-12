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

#include "ScriptQNetworkReply.h"

#include "ScriptQByteArray.h"
#include "ScriptUtils.h"

namespace Banana
{
namespace Script
{
struct NetworkReply : public QNetworkReply
{
	static void RegisterEnums(QScriptEngine *engine, QScriptValue &to)
	{
		static bool convertersRegistered = false;
		if (not convertersRegistered)
		{
			convertersRegistered = true;
			QMetaType::registerConverter<NetworkError, qint32>();
		}

		qScriptRegisterMetaType(engine, EnumToScriptValue<NetworkError>,
			EnumFromScriptValue<NetworkError>);

		SCRIPT_REG_ENUM(to, NoError);
		SCRIPT_REG_ENUM(to, ConnectionRefusedError);
		SCRIPT_REG_ENUM(to, RemoteHostClosedError);
		SCRIPT_REG_ENUM(to, HostNotFoundError);
		SCRIPT_REG_ENUM(to, TimeoutError);
		SCRIPT_REG_ENUM(to, OperationCanceledError);
		SCRIPT_REG_ENUM(to, SslHandshakeFailedError);
		SCRIPT_REG_ENUM(to, TemporaryNetworkFailureError);
		SCRIPT_REG_ENUM(to, NetworkSessionFailedError);
		SCRIPT_REG_ENUM(to, BackgroundRequestNotAllowedError);
		SCRIPT_REG_ENUM(to, TooManyRedirectsError);
		SCRIPT_REG_ENUM(to, InsecureRedirectError);
		SCRIPT_REG_ENUM(to, UnknownNetworkError);
		SCRIPT_REG_ENUM(to, ProxyConnectionRefusedError);
		SCRIPT_REG_ENUM(to, ProxyConnectionClosedError);
		SCRIPT_REG_ENUM(to, ProxyNotFoundError);
		SCRIPT_REG_ENUM(to, ProxyTimeoutError);
		SCRIPT_REG_ENUM(to, ProxyAuthenticationRequiredError);
		SCRIPT_REG_ENUM(to, UnknownProxyError);
		SCRIPT_REG_ENUM(to, ContentAccessDenied);
		SCRIPT_REG_ENUM(to, ContentOperationNotPermittedError);
		SCRIPT_REG_ENUM(to, ContentNotFoundError);
		SCRIPT_REG_ENUM(to, AuthenticationRequiredError);
		SCRIPT_REG_ENUM(to, ContentReSendError);
		SCRIPT_REG_ENUM(to, ContentConflictError);
		SCRIPT_REG_ENUM(to, ContentGoneError);
		SCRIPT_REG_ENUM(to, UnknownContentError);
		SCRIPT_REG_ENUM(to, ProtocolUnknownError);
		SCRIPT_REG_ENUM(to, ProtocolInvalidOperationError);
		SCRIPT_REG_ENUM(to, ProtocolFailure);
		SCRIPT_REG_ENUM(to, InternalServerError);
		SCRIPT_REG_ENUM(to, OperationNotImplementedError);
		SCRIPT_REG_ENUM(to, ServiceUnavailableError);
		SCRIPT_REG_ENUM(to, UnknownServerError);
	}
};
}

using namespace Script;

void ScriptQNetworkReply::Register(QScriptEngine *engine)
{
	auto proto = NewQObjectPrototype<ScriptQNetworkReply>(engine);
	proto.setPrototype(engine->defaultPrototype(qMetaTypeId<QIODevice *>()));
	engine->setDefaultPrototype(qMetaTypeId<QNetworkReply *>(), proto);

	auto qNetworkReply = engine->newObject();
	qNetworkReply.setProperty(QSTRKEY(prototype), proto, STATIC_SCRIPT_VALUE);

	NetworkReply::RegisterEnums(engine, qNetworkReply);

	engine->globalObject().setProperty(
		className(), qNetworkReply, STATIC_SCRIPT_VALUE);
}

ScriptQNetworkReply::ScriptQNetworkReply(QObject *parent)
	: ScriptQIODevice(parent)
{
}

qint64 ScriptQNetworkReply::readBufferSize() const
{
	auto r = thisReply();
	if (r)
		return r->readBufferSize();

	return -1;
}

void ScriptQNetworkReply::setReadBufferSize(qint64 size)
{
	auto r = thisReply();
	if (r)
		r->setReadBufferSize(size);
}

QScriptValue ScriptQNetworkReply::manager() const
{
	auto r = thisReply();
	if (r)
		return WrapQObject(r->manager(), engine());

	return QScriptValue::NullValue;
}

QNetworkAccessManager::Operation ScriptQNetworkReply::operation() const
{
	auto r = thisReply();
	if (r)
		return r->operation();

	return QNetworkAccessManager::UnknownOperation;
}

QNetworkRequest ScriptQNetworkReply::request() const
{
	auto r = thisReply();
	if (r)
		return r->request();

	return QNetworkRequest();
}

QNetworkReply::NetworkError ScriptQNetworkReply::resultCode() const
{
	auto r = thisReply();
	if (r)
		return r->error();

	return QNetworkReply::NoError;
}

bool ScriptQNetworkReply::isFinished() const
{
	auto r = thisReply();
	if (r)
		return r->isFinished();

	return false;
}

bool ScriptQNetworkReply::isRunning() const
{
	auto r = thisReply();
	if (r)
		return r->isRunning();

	return false;
}

QUrl ScriptQNetworkReply::url() const
{
	auto r = thisReply();
	if (r)
		return r->url();

	return QUrl();
}

QString ScriptQNetworkReply::toString() const
{
	auto r = thisReply();
	if (r)
	{
		return QStringLiteral("%1('%2')")
			.arg(className())
			.arg(r->url().toString());
	}

	return className();
}

QVariant ScriptQNetworkReply::attribute(QNetworkRequest::Attribute code) const
{
	auto r = thisReply();
	if (r)
		return r->attribute(code);

	return QVariant();
}

QVariant ScriptQNetworkReply::header(QNetworkRequest::KnownHeaders header) const
{
	auto r = thisReply();
	if (r)
		return r->header(header);

	return QVariant();
}

bool ScriptQNetworkReply::hasRawHeader(const QByteArray &headerName) const
{
	auto r = thisReply();
	if (r)
		return r->hasRawHeader(headerName);

	return false;
}

QScriptValue ScriptQNetworkReply::rawHeaderList() const
{
	auto r = thisReply();
	if (!r)
		return QScriptValue();

	auto list = r->rawHeaderList();

	int count = list.length();
	auto engine = this->engine();
	auto result = engine->newArray(count);

	for (int i = 0; i < count; i++)
	{
		result.setProperty(
			i, ScriptQByteArray::toScriptValue(engine, list.at(i)));
	}

	return result;
}

QByteArray ScriptQNetworkReply::rawHeader(const QByteArray &headerName) const
{
	auto r = thisReply();
	if (r)
		return r->rawHeader(headerName);

	return QByteArray();
}

QString ScriptQNetworkReply::className()
{
	return QSTRKEY(QNetworkReply);
}

QNetworkReply *ScriptQNetworkReply::thisReply() const
{
	return qobject_cast<QNetworkReply *>(thisObject().toQObject());
}
}
