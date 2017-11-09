/*******************************************************************************
Banana Qt Libraries

Copyright (c) 2016-2017 Alexandra Cherdantseva

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

#include "FileRegistrator.h"
#include "AbstractFile.h"

#include <QByteArray>

namespace Banana
{
class BinaryData
	: public Object
	, public QByteArray
{
	Q_OBJECT
};

class BinaryFile : public Banana::BaseFileRegistrator<Banana::AbstractFile>
{
	Q_OBJECT

public:
	Q_INVOKABLE explicit BinaryFile();

	virtual bool isWritableFormat(const QString &extension) const override;

protected:
	virtual QObject *doCreateFileData() const override;
	virtual bool doSave(QIODevice *device) override;
	virtual bool doLoad(QIODevice *device) override;
};
}

Q_DECLARE_METATYPE(Banana::BinaryFile *)
