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

#include "tst_NameEnumerator.h"

#include "TestsMain.h"

#include "BananaCore/NameEnumerator.h"

#include <QStringList>

REGISTER_TEST(NameEnumerator);

using NE = Banana::NameEnumerator;

namespace InternalNE
{

	class Collection : public QStringList, public Banana::INameCollection
	{
	public:
		virtual bool containsName(const QString &name) const override;
	};

	bool Collection::containsName(const QString &name) const
	{
		return indexOf(name) >= 0;
	}
}

NameEnumerator::NameEnumerator()
	: internalCollection(nullptr)
	, enumerator(nullptr)
{

}

void NameEnumerator::init()
{
	internalCollection = new InternalNE::Collection;
	collection.reset(internalCollection);
	enumerator = new NE(collection);
	IS_VALID(enumerator);
}

void NameEnumerator::testSeparateNameAndNumber_data()
{
	QADD_COLUMN(NE::Type, type);
	QADD_COLUMN(QString, separator);
	QADD_COLUMN(QString, name);
	QADD_COLUMN(QString, resultName);
	QADD_COLUMN(QString, resultNumber);

	QTest::newRow("1") << NE::EndsWithNumber << " " << "block 1" << "block" << "1";
	QTest::newRow("2") << NE::StartsWithNumber << " " << "block 5" << "block 5" << "";
	QTest::newRow("3") << NE::EndsWithNumber << "" << "256block" << "256block" << "";
	QTest::newRow("4") << NE::StartsWithNumber << ". " << "333. block" << "block" << "333";
}


void NameEnumerator::cleanup()
{
	delete enumerator;
	collection.reset();
}


void NameEnumerator::testSeparateNameAndNumber()
{
	QFETCH(NE::Type, type);
	QFETCH(QString, separator);
	QFETCH(QString, name);
	QFETCH(QString, resultName);
	QFETCH(QString, resultNumber);

	enumerator->setType(type);
	enumerator->setSeparator(separator);
	QString testResultName, testResultNumber;
	enumerator->separateNameAndNumber(name, &testResultName, &testResultNumber);

	QVERIFY(resultName == testResultName);
	QVERIFY(resultNumber == testResultNumber);
}

void NameEnumerator::testUniqueNameFor_data()
{
	QADD_COLUMN(NE::Type, type);
	QADD_COLUMN(QString, separator);
	QADD_COLUMN(QString, testName);
	QADD_COLUMN(QString, resultName);

	QTest::newRow("1") << NE::EndsWithNumber << " " << "buka" << "buka 1";
	QTest::newRow("2") << NE::StartsWithNumber << ". " << "buka" << "1. buka";
	QTest::newRow("3") << NE::EndsWithNumber << "" << "tornado" << "tornado";
	QTest::newRow("4") << NE::StartsWithNumber << ". " << "1. trash" << "2. trash";
	QTest::newRow("5") << NE::EndsWithNumber << "" << "tornado25" << "tornado26";
}

void NameEnumerator::testUniqueNameFor()
{
	QFETCH(NE::Type, type);
	QFETCH(QString, separator);
	QFETCH(QString, testName);
	QFETCH(QString, resultName);

	static const QStringList names =
	{
		"buka",
		"1. trash",
		"tornado25"
	};

	*dynamic_cast<QStringList *>(internalCollection) = names;
	enumerator->setType(type);
	enumerator->setSeparator(separator);
	QVERIFY(enumerator->uniqueNameFor(testName) == resultName);
}
