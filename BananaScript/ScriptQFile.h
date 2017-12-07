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

#include "ScriptQFileDevice.h"

#include <QFile>

Q_DECLARE_METATYPE(QFile *)

namespace Banana
{
class ScriptQFile : public ScriptQFileDevice
{
	Q_OBJECT

	Q_PROPERTY(QString filePath READ filePath WRITE setFilePath)
	Q_PROPERTY(QString symLinkTarget READ symLinkTarget)

public:
	explicit ScriptQFile(QObject *parent);
	static QScriptValue Register(QScriptEngine *engine);

	void setFilePath(const QString &value);

	QString symLinkTarget() const;

	virtual QString toString() const override;

public slots:
	bool exists();
	bool remove();
	bool rename(const QString &newFilePath);
	bool copy(const QString &newFilePath);
	bool link(const QString &linkPath);

private:
	static QString className();

	static QScriptValue exists(QScriptContext *context, QScriptEngine *engine);
	static QScriptValue remove(QScriptContext *context, QScriptEngine *engine);
	static QScriptValue rename(QScriptContext *context, QScriptEngine *engine);
	static QScriptValue copy(QScriptContext *context, QScriptEngine *engine);

	static QScriptValue Construct(
		QScriptContext *context, QScriptEngine *engine);

	QFile *thisFile() const;
};
}
