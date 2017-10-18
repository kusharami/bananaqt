/*******************************************************************************
Banana Qt Libraries

Copyright (c) 2016 Alexandra Cherdantseva

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

#include "Core.h"

#include "AbstractNameUnifier.h"

#include <QString>

namespace Banana
{
class NameEnumerator : public AbstractNameUnifier
{
	Q_GADGET

public:
	NameEnumerator();
	NameEnumerator(const NameCollectionPtr &ptr);

	enum Type
	{
		StartsWithNumber,
		EndsWithNumber
	};

	Q_ENUM(Type)

	inline Type type() const;
	inline void setType(Type value);

	inline QString separator() const;
	inline void setSeparator(const QString &value);

	void separateNameAndNumber(const QString &fullName,
		QString *resultNamePtr = nullptr,
		QString *resultNumberPtr = nullptr) const;

	virtual QString uniqueNameFor(const QString &name) const override;
	virtual bool isValid() const override;

private:
	QString attachNumber(const QString &name, quint64 number) const;

	Type mType;
	QString mSeparator;
};

NameEnumerator::Type NameEnumerator::type() const
{
	return mType;
}

void NameEnumerator::setType(Type value)
{
	mType = value;
}

QString NameEnumerator::separator() const
{
	return mSeparator;
}

void NameEnumerator::setSeparator(const QString &value)
{
	mSeparator = value;
}
}

Q_DECLARE_METATYPE(Banana::NameEnumerator::Type)
