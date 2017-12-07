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

#include "ScriptQIODevice.h"

#include <QFileDevice>

Q_DECLARE_METATYPE(QFileDevice *)
Q_DECLARE_METATYPE(QFileDevice::Permissions)
Q_DECLARE_METATYPE(QFileDevice::FileError)

namespace Banana
{
class ScriptQFileDevice : public ScriptQIODevice
{
	Q_OBJECT

	Q_PROPERTY(QFileDevice::FileError error READ error)
	Q_PROPERTY(QFileDevice::Permissions permissions READ permissions WRITE
			setPermissions)

	Q_PROPERTY(qint64 size READ size WRITE setSize)

public:
	explicit ScriptQFileDevice(QObject *parent);

	static QScriptValue Register(QScriptEngine *engine);

	QFileDevice::FileError error() const;

	QFileDevice::Permissions permissions() const;
	void setPermissions(QFileDevice::Permissions value);

	void setSize(qint64 size);

	QString filePath() const;

	virtual QString toString() const override;

public slots:
	bool resize(qint64 newSize);
	bool flush();
	void unsetError();

private:
	QFileDevice *thisDevice() const;
};
}
