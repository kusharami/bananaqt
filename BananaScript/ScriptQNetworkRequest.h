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
#include <QNetworkRequest>
#include <QScriptValue>
#include <QScriptable>

Q_DECLARE_METATYPE(QNetworkRequest *)
Q_DECLARE_METATYPE(QNetworkRequest::KnownHeaders)
Q_DECLARE_METATYPE(QNetworkRequest::Attribute)
Q_DECLARE_METATYPE(QNetworkRequest::Priority)

namespace Banana
{
class ScriptQNetworkRequest final
	: public QObject
	, public QScriptable
{
	Q_OBJECT

	Q_PROPERTY(QUrl url READ url WRITE setUrl)
	Q_PROPERTY(
		QNetworkRequest::Priority priority READ priority WRITE setPriority)
	Q_PROPERTY(int maximumRedirectsAllowed READ maximumRedirectsAllowed WRITE
			setMaximumRedirectsAllowed)

public:
	static void Register(QScriptEngine *engine);

	explicit ScriptQNetworkRequest(QObject *parent);

	QUrl url() const;
	void setUrl(const QUrl &url);

	QNetworkRequest::Priority priority() const;
	void setPriority(QNetworkRequest::Priority priority);

	int maximumRedirectsAllowed() const;
	void setMaximumRedirectsAllowed(int redirectCount);

	static QNetworkRequest *fromScriptValue(const QScriptValue &value);

public slots:
	QVariant header(QNetworkRequest::KnownHeaders header) const;
	void setHeader(QNetworkRequest::KnownHeaders header, const QVariant &value);

	QScriptValue rawHeaderList() const;
	bool hasRawHeader(const QByteArray &headerName) const;
	QByteArray rawHeader(const QByteArray &headerName) const;
	void setRawHeader(const QByteArray &headerName, const QByteArray &value);

	QVariant attribute(QNetworkRequest::Attribute code,
		const QVariant &defaultValue = QVariant()) const;
	void setAttribute(QNetworkRequest::Attribute code, const QVariant &value);

	bool equals(const QNetworkRequest &other) const;

	QString toString() const;

private:
	static QString className();
	QNetworkRequest *thisRequest() const;

	static QScriptValue Constructor(
		QScriptContext *context, QScriptEngine *engine);
	static QScriptValue ToScriptValue(
		QScriptEngine *engine, const QNetworkRequest &in);
	static void FromScriptValue(
		const QScriptValue &object, QNetworkRequest &out);
};
}
