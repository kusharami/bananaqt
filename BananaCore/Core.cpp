#include "Core.h"

#include <map>

namespace Core
{

	QVariant ConvertToUserVariant(int user_type, const QVariant &from)
	{
		auto &conversion = getConversion();
		for (auto &it : conversion)
		{
			QVariant result;
			if (it.second.to_user_type(user_type, from, result))
				return result;
		}

		return from;
	}

	QVariant ConvertFromUserVariant(const QVariant &from)
	{
		if (QVariant::UserType == from.type())
		{
			auto &conversion = getConversion();
			for (auto &it : conversion)
			{
				QVariant result;
				if (it.second.to_base_type(from, result))
					return result;
			}
		}

		return from;
	}

	UserTypeConversionMap &getConversion()
	{
		static UserTypeConversionMap result;
		return result;
	}

}

