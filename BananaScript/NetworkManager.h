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

#include "ScriptQUrl.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QScriptValue>
#include <QScriptContext>
#include <QScriptable>

Q_DECLARE_METATYPE(QNetworkAccessManager *)
Q_DECLARE_METATYPE(QNetworkAccessManager::Operation)
Q_DECLARE_METATYPE(QNetworkAccessManager::NetworkAccessibility)

Q_DECLARE_METATYPE(QNetworkRequest::KnownHeaders)
Q_DECLARE_METATYPE(QNetworkRequest::Attribute)
Q_DECLARE_METATYPE(QNetworkRequest::Priority)

Q_DECLARE_METATYPE(QNetworkReply *)

namespace Banana
{
namespace Script
{
class ScriptQNetworkRequest
	: public QObject
	, public QNetworkRequest
{
	Q_OBJECT
	Q_PROPERTY(QUrl url READ url WRITE setUrl)
	Q_PROPERTY(
		QNetworkRequest::Priority priority READ priority WRITE setPriority)
	Q_PROPERTY(int maximumRedirectsAllowed READ maximumRedirectsAllowed WRITE
			setMaximumRedirectsAllowed)

public:
	static void Register(QScriptEngine *engine);

	ScriptQNetworkRequest(
		const QNetworkRequest &request, QScriptEngine *engine);

public slots:
	inline QVariant header(QNetworkRequest::KnownHeaders header) const;
	inline void setHeader(
		QNetworkRequest::KnownHeaders header, const QVariant &value);

	QScriptValue rawHeaderList() const;
	bool hasRawHeader(const QScriptValue &headerName) const;
	QByteArray rawHeader(const QScriptValue &headerName) const;
	void setRawHeader(
		const QScriptValue &headerName, const QScriptValue &value);

	inline QVariant attribute(QNetworkRequest::Attribute code,
		const QVariant &defaultValue = QVariant()) const;
	inline void setAttribute(
		QNetworkRequest::Attribute code, const QVariant &value);

	QString toString() const;

private:
	static QScriptValue Constructor(
		QScriptContext *context, QScriptEngine *engine);
	static QScriptValue ToScriptValue(
		QScriptEngine *engine, const QNetworkRequest &in);
	static void FromScriptValue(
		const QScriptValue &object, QNetworkRequest &out);

	QScriptEngine *engine;
};

class ScriptQNetworkReply
	: public QObject
	, public QScriptable
{
	Q_OBJECT
	Q_PROPERTY(
		qint64 readBufferSize READ readBufferSize WRITE setReadBufferSize)
	Q_PROPERTY(qint64 bytesAvailable READ bytesAvailable)
	Q_PROPERTY(QNetworkAccessManager *manager READ manager)
	Q_PROPERTY(QNetworkAccessManager::Operation operation READ operation)
	Q_PROPERTY(QNetworkRequest request READ request)
	Q_PROPERTY(QNetworkReply::NetworkError resultCode READ error)
	Q_PROPERTY(QString errorString READ errorString)
	Q_PROPERTY(bool isFinished READ isFinished) // DO NOT RENAME !
	Q_PROPERTY(bool running READ isRunning)
	Q_PROPERTY(bool isOpen READ isOpen)
	Q_PROPERTY(bool isTransactionStarted READ isTransactionStarted)
	Q_PROPERTY(QUrl url READ url)

public:
	static void Register(QScriptEngine *engine);

	ScriptQNetworkReply(QObject *parent);

	qint64 readBufferSize() const;
	void setReadBufferSize(qint64 size);

	qint64 bytesAvailable() const;

	QNetworkAccessManager *manager() const;
	QNetworkAccessManager::Operation operation() const;
	QNetworkRequest request() const;
	QNetworkReply::NetworkError error() const;
	QString errorString() const;
	bool isFinished() const;
	bool isRunning() const;
	bool isOpen() const;
	QUrl url() const;
	bool isTransactionStarted() const;

public slots:
	void close();
	QVariant attribute(QNetworkRequest::Attribute code) const;

	QVariant header(QNetworkRequest::KnownHeaders header) const;

	bool hasRawHeader(const QScriptValue &headerName) const;
	QScriptValue rawHeaderList() const;
	QByteArray rawHeader(const QScriptValue &headerName) const;

	void startTransaction();
	void commitTransaction();
	void rollbackTransaction();

	QByteArray peek(int maxlen);
	QByteArray read(int maxlen);
	QByteArray readAll();
	QByteArray readLine(int maxlen = 0);

	QString toString() const;

private:
	QNetworkReply *thisReply() const;
};

class ScriptQNetworkAccessManager
	: public QObject
	, public QScriptable
{
	Q_OBJECT

public:
	static void Register(QScriptEngine *engine);

	ScriptQNetworkAccessManager(QObject *parent);

public slots:
	QScriptValue head(const QScriptValue &request);
	QScriptValue get(const QScriptValue &request);
	QScriptValue post(const QScriptValue &request, const QScriptValue &data);
	QScriptValue put(const QScriptValue &request, const QScriptValue &data);
	QScriptValue deleteResource(const QScriptValue &request);
	void connectToHostEncrypted(const QString &hostName, quint16 port = 443);
	void connectToHost(const QString &hostName, quint16 port = 80);
	QString toString() const;

private:
	QScriptValue wrappedReply(QNetworkReply *reply);

	static QScriptValue Construct(
		QScriptContext *context, QScriptEngine *engine);
	QNetworkAccessManager *thisManager();
};

QVariant ScriptQNetworkRequest::header(KnownHeaders header) const
{
	return QNetworkRequest::header(header);
}

void ScriptQNetworkRequest::setHeader(
	KnownHeaders header, const QVariant &value)
{
	QNetworkRequest::setHeader(header, value);
}

QVariant ScriptQNetworkRequest::attribute(
	Attribute code, const QVariant &defaultValue) const
{
	return QNetworkRequest::attribute(code, defaultValue);
}

void ScriptQNetworkRequest::setAttribute(Attribute code, const QVariant &value)
{
	QNetworkRequest::setAttribute(code, value);
}
}
}
