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

#include "NetworkManager.h"

#include "ScriptUtils.h"
#include "ScriptQByteArray.h"

namespace Banana
{
namespace Script
{
void ScriptQNetworkRequest::Register(QScriptEngine *engine)
{
	qScriptRegisterMetaType(engine,
		EnumToScriptValue<QNetworkRequest::KnownHeaders>,
		EnumFromScriptValue<QNetworkRequest::KnownHeaders>);

	qScriptRegisterMetaType(engine,
		EnumToScriptValue<QNetworkRequest::Attribute>,
		EnumFromScriptValue<QNetworkRequest::Attribute>);

	qScriptRegisterMetaType(engine,
		EnumToScriptValue<QNetworkRequest::Priority>,
		EnumFromScriptValue<QNetworkRequest::Priority>);

	static bool convertersRegistered = false;
	if (!convertersRegistered)
	{
		convertersRegistered = true;
		QMetaType::registerConverter<QNetworkRequest::KnownHeaders, qint32>();
		QMetaType::registerConverter<QNetworkRequest::Attribute, qint32>();
		QMetaType::registerConverter<QNetworkRequest::Priority, qint32>();
	}

	auto qRequestObject =
		engine->newFunction(ScriptQNetworkRequest::Constructor);

	SCRIPT_REG_ENUM(qRequestObject, ContentTypeHeader);
	SCRIPT_REG_ENUM(qRequestObject, ContentLengthHeader);
	SCRIPT_REG_ENUM(qRequestObject, LocationHeader);
	SCRIPT_REG_ENUM(qRequestObject, LastModifiedHeader);
	SCRIPT_REG_ENUM(qRequestObject, CookieHeader);
	SCRIPT_REG_ENUM(qRequestObject, SetCookieHeader);
	SCRIPT_REG_ENUM(qRequestObject, ContentDispositionHeader);
	SCRIPT_REG_ENUM(qRequestObject, UserAgentHeader);
	SCRIPT_REG_ENUM(qRequestObject, ServerHeader);
	SCRIPT_REG_ENUM(qRequestObject, HttpStatusCodeAttribute);
	SCRIPT_REG_ENUM(qRequestObject, HttpReasonPhraseAttribute);
	SCRIPT_REG_ENUM(qRequestObject, RedirectionTargetAttribute);
	SCRIPT_REG_ENUM(qRequestObject, ConnectionEncryptedAttribute);
	SCRIPT_REG_ENUM(qRequestObject, CacheLoadControlAttribute);
	SCRIPT_REG_ENUM(qRequestObject, CacheSaveControlAttribute);
	SCRIPT_REG_ENUM(qRequestObject, SourceIsFromCacheAttribute);
	SCRIPT_REG_ENUM(qRequestObject, DoNotBufferUploadDataAttribute);
	SCRIPT_REG_ENUM(qRequestObject, HttpPipeliningAllowedAttribute);
	SCRIPT_REG_ENUM(qRequestObject, HttpPipeliningWasUsedAttribute);
	SCRIPT_REG_ENUM(qRequestObject, CustomVerbAttribute);
	SCRIPT_REG_ENUM(qRequestObject, CookieLoadControlAttribute);
	SCRIPT_REG_ENUM(qRequestObject, AuthenticationReuseAttribute);
	SCRIPT_REG_ENUM(qRequestObject, CookieSaveControlAttribute);
	SCRIPT_REG_ENUM(qRequestObject, MaximumDownloadBufferSizeAttribute);
	SCRIPT_REG_ENUM(qRequestObject, DownloadBufferAttribute);
	SCRIPT_REG_ENUM(qRequestObject, SynchronousRequestAttribute);
	SCRIPT_REG_ENUM(qRequestObject, BackgroundRequestAttribute);
	SCRIPT_REG_ENUM(qRequestObject, SpdyAllowedAttribute);
	SCRIPT_REG_ENUM(qRequestObject, SpdyWasUsedAttribute);
	SCRIPT_REG_ENUM(qRequestObject, EmitAllUploadProgressSignalsAttribute);
	SCRIPT_REG_ENUM(qRequestObject, FollowRedirectsAttribute);
	SCRIPT_REG_ENUM(qRequestObject, User);
	SCRIPT_REG_ENUM(qRequestObject, UserMax);
	SCRIPT_REG_ENUM(qRequestObject, HighPriority);
	SCRIPT_REG_ENUM(qRequestObject, NormalPriority);
	SCRIPT_REG_ENUM(qRequestObject, LowPriority);

	engine->globalObject().setProperty(
		QSTRKEY(QNetworkRequest), qRequestObject, STATIC_SCRIPT_VALUE);
}

ScriptQNetworkRequest::ScriptQNetworkRequest(
	const QNetworkRequest &request, QScriptEngine *engine)
	: QNetworkRequest(request)
	, engine(engine)
{
}

QScriptValue ScriptQNetworkRequest::rawHeaderList() const
{
	auto list = QNetworkRequest::rawHeaderList();

	int count = list.length();
	auto result = engine->newArray(count);

	for (int i = 0; i < count; i++)
	{
		result.setProperty(i, engine->toScriptValue(list.at(0)));
	}

	return result;
}

bool ScriptQNetworkRequest::hasRawHeader(const QScriptValue &headerName) const
{
	auto ba = qscriptvalue_cast<QByteArray *>(headerName.data());

	if (nullptr != ba)
		return QNetworkRequest::hasRawHeader(*ba);

	return QNetworkRequest::hasRawHeader(headerName.toString().toUtf8());
}

QByteArray ScriptQNetworkRequest::rawHeader(
	const QScriptValue &headerName) const
{
	auto ba = qscriptvalue_cast<QByteArray *>(headerName.data());

	if (nullptr != ba)
		return QNetworkRequest::rawHeader(*ba);

	return QNetworkRequest::rawHeader(headerName.toString().toUtf8());
}

void ScriptQNetworkRequest::setRawHeader(
	const QScriptValue &headerName, const QScriptValue &value)
{
	auto vba = qscriptvalue_cast<QByteArray *>(value.data());
	auto ba = qscriptvalue_cast<QByteArray *>(headerName.data());

	auto valueBa = (nullptr != vba)
		? QByteArray::fromRawData(vba->data(), vba->size())
		: value.toString().toUtf8();

	if (nullptr != ba)
	{
		QNetworkRequest::setRawHeader(*ba, valueBa);
	} else
	{
		QNetworkRequest::setRawHeader(headerName.toString().toUtf8(), valueBa);
	}
}

QString ScriptQNetworkRequest::toString() const
{
	return QStringLiteral("QNetworkRequest(\"%1\")").arg(url().toString());
}

QScriptValue ScriptQNetworkRequest::Constructor(
	QScriptContext *context, QScriptEngine *engine)
{
	QNetworkRequest data;
	bool copy = false;
	QScriptValue argument;
	switch (context->argumentCount())
	{
		case 1:
		{
			argument = context->argument(0);
			auto url = ScriptQUrl::fromScriptValue(argument);
			if (url)
			{
				data = QNetworkRequest(*url);
			} else if (argument.isQObject())
			{
				auto source =
					dynamic_cast<ScriptQNetworkRequest *>(argument.toQObject());

				if (nullptr == source)
					return ThrowIncompatibleArgumentType(context, 1);

				data = *source;
			} else if (argument.isObject())
			{
				copy = true;
			} else if (!argument.isNull() && !argument.isUndefined())
			{
				data = QNetworkRequest(argument.toString());
			}
		} // fall through

		case 0:
		{
			auto request = new ScriptQNetworkRequest(data, engine);

			QScriptValue result;
			if (!context->isCalledAsConstructor())
			{
				result = engine->newQObject(context->thisObject(), request,
					QScriptEngine::ScriptOwnership,
					QScriptEngine::ExcludeChildObjects |
						QScriptEngine::ExcludeDeleteLater |
						QScriptEngine::SkipMethodsInEnumeration);
			} else
			{
				result =
					engine->newQObject(request, QScriptEngine::ScriptOwnership,
						QScriptEngine::ExcludeChildObjects |
							QScriptEngine::ExcludeDeleteLater |
							QScriptEngine::SkipMethodsInEnumeration);
			}

			if (copy)
				CopyScriptProperties(argument, result);

			return result;
		}
	}

	return ThrowBadNumberOfArguments(context);
}

QScriptValue ScriptQNetworkRequest::ToScriptValue(
	QScriptEngine *engine, const QNetworkRequest &in)
{
	return engine->newQObject(new ScriptQNetworkRequest(in, engine),
		QScriptEngine::ScriptOwnership,
		QScriptEngine::SkipMethodsInEnumeration);
}

void ScriptQNetworkRequest::FromScriptValue(
	const QScriptValue &object, QNetworkRequest &out)
{
	auto request = dynamic_cast<ScriptQNetworkRequest *>(object.toQObject());

	if (nullptr != request)
		out = *request;
	else
	{
		out = QNetworkRequest();
	}
}

void ScriptQNetworkAccessManager::Register(QScriptEngine *engine)
{
	auto proto = engine->newQObject(new ScriptQNetworkAccessManager(engine),
		QScriptEngine::QtOwnership,
		QScriptEngine::ExcludeChildObjects | QScriptEngine::ExcludeDeleteLater |
			QScriptEngine::SkipMethodsInEnumeration);

	engine->setDefaultPrototype(qMetaTypeId<QNetworkAccessManager *>(), proto);

	qScriptRegisterMetaType(engine,
		EnumToScriptValue<QNetworkAccessManager::Operation>,
		EnumFromScriptValue<QNetworkAccessManager::Operation>);

	qScriptRegisterMetaType(engine,
		EnumToScriptValue<QNetworkAccessManager::NetworkAccessibility>,
		EnumFromScriptValue<QNetworkAccessManager::NetworkAccessibility>);

	static bool convertersRegistered = false;
	if (!convertersRegistered)
	{
		convertersRegistered = true;
		QMetaType::registerConverter<QNetworkAccessManager::Operation,
			qint32>();
		QMetaType::registerConverter<
			QNetworkAccessManager::NetworkAccessibility, qint32>();
	}

	enum
	{
		HeadOperation = QNetworkAccessManager::HeadOperation,
		GetOperation = QNetworkAccessManager::GetOperation,
		PutOperation = QNetworkAccessManager::PutOperation,
		PostOperation = QNetworkAccessManager::PostOperation,
		DeleteOperation = QNetworkAccessManager::DeleteOperation,
		CustomOperation = QNetworkAccessManager::CustomOperation,
		UnknownOperation = QNetworkAccessManager::UnknownOperation,
		UnknownAccessibility = QNetworkAccessManager::UnknownAccessibility,
		NotAccessible = QNetworkAccessManager::NotAccessible,
		Accessible = QNetworkAccessManager::Accessible
	};

	auto qManagerObject = engine->newFunction(Construct, proto);

	SCRIPT_REG_ENUM(qManagerObject, HeadOperation);
	SCRIPT_REG_ENUM(qManagerObject, GetOperation);
	SCRIPT_REG_ENUM(qManagerObject, PutOperation);
	SCRIPT_REG_ENUM(qManagerObject, PostOperation);
	SCRIPT_REG_ENUM(qManagerObject, DeleteOperation);
	SCRIPT_REG_ENUM(qManagerObject, CustomOperation);
	SCRIPT_REG_ENUM(qManagerObject, UnknownOperation);
	SCRIPT_REG_ENUM(qManagerObject, UnknownAccessibility);
	SCRIPT_REG_ENUM(qManagerObject, NotAccessible);
	SCRIPT_REG_ENUM(qManagerObject, Accessible);

	engine->globalObject().setProperty(
		QSTRKEY(QNetworkAccessManager), qManagerObject, STATIC_SCRIPT_VALUE);

	ScriptQNetworkRequest::Register(engine);
	ScriptQNetworkReply::Register(engine);
}

ScriptQNetworkAccessManager::ScriptQNetworkAccessManager(QObject *parent)
	: QObject(parent)
{
}

QScriptValue ScriptQNetworkAccessManager::wrappedReply(QNetworkReply *reply)
{
	return engine()->newQObject(reply, QScriptEngine::AutoOwnership);
}

QScriptValue ScriptQNetworkAccessManager::head(const QScriptValue &request)
{
	auto manager = thisManager();
	if (!manager)
		return QScriptValue::NullValue;

	auto req = dynamic_cast<ScriptQNetworkRequest *>(request.toQObject());
	if (nullptr == req)
	{
		ThrowIncompatibleArgumentType(context(), 1);
		return QScriptValue::NullValue;
	}

	return wrappedReply(manager->head(*req));
}

QScriptValue ScriptQNetworkAccessManager::get(const QScriptValue &request)
{
	auto manager = thisManager();
	if (!manager)
		return QScriptValue::NullValue;

	auto req = dynamic_cast<ScriptQNetworkRequest *>(request.toQObject());
	if (nullptr == req)
	{
		ThrowIncompatibleArgumentType(context(), 1);
		return QScriptValue::NullValue;
	}
	return wrappedReply(manager->get(*req));
}

QScriptValue ScriptQNetworkAccessManager::post(
	const QScriptValue &request, const QScriptValue &data)
{
	auto manager = thisManager();
	if (!manager)
		return QScriptValue::NullValue;

	auto req = dynamic_cast<ScriptQNetworkRequest *>(request.toQObject());
	if (nullptr == req)
	{
		ThrowIncompatibleArgumentType(context(), 1);
		return QScriptValue::NullValue;
	}

	if (data.isNull() || data.isUndefined())
		return wrappedReply(manager->post(*req, QByteArray()));

	if (data.isQObject())
	{
		auto device = dynamic_cast<QIODevice *>(data.toQObject());

		if (nullptr != device)
			return wrappedReply(manager->post(*req, device));
	}

	auto ba = qscriptvalue_cast<QByteArray *>(data.data());

	if (nullptr != ba)
		return wrappedReply(manager->post(*req, *ba));

	return wrappedReply(manager->post(*req, data.toString().toUtf8()));
}

QScriptValue ScriptQNetworkAccessManager::put(
	const QScriptValue &request, const QScriptValue &data)
{
	auto manager = thisManager();
	if (!manager)
		return QScriptValue::NullValue;

	auto req = dynamic_cast<ScriptQNetworkRequest *>(request.toQObject());
	if (nullptr == req)
	{
		ThrowIncompatibleArgumentType(context(), 1);
		return QScriptValue::NullValue;
	}

	if (data.isNull() || data.isUndefined())
		return wrappedReply(manager->put(*req, QByteArray()));

	if (data.isQObject())
	{
		auto device = dynamic_cast<QIODevice *>(data.toQObject());

		if (nullptr != device)
			return wrappedReply(manager->put(*req, device));
	}

	auto ba = qscriptvalue_cast<QByteArray *>(data.data());

	if (nullptr != ba)
		return wrappedReply(manager->put(*req, *ba));

	return wrappedReply(manager->put(*req, data.toString().toUtf8()));
}

QScriptValue ScriptQNetworkAccessManager::deleteResource(
	const QScriptValue &request)
{
	auto manager = thisManager();
	if (!manager)
		return QScriptValue::NullValue;

	auto req = dynamic_cast<ScriptQNetworkRequest *>(request.toQObject());
	if (nullptr == req)
	{
		ThrowIncompatibleArgumentType(context(), 1);
		return QScriptValue::NullValue;
	}

	return wrappedReply(manager->deleteResource(*req));
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

QString ScriptQNetworkAccessManager::toString() const
{
	return QSTRKEY(QNetworkAccessManager);
}

QScriptValue ScriptQNetworkAccessManager::Construct(
	QScriptContext *context, QScriptEngine *engine)
{
	int argumentCount = context->argumentCount();

	if (argumentCount != 0)
	{
		return ThrowBadNumberOfArguments(context);
	}

	auto newFile = new QNetworkAccessManager;

	if (!context->isCalledAsConstructor())
	{
		return engine->newQObject(context->thisObject(), newFile,
			QScriptEngine::ScriptOwnership,
			QScriptEngine::ExcludeChildObjects |
				QScriptEngine::ExcludeDeleteLater |
				QScriptEngine::SkipMethodsInEnumeration);
	}

	return engine->newQObject(newFile, QScriptEngine::ScriptOwnership,
		QScriptEngine::ExcludeChildObjects | QScriptEngine::ExcludeDeleteLater |
			QScriptEngine::SkipMethodsInEnumeration);
}

QNetworkAccessManager *ScriptQNetworkAccessManager::thisManager()
{
	return dynamic_cast<QNetworkAccessManager *>(thisObject().toQObject());
}

void ScriptQNetworkReply::Register(QScriptEngine *engine)
{
	auto proto = engine->newQObject(new ScriptQNetworkReply(engine),
		QScriptEngine::QtOwnership,
		QScriptEngine::ExcludeChildObjects | QScriptEngine::ExcludeDeleteLater);

	engine->setDefaultPrototype(qMetaTypeId<QNetworkReply *>(), proto);

	qScriptRegisterMetaType(engine,
		EnumToScriptValue<QNetworkReply::NetworkError>,
		EnumFromScriptValue<QNetworkReply::NetworkError>);

	static bool convertersRegistered = false;
	if (!convertersRegistered)
	{
		convertersRegistered = true;
		QMetaType::registerConverter<QNetworkReply::NetworkError, qint32>();
	}

	enum
	{
		NoError = QNetworkReply::NoError,
		ConnectionRefusedError = QNetworkReply::ConnectionRefusedError,
		RemoteHostClosedError = QNetworkReply::RemoteHostClosedError,
		HostNotFoundError = QNetworkReply::HostNotFoundError,
		TimeoutError = QNetworkReply::TimeoutError,
		OperationCanceledError = QNetworkReply::OperationCanceledError,
		SslHandshakeFailedError = QNetworkReply::SslHandshakeFailedError,
		TemporaryNetworkFailureError =
			QNetworkReply::TemporaryNetworkFailureError,
		NetworkSessionFailedError = QNetworkReply::NetworkSessionFailedError,
		BackgroundRequestNotAllowedError =
			QNetworkReply::BackgroundRequestNotAllowedError,
		TooManyRedirectsError = QNetworkReply::TooManyRedirectsError,
		InsecureRedirectError = QNetworkReply::InsecureRedirectError,
		UnknownNetworkError = QNetworkReply::UnknownNetworkError,
		ProxyConnectionRefusedError =
			QNetworkReply::ProxyConnectionRefusedError,
		ProxyConnectionClosedError = QNetworkReply::ProxyConnectionClosedError,
		ProxyNotFoundError = QNetworkReply::ProxyNotFoundError,
		ProxyTimeoutError = QNetworkReply::ProxyTimeoutError,
		ProxyAuthenticationRequiredError =
			QNetworkReply::ProxyAuthenticationRequiredError,
		UnknownProxyError = QNetworkReply::UnknownProxyError,
		ContentAccessDenied = QNetworkReply::ContentAccessDenied,
		ContentOperationNotPermittedError =
			QNetworkReply::ContentOperationNotPermittedError,
		ContentNotFoundError = QNetworkReply::ContentNotFoundError,
		AuthenticationRequiredError =
			QNetworkReply::AuthenticationRequiredError,
		ContentReSendError = QNetworkReply::ContentReSendError,
		ContentConflictError = QNetworkReply::ContentConflictError,
		ContentGoneError = QNetworkReply::ContentGoneError,
		UnknownContentError = QNetworkReply::UnknownContentError,
		ProtocolUnknownError = QNetworkReply::ProtocolUnknownError,
		ProtocolInvalidOperationError =
			QNetworkReply::ProtocolInvalidOperationError,
		ProtocolFailure = QNetworkReply::ProtocolFailure,
		InternalServerError = QNetworkReply::InternalServerError,
		OperationNotImplementedError =
			QNetworkReply::OperationNotImplementedError,
		ServiceUnavailableError = QNetworkReply::ServiceUnavailableError,
		UnknownServerError = QNetworkReply::UnknownServerError,
	};

	auto qReplyObject = engine->newObject();

	SCRIPT_REG_ENUM(qReplyObject, NoError);
	SCRIPT_REG_ENUM(qReplyObject, ConnectionRefusedError);
	SCRIPT_REG_ENUM(qReplyObject, RemoteHostClosedError);
	SCRIPT_REG_ENUM(qReplyObject, HostNotFoundError);
	SCRIPT_REG_ENUM(qReplyObject, TimeoutError);
	SCRIPT_REG_ENUM(qReplyObject, OperationCanceledError);
	SCRIPT_REG_ENUM(qReplyObject, SslHandshakeFailedError);
	SCRIPT_REG_ENUM(qReplyObject, TemporaryNetworkFailureError);
	SCRIPT_REG_ENUM(qReplyObject, NetworkSessionFailedError);
	SCRIPT_REG_ENUM(qReplyObject, BackgroundRequestNotAllowedError);
	SCRIPT_REG_ENUM(qReplyObject, TooManyRedirectsError);
	SCRIPT_REG_ENUM(qReplyObject, InsecureRedirectError);
	SCRIPT_REG_ENUM(qReplyObject, UnknownNetworkError);
	SCRIPT_REG_ENUM(qReplyObject, ProxyConnectionRefusedError);
	SCRIPT_REG_ENUM(qReplyObject, ProxyConnectionClosedError);
	SCRIPT_REG_ENUM(qReplyObject, ProxyNotFoundError);
	SCRIPT_REG_ENUM(qReplyObject, ProxyTimeoutError);
	SCRIPT_REG_ENUM(qReplyObject, ProxyAuthenticationRequiredError);
	SCRIPT_REG_ENUM(qReplyObject, UnknownProxyError);
	SCRIPT_REG_ENUM(qReplyObject, ContentAccessDenied);
	SCRIPT_REG_ENUM(qReplyObject, ContentOperationNotPermittedError);
	SCRIPT_REG_ENUM(qReplyObject, ContentNotFoundError);
	SCRIPT_REG_ENUM(qReplyObject, AuthenticationRequiredError);
	SCRIPT_REG_ENUM(qReplyObject, ContentReSendError);
	SCRIPT_REG_ENUM(qReplyObject, ContentConflictError);
	SCRIPT_REG_ENUM(qReplyObject, ContentGoneError);
	SCRIPT_REG_ENUM(qReplyObject, UnknownContentError);
	SCRIPT_REG_ENUM(qReplyObject, ProtocolUnknownError);
	SCRIPT_REG_ENUM(qReplyObject, ProtocolInvalidOperationError);
	SCRIPT_REG_ENUM(qReplyObject, ProtocolFailure);
	SCRIPT_REG_ENUM(qReplyObject, InternalServerError);
	SCRIPT_REG_ENUM(qReplyObject, OperationNotImplementedError);
	SCRIPT_REG_ENUM(qReplyObject, ServiceUnavailableError);
	SCRIPT_REG_ENUM(qReplyObject, UnknownServerError);

	engine->globalObject().setProperty(
		QSTRKEY(QNetworkReply), qReplyObject, STATIC_SCRIPT_VALUE);
}

ScriptQNetworkReply::ScriptQNetworkReply(QObject *parent)
	: QObject(parent)
{
}

qint64 ScriptQNetworkReply::readBufferSize() const
{
	auto reply = thisReply();
	if (!reply)
		return 0;

	return reply->readBufferSize();
}

void ScriptQNetworkReply::setReadBufferSize(qint64 size)
{
	auto reply = thisReply();
	if (!reply)
		return;

	reply->setReadBufferSize(size);
}

qint64 ScriptQNetworkReply::bytesAvailable() const
{
	auto reply = thisReply();
	if (!reply)
		return 0;

	return reply->bytesAvailable();
}

QNetworkAccessManager *ScriptQNetworkReply::manager() const
{
	auto reply = thisReply();
	if (!reply)
		return nullptr;

	return reply->manager();
}

QNetworkAccessManager::Operation ScriptQNetworkReply::operation() const
{
	auto reply = thisReply();
	if (!reply)
		return QNetworkAccessManager::UnknownOperation;

	return reply->operation();
}

QNetworkRequest ScriptQNetworkReply::request() const
{
	auto reply = thisReply();
	if (!reply)
		return QNetworkRequest();

	return reply->request();
}

QNetworkReply::NetworkError ScriptQNetworkReply::error() const
{
	auto reply = thisReply();
	if (!reply)
		return QNetworkReply::NoError;

	return reply->error();
}

QString ScriptQNetworkReply::errorString() const
{
	auto reply = thisReply();
	if (!reply)
		return QString();

	return reply->errorString();
}

bool ScriptQNetworkReply::isFinished() const
{
	auto reply = thisReply();
	if (!reply)
		return true;

	return reply->isFinished();
}

bool ScriptQNetworkReply::isRunning() const
{
	auto reply = thisReply();
	if (!reply)
		return false;

	return reply->isRunning();
}

bool ScriptQNetworkReply::isOpen() const
{
	auto reply = thisReply();
	if (!reply)
		return false;

	return reply->isOpen();
}

QUrl ScriptQNetworkReply::url() const
{
	auto reply = thisReply();
	if (!reply)
		return QUrl();

	return reply->url();
}

bool ScriptQNetworkReply::isTransactionStarted() const
{
	auto reply = thisReply();
	if (!reply)
		return false;

	return reply->isTransactionStarted();
}

void ScriptQNetworkReply::close()
{
	auto reply = thisReply();
	if (!reply)
		return;

	reply->close();
}

QVariant ScriptQNetworkReply::attribute(QNetworkRequest::Attribute code) const
{
	auto reply = thisReply();
	if (!reply)
		return QVariant();

	return reply->attribute(code);
}

QVariant ScriptQNetworkReply::header(QNetworkRequest::KnownHeaders header) const
{
	auto reply = thisReply();
	if (!reply)
		return QVariant();

	return reply->header(header);
}

bool ScriptQNetworkReply::hasRawHeader(const QScriptValue &headerName) const
{
	auto reply = thisReply();
	if (!reply)
		return false;

	auto ba = qscriptvalue_cast<QByteArray *>(headerName.data());

	if (nullptr != ba)
		return reply->hasRawHeader(*ba);

	return reply->hasRawHeader(headerName.toString().toUtf8());
}

QScriptValue ScriptQNetworkReply::rawHeaderList() const
{
	auto reply = thisReply();
	if (!reply)
		return QScriptValue();

	auto list = reply->rawHeaderList();

	auto engine = this->engine();

	int count = list.length();
	auto result = engine->newArray(count);

	for (int i = 0; i < count; i++)
	{
		result.setProperty(i, engine->toScriptValue(list.at(0)));
	}

	return result;
}

QByteArray ScriptQNetworkReply::rawHeader(const QScriptValue &headerName) const
{
	auto reply = thisReply();
	if (!reply)
		return QByteArray();

	auto ba = qscriptvalue_cast<QByteArray *>(headerName.data());

	if (nullptr != ba)
		return reply->rawHeader(*ba);

	return reply->rawHeader(headerName.toString().toUtf8());
}

void ScriptQNetworkReply::startTransaction()
{
	auto reply = thisReply();
	if (!reply)
		return;

	reply->startTransaction();
}

void ScriptQNetworkReply::commitTransaction()
{
	auto reply = thisReply();
	if (!reply)
		return;

	reply->commitTransaction();
}

void ScriptQNetworkReply::rollbackTransaction()
{
	auto reply = thisReply();
	if (!reply)
		return;

	reply->rollbackTransaction();
}

QByteArray ScriptQNetworkReply::peek(int maxlen)
{
	auto reply = thisReply();
	if (!reply)
		return QByteArray();

	return reply->peek(maxlen);
}

QByteArray ScriptQNetworkReply::read(int maxlen)
{
	auto reply = thisReply();
	if (!reply)
		return QByteArray();

	return reply->read(maxlen);
}

QByteArray ScriptQNetworkReply::readAll()
{
	auto reply = thisReply();
	if (!reply)
		return QByteArray();

	return reply->readAll();
}

QByteArray ScriptQNetworkReply::readLine(int maxlen)
{
	auto reply = thisReply();
	if (!reply)
		return QByteArray();

	return reply->readLine(maxlen);
}

QString ScriptQNetworkReply::toString() const
{
	auto reply = thisReply();
	if (nullptr != reply)
		return QStringLiteral(
			CSTRKEY(QNetworkReply) QT_UNICODE_LITERAL("(\"%1\")"))
			.arg(reply->url().toString());

	return QSTRKEY(QNetworkReply);
}

QNetworkReply *ScriptQNetworkReply::thisReply() const
{
	return dynamic_cast<QNetworkReply *>(thisObject().toQObject());
}
}
}
