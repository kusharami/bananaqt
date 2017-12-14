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

#pragma once

#include <QObject>
#include <QUrl>
#include <QScriptValue>
#include <QScriptable>

Q_DECLARE_METATYPE(QUrl *)
Q_DECLARE_METATYPE(QUrl::ParsingMode)
Q_DECLARE_METATYPE(QUrl::FormattingOptions)
Q_DECLARE_METATYPE(QUrl::ComponentFormattingOptions)

namespace Banana
{
class ScriptQUrl final
	: public QObject
	, public QScriptable
{
	Q_OBJECT

	Q_PROPERTY(bool valid READ isValid)
	Q_PROPERTY(bool empty READ isEmpty)
	Q_PROPERTY(bool relative READ isRelative)
	Q_PROPERTY(bool hasQuery READ hasQuery)
	Q_PROPERTY(bool hasFragment READ hasFragment)
	Q_PROPERTY(bool isLocalFile READ isLocalFile)

	Q_PROPERTY(QString url READ url WRITE setUrl)
	Q_PROPERTY(QString scheme READ scheme WRITE setScheme)
	Q_PROPERTY(QString authority READ authority WRITE setAuthority)
	Q_PROPERTY(QString userInfo READ userInfo WRITE setUserInfo)
	Q_PROPERTY(QString userName READ userName WRITE setUserName)
	Q_PROPERTY(QString password READ password WRITE setPassword)
	Q_PROPERTY(QString host READ host WRITE setHost)
	Q_PROPERTY(int port READ port WRITE setPort)
	Q_PROPERTY(QString path READ path WRITE setPath)
	Q_PROPERTY(QString query READ query WRITE setQuery)
	Q_PROPERTY(QString fragment READ fragment WRITE setFragment)
	Q_PROPERTY(QString topLevelDomain READ topLevelDomain)
	Q_PROPERTY(QString fileName READ fileName)
	Q_PROPERTY(QString filePath READ filePath WRITE setFilePath)
	Q_PROPERTY(QString errorString READ errorString)

public:
	static void Register(QScriptEngine *engine);

	explicit ScriptQUrl(QObject *parent);

	inline QString url() const;
	inline QString authority() const;
	inline QString userInfo() const;
	inline QString userName() const;
	inline QString password() const;
	inline QString host() const;
	inline int port() const;
	inline QString path() const;
	inline QString query() const;
	inline QString fragment() const;
	inline QString topLevelDomain() const;
	inline QString fileName() const;

	void setPort(int port);

	QString errorString() const;
	bool isEmpty() const;
	bool isValid() const;
	bool isRelative() const;
	bool isLocalFile() const;
	bool hasQuery() const;
	bool hasFragment() const;

	void setScheme(const QString &scheme);
	QString scheme() const;

	QString filePath() const;
	void setFilePath(const QString &filePath);

	static QUrl *fromScriptValue(const QScriptValue &value);

public slots:
	void setUrl(
		const QString &url, QUrl::ParsingMode mode = QUrl::TolerantMode);

	void setAuthority(
		const QString &authority, QUrl::ParsingMode mode = QUrl::TolerantMode);
	QString getAuthority(QUrl::ComponentFormattingOptions options) const;

	void setUserInfo(
		const QString &userInfo, QUrl::ParsingMode mode = QUrl::TolerantMode);
	QString getUserInfo(QUrl::ComponentFormattingOptions options) const;

	void setUserName(
		const QString &userName, QUrl::ParsingMode mode = QUrl::DecodedMode);
	QString getUserName(QUrl::ComponentFormattingOptions options) const;

	void setPassword(
		const QString &password, QUrl::ParsingMode mode = QUrl::DecodedMode);
	QString getPassword(QUrl::ComponentFormattingOptions options) const;

	void setHost(
		const QString &host, QUrl::ParsingMode mode = QUrl::DecodedMode);
	QString getHost(QUrl::ComponentFormattingOptions options) const;

	QString getTopLevelDomain(QUrl::ComponentFormattingOptions options) const;

	int getPort(int defaultPort) const;

	void setPath(
		const QString &path, QUrl::ParsingMode mode = QUrl::DecodedMode);
	QString getPath(QUrl::ComponentFormattingOptions options) const;
	QString getFileName(QUrl::ComponentFormattingOptions options) const;

	void setQuery(
		const QString &query, QUrl::ParsingMode mode = QUrl::TolerantMode);
	QString getQuery(QUrl::ComponentFormattingOptions options) const;

	void setFragment(
		const QString &fragment, QUrl::ParsingMode mode = QUrl::TolerantMode);
	QString getFragment(QUrl::ComponentFormattingOptions options) const;

	void clear();

	bool isParentOf(const QUrl &url) const;

	QUrl resolved(const QUrl &relative) const;
	QUrl adjusted(QUrl::FormattingOptions options) const;

	QString toString() const;
	QString valueOf() const;
	QString toString(QUrl::FormattingOptions options) const;
	QString toDisplayString(
		QUrl::FormattingOptions options = QUrl::PrettyDecoded) const;

	QString toEncoded(
		QUrl::FormattingOptions options = QUrl::FullyEncoded) const;

	bool equals(const QUrl &other) const;

private:
	static QString className();
	QUrl *thisUrl() const;

	static QScriptValue fromEncoded(
		QScriptContext *context, QScriptEngine *engine);
	static QScriptValue fromUserInput(
		QScriptContext *context, QScriptEngine *engine);
	static QScriptValue fromLocalFile(
		QScriptContext *context, QScriptEngine *engine);

	static QScriptValue Constructor(
		QScriptContext *context, QScriptEngine *engine);
	static QScriptValue ToScriptValue(QScriptEngine *engine, const QUrl &in);
	static void FromScriptValue(const QScriptValue &object, QUrl &out);
};

QString ScriptQUrl::url() const
{
	return toString(QUrl::PrettyDecoded);
}

QString ScriptQUrl::authority() const
{
	return getAuthority(QUrl::PrettyDecoded);
}

QString ScriptQUrl::userInfo() const
{
	return getUserInfo(QUrl::PrettyDecoded);
}

QString ScriptQUrl::userName() const
{
	return getUserName(QUrl::FullyDecoded);
}

QString ScriptQUrl::password() const
{
	return getPassword(QUrl::FullyDecoded);
}

QString ScriptQUrl::host() const
{
	return getHost(QUrl::FullyDecoded);
}

int ScriptQUrl::port() const
{
	return getPort(-1);
}

QString ScriptQUrl::path() const
{
	return getPath(QUrl::FullyDecoded);
}

QString ScriptQUrl::query() const
{
	return getQuery(QUrl::PrettyDecoded);
}

QString ScriptQUrl::fragment() const
{
	return getFragment(QUrl::PrettyDecoded);
}

QString ScriptQUrl::topLevelDomain() const
{
	return getTopLevelDomain(QUrl::FullyDecoded);
}

QString ScriptQUrl::fileName() const
{
	return getFileName(QUrl::FullyDecoded);
}
}
