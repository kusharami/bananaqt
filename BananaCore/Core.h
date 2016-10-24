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

#pragma once

#include <QVariant>

#include <map>
#include <set>

#ifdef QT_DEBUG
#	define IS_VALID(obj) Q_ASSERT(nullptr != (obj) && (obj)->isValid())
#	define REQUIRE(expression) Q_ASSERT(expression)
#	define ENSURE(expression) Q_ASSERT(expression)
#else
#	define IS_VALID(obj) qt_noop()
#	define REQUIRE(expression) qt_noop()
#	define ENSURE(expression) qt_noop()
#endif

class QObject;

namespace Banana
{

	class BaseObject
	{
	public:
		virtual ~BaseObject() {}

		virtual bool isValid() const = 0;
	};

	template <typename T, typename... ARG_T>
	static inline T *createObject(QObject *parent, ARG_T... args)
	{
		auto result = new T(args...);
		result->setParent(parent);
		return result;
	}

	QVariant ConvertToUserVariant(int user_type, const QVariant &from);
	QVariant ConvertFromUserVariant(const QVariant &from);

	enum class ConnectionState
	{
		Connected,
		Disconnected,
		Destroyed
	};

	struct UserTypeConversion
	{
		bool (*to_base_type)(const QVariant &from, QVariant &to);
		bool (*to_user_type)(int user_type, const QVariant &from, QVariant &to);
	};

	typedef std::map<int, UserTypeConversion> UserTypeConversionMap;

	UserTypeConversionMap &getConversion();

	template <typename FROM_T, typename TO_T>
	bool convert_to_user_variant(int user_type, const QVariant &from, QVariant &to)
	{
		if (user_type == qMetaTypeId<TO_T>())
		{
			to = QVariant::fromValue<TO_T>(TO_T(from.value<FROM_T>()));
			return true;
		}

		return false;
	}

	template <typename FROM_T, typename TO_T>
	bool convert_from_user_variant(const QVariant &from, QVariant &to)
	{
		if (from.userType() == qMetaTypeId<FROM_T>())
		{
			to = QVariant(TO_T(from.value<FROM_T>()));
			return true;
		}

		return false;
	}

	template<typename From, typename To>
	static inline To convertImplicit(const From& from)
	{
		return To(from);
	}

	template <typename TYPE, typename BASE_TYPE>
	void RegisterType()
	{
		qRegisterMetaType<TYPE>();
		QMetaType::registerConverter<BASE_TYPE, TYPE>(convertImplicit<BASE_TYPE, TYPE>);

		getConversion()[qMetaTypeId<TYPE>()] =
		{
			&convert_from_user_variant<TYPE, BASE_TYPE>,
			&convert_to_user_variant<BASE_TYPE, TYPE>
		};
	}
}

