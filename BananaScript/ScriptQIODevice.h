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

#include <QIODevice>
#include <QScriptable>

Q_DECLARE_METATYPE(QIODevice *)
Q_DECLARE_METATYPE(QIODevice::OpenMode)

namespace Banana
{
class ScriptQIODevice
	: public QObject
	, public QScriptable
{
	Q_OBJECT

	Q_PROPERTY(qint64 bytesAvailable READ bytesAvailable)
	Q_PROPERTY(qint64 bytesToWrite READ bytesToWrite)
	Q_PROPERTY(QString errorString READ errorString)

	Q_PROPERTY(
		bool isTextModeEnabled READ isTextModeEnabled WRITE setTextModeEnabled)
	Q_PROPERTY(bool isOpen READ isOpen)
	Q_PROPERTY(bool sequential READ isSequential)
	Q_PROPERTY(bool readable READ isReadable)
	Q_PROPERTY(bool writable READ isWritable)
	Q_PROPERTY(bool atEnd READ atEnd)
	Q_PROPERTY(bool isTransactionStarted READ isTransactionStarted)

	Q_PROPERTY(qint64 pos READ pos WRITE setPos)
	Q_PROPERTY(QIODevice::OpenMode openMode READ openMode)

public:
	explicit ScriptQIODevice(QObject *parent);
	static QScriptValue Register(QScriptEngine *engine);

	QString errorString() const;

	bool isTextModeEnabled() const;
	void setTextModeEnabled(bool value);

	bool isSequential() const;
	bool isOpen() const;
	bool isReadable() const;
	bool isWritable() const;
	bool atEnd() const;
	bool isTransactionStarted() const;
	qint64 bytesAvailable() const;
	qint64 bytesToWrite() const;

	qint64 size() const;

	qint64 pos() const;
	void setPos(qint64 pos);

	QIODevice::OpenMode openMode() const;

public slots:
	bool open(QIODevice::OpenMode mode);
	virtual void close();
	bool seek(qint64 offset);
	bool reset();

	void startTransaction();
	void commitTransaction();
	void rollbackTransaction();

	QByteArray peek(int maxlen);
	QByteArray read(int maxlen);
	QByteArray readAll();
	QByteArray readLine(qint64 maxLen = 0);
	bool canReadLine() const;

	int write(const QByteArray &data);

	virtual QString toString() const;

private:
	QIODevice *thisDevice() const;
};
}
