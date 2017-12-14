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

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QScriptValue>
#include <QScriptable>

Q_DECLARE_METATYPE(QNetworkAccessManager *)
Q_DECLARE_METATYPE(QNetworkAccessManager::Operation)
Q_DECLARE_METATYPE(QNetworkAccessManager::NetworkAccessibility)

namespace Banana
{
class ScriptQNetworkAccessManager
	: public QObject
	, public QScriptable
{
	Q_OBJECT

	Q_PROPERTY(QNetworkAccessManager::NetworkAccessibility networkAccessible
			READ networkAccessible WRITE setNetworkAccessible)

public:
	static void Register(QScriptEngine *engine);

	explicit ScriptQNetworkAccessManager(QObject *parent);

	void setNetworkAccessible(
		QNetworkAccessManager::NetworkAccessibility accessible);
	QNetworkAccessManager::NetworkAccessibility networkAccessible() const;

public slots:
	QScriptValue head(const QNetworkRequest &request);
	QScriptValue get(const QNetworkRequest &request);
	QScriptValue post(const QNetworkRequest &request, const QScriptValue &data);
	QScriptValue put(const QNetworkRequest &request, const QScriptValue &data);
	QScriptValue deleteResource(const QNetworkRequest &request);
	void connectToHostEncrypted(const QString &hostName, quint16 port = 443);
	void connectToHost(const QString &hostName, quint16 port = 80);
	inline QString toString() const;

private:
	static QString className();
	QNetworkAccessManager *thisManager() const;

	QScriptValue wrappedReply(QNetworkReply *reply);

	static QScriptValue Construct(
		QScriptContext *context, QScriptEngine *engine);
};

QString ScriptQNetworkAccessManager::toString() const
{
	return className();
}
}
