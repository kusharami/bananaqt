#include "UniqueFileNameScope.h"

#include "AbstractFileSystemObject.h"

namespace Banana
{

	UniqueFileNameScope::UniqueFileNameScope(const QMetaObject *meta_object,
											 QObject *parent)
		: UniqueNameScope(meta_object, Qt::CaseInsensitive, parent)
	{

	}

	bool UniqueFileNameScope::checkSiblingNameForObject(QObject *sibling,
														const QString &name,
														QObject *object) const
	{
		auto object_fsys = dynamic_cast<AbstractFileSystemObject *>(object);
		auto sibling_fsys = dynamic_cast<AbstractFileSystemObject *>(sibling);

		if (nullptr != object_fsys && nullptr != sibling_fsys)
		{
			QString check_name(name + object_fsys->getFileExtension());
			return (0 == check_name.compare(sibling_fsys->getFileName(), Qt::CaseInsensitive));
		}

		return UniqueNameScope::checkSiblingNameForObject(sibling, name, object);
	}

}
