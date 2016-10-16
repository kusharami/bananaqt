#pragma once

#include <QVariant>

#include <map>
#include <set>

namespace Core
{
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

