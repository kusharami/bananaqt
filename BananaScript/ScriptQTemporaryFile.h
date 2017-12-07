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

#include "ScriptQFile.h"

#include <QTemporaryFile>

Q_DECLARE_METATYPE(QTemporaryFile *)

namespace Banana
{
class ScriptQTemporaryFile final : public ScriptQFile
{
	Q_OBJECT

	Q_PROPERTY(bool autoRemove READ autoRemove WRITE setAutoRemove)
	Q_PROPERTY(QString fileTemplate READ fileTemplate WRITE setFileTemplate)

public:
	explicit ScriptQTemporaryFile(QObject *parent);
	static void Register(QScriptEngine *engine);

	bool autoRemove() const;
	void setAutoRemove(bool b);

	QString fileTemplate() const;
	void setFileTemplate(const QString &name);

	virtual QString toString() const override;
	virtual void close() override;

public slots:
	bool open();

private:
	static QScriptValue Construct(
		QScriptContext *context, QScriptEngine *engine);

	QTemporaryFile *thisFile() const;
};
}
