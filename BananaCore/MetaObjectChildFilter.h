#pragma once

#include "IChildFilter.h"

struct QMetaObject;

namespace Core
{

	class MetaObjectChildFilter : public IChildFilter
	{
	public:
		MetaObjectChildFilter(const QMetaObject *meta_object = nullptr);

		virtual bool filterMatch(QObject *object) const override;
		virtual bool shouldFilterDeeper(QObject *object) const override;

		const QMetaObject *getMetaObject() const;
		void setMetaObject(const QMetaObject *value);

	private:
		const QMetaObject *meta_object;
	};

}

