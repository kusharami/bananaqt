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

#include "ScriptQNetworkAccessManager.h"
#include "ScriptQIODevice.h"

Q_DECLARE_METATYPE(QNetworkReply *)

namespace Banana
{
class ScriptQNetworkReply : public ScriptQIODevice
{
	Q_OBJECT

	Q_PROPERTY(
		qint64 readBufferSize READ readBufferSize WRITE setReadBufferSize)
	Q_PROPERTY(QScriptValue manager READ manager)
	Q_PROPERTY(QNetworkAccessManager::Operation operation READ operation)
	Q_PROPERTY(QNetworkRequest request READ request)
	Q_PROPERTY(QNetworkReply::NetworkError resultCode READ resultCode)
	Q_PROPERTY(bool isFinished READ isFinished) // DO NOT RENAME !
	Q_PROPERTY(bool running READ isRunning)
	Q_PROPERTY(QUrl url READ url)

public:
	static void Register(QScriptEngine *engine);

	explicit ScriptQNetworkReply(QObject *parent);

	qint64 readBufferSize() const;
	void setReadBufferSize(qint64 size);

	QScriptValue manager() const;
	QNetworkAccessManager::Operation operation() const;
	QNetworkRequest request() const;
	QNetworkReply::NetworkError resultCode() const;
	bool isFinished() const;
	bool isRunning() const;
	QUrl url() const;

	virtual QString toString() const override;

public slots:
	QVariant attribute(QNetworkRequest::Attribute code) const;
	QVariant header(QNetworkRequest::KnownHeaders header) const;

	bool hasRawHeader(const QByteArray &headerName) const;
	QScriptValue rawHeaderList() const;
	QByteArray rawHeader(const QByteArray &headerName) const;

private:
	static QString className();
	QNetworkReply *thisReply() const;
};
}
