#include "PropertyDef.h"

#include <QMetaProperty>

namespace Core
{

	quint64 findPropertyFlags(const QMetaProperty &metaProperty, const PropertyFlagsMap &in)
	{
		auto it = in.find(metaProperty.propertyIndex());
		if (it != in.end())
			return it->second;

		return 0;
	}

}
