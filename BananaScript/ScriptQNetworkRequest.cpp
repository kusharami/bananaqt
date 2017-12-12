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

#include "ScriptQNetworkRequest.h"

#include "ScriptUtils.h"
#include "ScriptQByteArray.h"

namespace Banana
{
namespace Script
{
struct NetworkRequest : public QNetworkRequest
{
	static void RegisterEnums(QScriptEngine *engine, QScriptValue &to)
	{
		static bool convertersRegistered = false;
		if (not convertersRegistered)
		{
			convertersRegistered = true;
			QMetaType::registerConverter<KnownHeaders, qint32>();
			QMetaType::registerConverter<Attribute, qint32>();
			QMetaType::registerConverter<Priority, qint32>();
		}

		qScriptRegisterMetaType(engine, EnumToScriptValue<KnownHeaders>,
			EnumFromScriptValue<KnownHeaders>);

		qScriptRegisterMetaType(engine, EnumToScriptValue<Attribute>,
			EnumFromScriptValue<Attribute>);

		qScriptRegisterMetaType(
			engine, EnumToScriptValue<Priority>, EnumFromScriptValue<Priority>);

		SCRIPT_REG_ENUM(to, ContentTypeHeader);
		SCRIPT_REG_ENUM(to, ContentLengthHeader);
		SCRIPT_REG_ENUM(to, LocationHeader);
		SCRIPT_REG_ENUM(to, LastModifiedHeader);
		SCRIPT_REG_ENUM(to, CookieHeader);
		SCRIPT_REG_ENUM(to, SetCookieHeader);
		SCRIPT_REG_ENUM(to, ContentDispositionHeader);
		SCRIPT_REG_ENUM(to, UserAgentHeader);
		SCRIPT_REG_ENUM(to, ServerHeader);
		SCRIPT_REG_ENUM(to, HttpStatusCodeAttribute);
		SCRIPT_REG_ENUM(to, HttpReasonPhraseAttribute);
		SCRIPT_REG_ENUM(to, RedirectionTargetAttribute);
		SCRIPT_REG_ENUM(to, ConnectionEncryptedAttribute);
		SCRIPT_REG_ENUM(to, CacheLoadControlAttribute);
		SCRIPT_REG_ENUM(to, CacheSaveControlAttribute);
		SCRIPT_REG_ENUM(to, SourceIsFromCacheAttribute);
		SCRIPT_REG_ENUM(to, DoNotBufferUploadDataAttribute);
		SCRIPT_REG_ENUM(to, HttpPipeliningAllowedAttribute);
		SCRIPT_REG_ENUM(to, HttpPipeliningWasUsedAttribute);
		SCRIPT_REG_ENUM(to, CustomVerbAttribute);
		SCRIPT_REG_ENUM(to, CookieLoadControlAttribute);
		SCRIPT_REG_ENUM(to, AuthenticationReuseAttribute);
		SCRIPT_REG_ENUM(to, CookieSaveControlAttribute);
		SCRIPT_REG_ENUM(to, MaximumDownloadBufferSizeAttribute);
		SCRIPT_REG_ENUM(to, DownloadBufferAttribute);
		SCRIPT_REG_ENUM(to, SynchronousRequestAttribute);
		SCRIPT_REG_ENUM(to, BackgroundRequestAttribute);
		SCRIPT_REG_ENUM(to, SpdyAllowedAttribute);
		SCRIPT_REG_ENUM(to, SpdyWasUsedAttribute);
		SCRIPT_REG_ENUM(to, EmitAllUploadProgressSignalsAttribute);
		SCRIPT_REG_ENUM(to, FollowRedirectsAttribute);
		SCRIPT_REG_ENUM(to, User);
		SCRIPT_REG_ENUM(to, UserMax);
		SCRIPT_REG_ENUM(to, HighPriority);
		SCRIPT_REG_ENUM(to, NormalPriority);
		SCRIPT_REG_ENUM(to, LowPriority);
		SCRIPT_REG_ENUM(to, AlwaysNetwork);
		SCRIPT_REG_ENUM(to, PreferNetwork);
		SCRIPT_REG_ENUM(to, PreferCache);
		SCRIPT_REG_ENUM(to, AlwaysCache);
		SCRIPT_REG_ENUM(to, Automatic);
		SCRIPT_REG_ENUM(to, Manual);
	}
};
}

using namespace Script;

void ScriptQNetworkRequest::Register(QScriptEngine *engine)
{
	qScriptRegisterMetaType(engine, ToScriptValue, FromScriptValue);

	auto proto = NewQObjectPrototype<ScriptQNetworkRequest>(engine);
	auto qNetworkRequest = engine->newFunction(Constructor, proto);
	qNetworkRequest.setData(proto);

	NetworkRequest::RegisterEnums(engine, qNetworkRequest);

	engine->globalObject().setProperty(
		className(), qNetworkRequest, STATIC_SCRIPT_VALUE);
}

ScriptQNetworkRequest::ScriptQNetworkRequest(QObject *parent)
	: QObject(parent)
{
}

QUrl ScriptQNetworkRequest::url() const
{
	auto req = thisRequest();
	if (req)
		return req->url();

	return QUrl();
}

void ScriptQNetworkRequest::setUrl(const QUrl &url)
{
	auto req = thisRequest();
	if (req)
		req->setUrl(url);
}

QNetworkRequest::Priority ScriptQNetworkRequest::priority() const
{
	auto req = thisRequest();
	if (req)
		return req->priority();

	return QNetworkRequest::Priority(0);
}

void ScriptQNetworkRequest::setPriority(QNetworkRequest::Priority priority)
{
	auto req = thisRequest();
	if (req)
		req->setPriority(priority);
}

int ScriptQNetworkRequest::maximumRedirectsAllowed() const
{
	auto req = thisRequest();
	if (req)
		return req->maximumRedirectsAllowed();

	return -1;
}

void ScriptQNetworkRequest::setMaximumRedirectsAllowed(int redirectCount)
{
	auto req = thisRequest();
	if (req)
		req->setMaximumRedirectsAllowed(redirectCount);
}

QNetworkRequest *ScriptQNetworkRequest::fromScriptValue(
	const QScriptValue &value)
{
	return qscriptvalue_cast<QNetworkRequest *>(value.data());
}

QVariant ScriptQNetworkRequest::header(
	QNetworkRequest::KnownHeaders header) const
{
	auto req = thisRequest();
	if (req)
		return req->header(header);

	return QVariant();
}

void ScriptQNetworkRequest::setHeader(
	QNetworkRequest::KnownHeaders header, const QVariant &value)
{
	auto req = thisRequest();
	if (req)
		req->setHeader(header, value);
}

QScriptValue ScriptQNetworkRequest::rawHeaderList() const
{
	auto req = thisRequest();
	if (!req)
		return QScriptValue();

	auto list = req->rawHeaderList();

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

bool ScriptQNetworkRequest::hasRawHeader(const QByteArray &headerName) const
{
	auto req = thisRequest();
	if (req)
		return req->hasRawHeader(headerName);

	return false;
}

QByteArray ScriptQNetworkRequest::rawHeader(const QByteArray &headerName) const
{
	auto req = thisRequest();
	if (req)
		return req->rawHeader(headerName);

	return QByteArray();
}

void ScriptQNetworkRequest::setRawHeader(
	const QByteArray &headerName, const QByteArray &value)
{
	auto req = thisRequest();
	if (req)
		req->setRawHeader(headerName, value);
}

QVariant ScriptQNetworkRequest::attribute(
	QNetworkRequest::Attribute code, const QVariant &defaultValue) const
{
	auto req = thisRequest();
	if (req)
		return req->attribute(code, defaultValue);

	return QVariant();
}

void ScriptQNetworkRequest::setAttribute(
	QNetworkRequest::Attribute code, const QVariant &value)
{
	auto req = thisRequest();
	if (req)
		req->setAttribute(code, value);
}

bool ScriptQNetworkRequest::equals(const QNetworkRequest &other) const
{
	auto req = thisRequest();
	if (req)
		return *req == other;

	return false;
}

QString ScriptQNetworkRequest::toString() const
{
	auto req = thisRequest();
	if (req)
	{
		return req->url().toString();
	}

	return className();
}

QString ScriptQNetworkRequest::className()
{
	return QSTRKEY(QNetworkRequest);
}

QNetworkRequest *ScriptQNetworkRequest::thisRequest() const
{
	return qscriptvalue_cast<QNetworkRequest *>(thisObject().data());
}

QScriptValue ScriptQNetworkRequest::Constructor(
	QScriptContext *context, QScriptEngine *engine)
{
	QNetworkRequest request;
	switch (context->argumentCount())
	{
		case 1:
			FromScriptValue(context->argument(0), request);
			break;

		case 0:
			break;

		default:
			return Script::ThrowBadNumberOfArguments(context);
	}

	return ToScriptValue(engine, request);
}

QScriptValue ScriptQNetworkRequest::ToScriptValue(
	QScriptEngine *engine, const QNetworkRequest &in)
{
	auto ctor = engine->globalObject().property(className());

	auto result = engine->newObject();
	result.setData(engine->newVariant(QVariant::fromValue(in)));
	result.setPrototype(ctor.data());
	return result;
}

void ScriptQNetworkRequest::FromScriptValue(
	const QScriptValue &object, QNetworkRequest &out)
{
	auto req = qscriptvalue_cast<QNetworkRequest *>(object.data());
	out = req ? *req : QNetworkRequest(object.toString());
}
}
