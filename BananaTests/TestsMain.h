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

#include <QString>
#include <QtTest>
#include <QMetaObject>

#include <functional>

#define UTF16(c) QString::fromWCharArray(L##c)
#define UTF8(c) QString::fromUtf8(c)

#if defined(_UNICODE) && defined(_MSC_VER)
 #define _T(c) UTF16(c)
#else
 #define _T(c) UTF8(c)
#endif

#define QADD_COLUMN(Type, Name) QTest::addColumn<Type>(#Name)

typedef std::function<QObject *()> TestCreator;
size_t registerTestCreator(const TestCreator &create);

template <typename T>
size_t registerTest()
{
	return registerTestCreator([]() -> QObject * { return new T; });
}

#define CAT2(a,b) a##b
#define CAT(a,b) CAT2(a,b)

#define REGISTER_TEST(Class) \
static auto CAT(test, __COUNTER__) = registerTest<Class>()
