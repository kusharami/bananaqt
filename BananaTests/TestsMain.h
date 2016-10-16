#pragma once

#include <QString>
#include <QtTest>
#include <QMetaObject>

#include <functional>

#if defined(_UNICODE) && defined(_MSC_VER)
 #define _T(c) QString::fromWCharArray(L##c)
#else
 #define _T(c) QString::fromUtf8(c)
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
