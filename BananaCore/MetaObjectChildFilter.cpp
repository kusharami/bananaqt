#include "MetaObjectChildFilter.h"

#include <QObject>

namespace Core
{

	MetaObjectChildFilter::MetaObjectChildFilter(const QMetaObject *meta_object)
		: meta_object(meta_object)
	{

	}

	bool MetaObjectChildFilter::filterMatch(QObject *object) const
	{
		return object->metaObject() == meta_object;
	}

	bool MetaObjectChildFilter::shouldFilterDeeper(QObject *) const
	{
		return true;
	}

	const QMetaObject *MetaObjectChildFilter::getMetaObject() const
	{
		return meta_object;
	}

	void MetaObjectChildFilter::setMetaObject(const QMetaObject *value)
	{
		meta_object = value;
	}
}
