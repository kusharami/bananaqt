/*
 * Banana Qt Libraries
 *
 * Copyright (c) 2016 Alexandra Cherdantseva
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "NameEnumerator.h"

#include "Utils.h"

namespace Banana
{
	static const QString sNumericFormatTag("%u");
	static const QString sStringFormatTag("%s");
	static const int nFormatTagsLength = sNumericFormatTag.length() + sStringFormatTag.length();

	NameEnumerator::NameEnumerator()
		: mType(EndsWithNumber)
	{
	}

	NameEnumerator::NameEnumerator(const NameCollectionPtr &ptr)
		: mType(EndsWithNumber)
	{
		setNameCollection(ptr);
	}

	QString NameEnumerator::uniqueNameFor(const QString &name) const
	{
		QString result(name);
		QString withoutNumber;

		quint64 number = 0, originalNumber = 0;

		bool first = true;

		IS_VALID(mNameCollection.get());
		while (mNameCollection->containsName(result))
		{
			Q_ASSERT(first || number != originalNumber);

			if (first)
			{
				first = false;
				separateNameAndNumber(name, &withoutNumber, &result);
				originalNumber = result.toULongLong();
				number = originalNumber;
			}

			if (number == 0)
				result = withoutNumber;
			else
				result = attachNumber(withoutNumber, number);

			number++;
		}

		if (!first)
		{
			switch (mType)
			{
				case StartsWithNumber:
					ENSURE(result.endsWith(withoutNumber));
					break;

				case EndsWithNumber:
					ENSURE(result.startsWith(withoutNumber));
					break;
			}
		} else
			ENSURE(result == name);

		return result;
	}

	void NameEnumerator::separateNameAndNumber(const QString &fullName,
											   QString *resultNamePtr,
											   QString *resultNumberPtr) const
	{
		int numberLength = 0;

		QString resultName(fullName);
		QString resultNumber;

		switch (mType)
		{
			case StartsWithNumber:
			{
				for (int i = 0, length = fullName.length();
					 i < length && fullName.at(i).isDigit();
					 i++, numberLength++)
				{
				}

				if (numberLength > 0
				&&	fullName.indexOf(mSeparator) == numberLength)
				{
					resultName = QString(&fullName.data()
									 [numberLength + mSeparator.length()]);
					resultNumber = QString(fullName.data(), numberLength);
				}

			}	break;

			case EndsWithNumber:
			{
				for (int i = fullName.length() - 1;
					 i >= 0 && fullName.at(i).isDigit();
					 i--, numberLength++)
				{
				}

				int len = fullName.length() - numberLength - mSeparator.length();

				if (numberLength > 0 && len >= 0
				&&	0 == memcmp(mSeparator.data(), &fullName.data()[len],
								mSeparator.length() * sizeof(QChar)))
				{
					resultName = QString(fullName.data(), len);
					resultNumber = QString(&fullName.data()[fullName.length() - numberLength],
							numberLength);
				}

			}	break;
		}

		switch (mType)
		{
			case StartsWithNumber:
				ENSURE(fullName.endsWith(resultName));
				ENSURE(fullName.startsWith(resultNumber));
				break;

			case EndsWithNumber:
				ENSURE(fullName.startsWith(resultName));
				ENSURE(fullName.endsWith(resultNumber));
				break;
		}

		ENSURE(resultNumber.isEmpty()
			|| resultName.length() + resultNumber.length() == fullName.length()
			|| resultName.length() == fullName.length() - resultNumber.length() - mSeparator.length());

		ENSURE(resultNumber.isEmpty() || Utils::IsDigitString(resultNumber));

		if (nullptr != resultNamePtr)
			*resultNamePtr = resultName;

		if (nullptr != resultNumberPtr)
			*resultNumberPtr = resultNumber;
	}

	QString NameEnumerator::attachNumber(const QString &name, quint64 number) const
	{
		QString result;

		switch (mType)
		{
			case StartsWithNumber:
				result = QString::number(number) + mSeparator + name;
				break;

			case EndsWithNumber:
				result = name + mSeparator + QString::number(number);
				break;
		}

		return result;
	}

	bool NameEnumerator::isValid() const
	{
		if (AbstractNameUnifier::isValid())
		{
			switch (mType)
			{
				case StartsWithNumber:
				case EndsWithNumber:
					return true;
			}
		}

		return false;
	}

}
