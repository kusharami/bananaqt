#pragma once

#include <QObjectList>

namespace Core
{
	struct IChildFilter;
	class AbstractObjectGroup
	{
	public:
		virtual ~AbstractObjectGroup() {}

		virtual const QObjectList &getChildren();

		QObjectList filterChildren(const IChildFilter *filter, bool sort = true);
		int getChildIndex(const QObject *object);
		QObject *getChildAt(int index);

		void removeAllGroupChildren();
		virtual void resetChildren() = 0;

	protected:
		virtual void sortChildren(QObjectList &children);
		virtual void deleteChild(QObject *child);
	};

}
