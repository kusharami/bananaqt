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

#include "tst_AbstractNameUnifier.h"

#include "TestsMain.h"

#include "BananaCore/AbstractNameUnifier.h"

REGISTER_TEST(AbstractNameUnifier);

namespace InternalANU
{

	class Unifier : public Banana::AbstractNameUnifier
	{
	public:
		virtual QString uniqueNameFor(const QString &) const override { return QString(); }
	};

	class Collection : public Banana::INameCollection
	{
	public:
		virtual bool containsName(const QString &) const override { return false; }
};

}

AbstractNameUnifier::AbstractNameUnifier()
	: unifier(nullptr)
{
}

void AbstractNameUnifier::init()
{
	unifier = new InternalANU::Unifier;
	collection.reset(new InternalANU::Collection);
}

void AbstractNameUnifier::cleanup()
{
	delete unifier;
	collection.reset();
}

void AbstractNameUnifier::testNameCollectionProperty()
{
	unifier->setNameCollection(collection);
	QVERIFY(unifier->nameCollection() == collection);
}
