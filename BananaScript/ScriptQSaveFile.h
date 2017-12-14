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

#include <QSaveFile>

Q_DECLARE_METATYPE(QSaveFile *)

namespace Banana
{
class ScriptQSaveFile final : public ScriptQFileDevice
{
	Q_OBJECT

	Q_PROPERTY(QString filePath READ filePath WRITE setFilePath)
	Q_PROPERTY(bool directWriteFallback READ directWriteFallback WRITE
			setDirectWriteFallback)

public:
	explicit ScriptQSaveFile(QObject *parent);
	static void Register(QScriptEngine *engine);

	void setFilePath(const QString &value);

	void setDirectWriteFallback(bool enabled);
	bool directWriteFallback() const;

	virtual QString toString() const override;
	virtual void close() override;

public slots:
	bool commit();
	void cancelWriting();

private:
	static QScriptValue Construct(
		QScriptContext *context, QScriptEngine *engine);

	QSaveFile *thisFile() const;
};
}
