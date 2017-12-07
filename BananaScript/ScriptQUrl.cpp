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

#include "ScriptQUrl.h"

#include "ScriptQByteArray.h"
#include "ScriptUtils.h"

#include <QDir>

namespace Banana
{
namespace Script
{
struct Url : public QUrl
{
	static void RegisterEnums(QScriptEngine *engine, QScriptValue &to)
	{
		static bool convertersRegistered = false;
		if (not convertersRegistered)
		{
			convertersRegistered = true;
			QMetaType::registerConverter<ParsingMode, qint32>();
			QMetaType::registerConverter<FormattingOptions, qint32>();
			QMetaType::registerConverter<ComponentFormattingOptions, qint32>();
		}

		qScriptRegisterMetaType(engine, EnumToScriptValue<ParsingMode>,
			EnumFromScriptValue<ParsingMode>);

		qScriptRegisterMetaType(engine, EnumToScriptValue<FormattingOptions>,
			EnumFromScriptValue<FormattingOptions>);

		qScriptRegisterMetaType(engine,
			EnumToScriptValue<ComponentFormattingOptions>,
			EnumFromScriptValue<ComponentFormattingOptions>);

		SCRIPT_REG_ENUM(to, TolerantMode);
		SCRIPT_REG_ENUM(to, StrictMode);
		SCRIPT_REG_ENUM(to, DecodedMode);

		SCRIPT_REG_ENUM(to, None);
		SCRIPT_REG_ENUM(to, RemoveScheme);
		SCRIPT_REG_ENUM(to, RemovePassword);
		SCRIPT_REG_ENUM(to, RemoveUserInfo);
		SCRIPT_REG_ENUM(to, RemovePort);
		SCRIPT_REG_ENUM(to, RemoveAuthority);
		SCRIPT_REG_ENUM(to, RemovePath);
		SCRIPT_REG_ENUM(to, RemoveQuery);
		SCRIPT_REG_ENUM(to, RemoveFragment);
		SCRIPT_REG_ENUM(to, PreferLocalFile);
		SCRIPT_REG_ENUM(to, StripTrailingSlash);
		SCRIPT_REG_ENUM(to, RemoveFilename);
		SCRIPT_REG_ENUM(to, NormalizePathSegments);

		SCRIPT_REG_ENUM(to, PrettyDecoded);
		SCRIPT_REG_ENUM(to, EncodeSpaces);
		SCRIPT_REG_ENUM(to, EncodeUnicode);
		SCRIPT_REG_ENUM(to, EncodeDelimiters);
		SCRIPT_REG_ENUM(to, EncodeReserved);
		SCRIPT_REG_ENUM(to, DecodeReserved);
		SCRIPT_REG_ENUM(to, FullyEncoded);
		SCRIPT_REG_ENUM(to, FullyDecoded);

		SCRIPT_REG_ENUM(to, DefaultResolution);
		SCRIPT_REG_ENUM(to, AssumeLocalFile);
	}
};
}

using namespace Script;

void ScriptQUrl::Register(QScriptEngine *engine)
{
	qScriptRegisterMetaType(
		engine, ScriptQUrl::ToScriptValue, ScriptQUrl::FromScriptValue);

	auto proto = NewQObjectPrototype<ScriptQUrl>(engine);

	auto qUrl = engine->newFunction(Constructor, proto);
	qUrl.setData(proto);

	Url::RegisterEnums(engine, qUrl);

	qUrl.setProperty(QSTRKEY(fromEncoded), engine->newFunction(fromEncoded),
		STATIC_SCRIPT_VALUE);
	qUrl.setProperty(QSTRKEY(fromUserInput), engine->newFunction(fromUserInput),
		STATIC_SCRIPT_VALUE);
	qUrl.setProperty(QSTRKEY(fromLocalFile), engine->newFunction(fromLocalFile),
		STATIC_SCRIPT_VALUE);

	engine->globalObject().setProperty(className(), qUrl, STATIC_SCRIPT_VALUE);
}

ScriptQUrl::ScriptQUrl(QObject *parent)
	: QObject(parent)
{
}

void ScriptQUrl::setPort(int port)
{
	auto url = thisUrl();
	if (url)
		url->setPort(port);
}

QString ScriptQUrl::errorString() const
{
	auto url = thisUrl();
	if (url)
		return url->errorString();

	return QString();
}

bool ScriptQUrl::isEmpty() const
{
	auto url = thisUrl();
	if (url)
		return url->isEmpty();

	return true;
}

bool ScriptQUrl::isValid() const
{
	auto url = thisUrl();
	if (url)
		return url->isValid();

	return false;
}

bool ScriptQUrl::isRelative() const
{
	auto url = thisUrl();
	if (url)
		return url->isRelative();

	return false;
}

bool ScriptQUrl::isLocalFile() const
{
	auto url = thisUrl();
	if (url)
		return url->isLocalFile();

	return false;
}

bool ScriptQUrl::hasQuery() const
{
	auto url = thisUrl();
	if (url)
		return url->hasQuery();

	return false;
}

bool ScriptQUrl::hasFragment() const
{
	auto url = thisUrl();
	if (url)
		return url->hasFragment();

	return false;
}

void ScriptQUrl::setScheme(const QString &scheme)
{
	auto url = thisUrl();
	if (url)
		url->setScheme(scheme);
}

QString ScriptQUrl::scheme() const
{
	auto url = thisUrl();
	if (url)
		return url->scheme();

	return QString();
}

QString ScriptQUrl::filePath() const
{
	auto url = thisUrl();
	if (url)
		return url->toLocalFile();

	return QString();
}

void ScriptQUrl::setFilePath(const QString &filePath)
{
	auto url = thisUrl();
	if (url)
	{
		if (QDir::isAbsolutePath(filePath))
		{
			auto fileUrl = QUrl::fromLocalFile(filePath);
			url->setScheme(fileUrl.scheme());
			url->setPath(fileUrl.path());
		} else
		{
			url->setUrl(filePath);
		}
	}
}

QUrl *ScriptQUrl::fromScriptValue(const QScriptValue &value)
{
	return qscriptvalue_cast<QUrl *>(value.data());
}

void ScriptQUrl::setUrl(const QString &url, QUrl::ParsingMode mode)
{
	auto u = thisUrl();
	if (u)
		u->setUrl(url, mode);
}

void ScriptQUrl::setAuthority(const QString &authority, QUrl::ParsingMode mode)
{
	auto url = thisUrl();
	if (url)
		url->setAuthority(authority, mode);
}

QString ScriptQUrl::getAuthority(QUrl::ComponentFormattingOptions options) const
{
	auto url = thisUrl();
	if (url)
		return url->authority(options);

	return QString();
}

void ScriptQUrl::setUserInfo(const QString &userInfo, QUrl::ParsingMode mode)
{
	auto url = thisUrl();
	if (url)
		url->setUserInfo(userInfo, mode);
}

QString ScriptQUrl::getUserInfo(QUrl::ComponentFormattingOptions options) const
{
	auto url = thisUrl();
	if (url)
		return url->userInfo(options);

	return QString();
}

void ScriptQUrl::setUserName(const QString &userName, QUrl::ParsingMode mode)
{
	auto url = thisUrl();
	if (url)
		url->setUserName(userName, mode);
}

QString ScriptQUrl::getUserName(QUrl::ComponentFormattingOptions options) const
{
	auto url = thisUrl();
	if (url)
		return url->userName(options);

	return QString();
}

void ScriptQUrl::setPassword(const QString &password, QUrl::ParsingMode mode)
{
	auto url = thisUrl();
	if (url)
		url->setPassword(password, mode);
}

QString ScriptQUrl::getPassword(QUrl::ComponentFormattingOptions options) const
{
	auto url = thisUrl();
	if (url)
		return url->password(options);

	return QString();
}

void ScriptQUrl::setHost(const QString &host, QUrl::ParsingMode mode)
{
	auto url = thisUrl();
	if (url)
		url->setHost(host, mode);
}

QString ScriptQUrl::getHost(QUrl::ComponentFormattingOptions options) const
{
	auto url = thisUrl();
	if (url)
		return url->host(options);

	return QString();
}

QString ScriptQUrl::getTopLevelDomain(
	QUrl::ComponentFormattingOptions options) const
{
	auto url = thisUrl();
	if (url)
		return url->topLevelDomain(options);

	return QString();
}

int ScriptQUrl::getPort(int defaultPort) const
{
	auto url = thisUrl();
	if (url)
		return url->port(defaultPort);

	return -1;
}

void ScriptQUrl::setPath(const QString &path, QUrl::ParsingMode mode)
{
	auto url = thisUrl();
	if (url)
		url->setPath(path, mode);
}

QString ScriptQUrl::getPath(QUrl::ComponentFormattingOptions options) const
{
	auto url = thisUrl();
	if (url)
		return url->path(options);

	return QString();
}

QString ScriptQUrl::getFileName(QUrl::ComponentFormattingOptions options) const
{
	auto url = thisUrl();
	if (url)
		return url->fileName(options);

	return QString();
}

void ScriptQUrl::setQuery(const QString &query, QUrl::ParsingMode mode)
{
	auto url = thisUrl();
	if (url)
		url->setQuery(query, mode);
}

QString ScriptQUrl::getQuery(QUrl::ComponentFormattingOptions options) const
{
	auto url = thisUrl();
	if (url)
		return url->query(options);

	return QString();
}

void ScriptQUrl::setFragment(const QString &fragment, QUrl::ParsingMode mode)
{
	auto url = thisUrl();
	if (url)
		url->setFragment(fragment, mode);
}

QString ScriptQUrl::getFragment(QUrl::ComponentFormattingOptions options) const
{
	auto url = thisUrl();
	if (url)
		return url->fragment(options);

	return QString();
}

void ScriptQUrl::clear()
{
	auto url = thisUrl();
	if (url)
		url->clear();
}

bool ScriptQUrl::isParentOf(const QUrl &url) const
{
	auto u = thisUrl();
	if (u)
		return u->isParentOf(url);

	return false;
}

QUrl ScriptQUrl::resolved(const QUrl &relative) const
{
	auto url = thisUrl();
	if (url)
		return url->resolved(relative);

	return QUrl();
}

QUrl ScriptQUrl::adjusted(QUrl::FormattingOptions options) const
{
	auto url = thisUrl();
	if (url)
		return url->adjusted(options);

	return QUrl();
}

QString ScriptQUrl::toString() const
{
	auto url = thisUrl();
	if (url)
		return url->toString();

	return className();
}

QString ScriptQUrl::valueOf() const
{
	auto url = thisUrl();
	if (url)
		return url->url();

	return QString();
}

QString ScriptQUrl::toString(QUrl::FormattingOptions options) const
{
	auto url = thisUrl();
	if (url)
		return url->toString(options);

	return QString();
}

QString ScriptQUrl::toDisplayString(QUrl::FormattingOptions options) const
{
	auto url = thisUrl();
	if (url)
		return url->toDisplayString(options);

	return QString();
}

QString ScriptQUrl::toEncoded(QUrl::FormattingOptions options) const
{
	auto url = thisUrl();
	if (url)
		return QString::fromLatin1(url->toEncoded(options));

	return QString();
}

bool ScriptQUrl::equals(const QUrl &other) const
{
	auto url = thisUrl();
	if (url)
		return *url == other;

	return false;
}

QString ScriptQUrl::className()
{
	return QSTRKEY(QUrl);
}

QUrl *ScriptQUrl::thisUrl() const
{
	return fromScriptValue(thisObject());
}

QScriptValue ScriptQUrl::fromEncoded(
	QScriptContext *context, QScriptEngine *engine)
{
	int argc = context->argumentCount();

	switch (argc)
	{
		case 1:
		case 2:
			break;

		default:
			return ThrowBadNumberOfArguments(context);
	}

	QByteArray encoded;
	ScriptQByteArray::fromScriptValue(context->argument(0), encoded);

	auto parsingMode = (argc == 2)
		? QUrl::ParsingMode(context->argument(1).toInt32())
		: QUrl::TolerantMode;

	return ToScriptValue(engine, QUrl::fromEncoded(encoded, parsingMode));
}

QScriptValue ScriptQUrl::fromUserInput(
	QScriptContext *context, QScriptEngine *engine)
{
	int argc = context->argumentCount();

	switch (argc)
	{
		case 1:
		{
			return ToScriptValue(
				engine, QUrl::fromUserInput(context->argument(0).toString()));
		}

		case 2:
		case 3:
		{
			auto resolution = (argc == 3) ? QUrl::UserInputResolutionOptions(
												context->argument(2).toInt32())
										  : QUrl::DefaultResolution;

			return ToScriptValue(engine,
				QUrl::fromUserInput(context->argument(0).toString(),
					context->argument(1).toString(), resolution));
		}

		default:
			break;
	}

	return ThrowBadNumberOfArguments(context);
}

QScriptValue ScriptQUrl::fromLocalFile(
	QScriptContext *context, QScriptEngine *engine)
{
	if (context->argumentCount() != 1)
		return ThrowBadNumberOfArguments(context);

	return ToScriptValue(
		engine, QUrl::fromLocalFile(context->argument(0).toString()));
}

QScriptValue ScriptQUrl::Constructor(
	QScriptContext *context, QScriptEngine *engine)
{
	QUrl url;
	switch (context->argumentCount())
	{
		case 1:
			FromScriptValue(context->argument(0), url);
			break;

		case 2:
			url = QUrl(context->argument(0).toString(),
				QUrl::ParsingMode(context->argument(1).toInt32()));
			break;

		case 0:
			break;

		default:
			return ThrowBadNumberOfArguments(context);
	}

	return ToScriptValue(engine, url);
}

QScriptValue ScriptQUrl::ToScriptValue(QScriptEngine *engine, const QUrl &in)
{
	auto ctor = engine->globalObject().property(className());

	auto result = engine->newObject();
	result.setData(engine->newVariant(QVariant::fromValue(in)));
	result.setPrototype(ctor.data());
	return result;
}

void ScriptQUrl::FromScriptValue(const QScriptValue &object, QUrl &out)
{
	auto url = qscriptvalue_cast<QUrl *>(object.data());
	out = url ? *url : QUrl(object.toString());
}
}
