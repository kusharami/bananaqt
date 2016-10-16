#pragma once

#include  "UniqueNameScope.h"

namespace Banana
{

	class UniqueFileNameScope : public UniqueNameScope
	{
		Q_OBJECT

	public:
		explicit UniqueFileNameScope(const QMetaObject *meta_object,
									 QObject *parent = nullptr);

	protected:
		virtual bool checkSiblingNameForObject(QObject *sibling, const QString &name, QObject *object) const override;
	};
}
