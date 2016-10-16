#include "AbstractObjectGroup.h"

#include "IChildFilter.h"

namespace Core
{
	QObjectList AbstractObjectGroup::filterChildren(const IChildFilter *filter, bool sort)
	{
		auto children = getChildren();

		if (nullptr == filter)
			return children;

		QObjectList result;

		for (auto child : children)
		{
			bool ok = false;
			if (filter->filterMatch(child))
			{
				result.push_back(child);
				ok = true;
			}

			AbstractObjectGroup *group = nullptr;

			if (filter->shouldFilterDeeper(child))
			{
				group = dynamic_cast<AbstractObjectGroup *>(child);
				if (nullptr != group)
				{
					if (group->filterChildren(filter, false).empty())
						continue;
				}
			}

			if (!ok && nullptr != group)
				result.push_back(child);
		}

		if (sort)
			sortChildren(result);

		return result;
	}

	int AbstractObjectGroup::getChildIndex(const QObject *object)
	{
		if (nullptr != object)
		{
			auto &children = this->getChildren();

			auto it = std::find(children.begin(), children.end(), object);

			if (children.end() != it)
				return it - children.begin();
		}
		return -1;
	}

	QObject *AbstractObjectGroup::getChildAt(int index)
	{
		auto &children = getChildren();
		if (index >= 0 && index < children.count())
		{
			return children.at(index);
		}

		return nullptr;
	}

	void AbstractObjectGroup::removeAllGroupChildren()
	{
		auto children = getChildren();

		for (auto child : children)
		{
			deleteChild(child);
		}
	}

	void AbstractObjectGroup::deleteChild(QObject *child)
	{
		delete child;
	}

	const QObjectList &AbstractObjectGroup::getChildren()
	{
		auto object = dynamic_cast<QObject *>(this);
		Q_ASSERT(nullptr != object);
		return object->children();
	}

	void AbstractObjectGroup::sortChildren(QObjectList &children)
	{
		std::sort(children.begin(), children.end(),
		[](QObject *a, QObject *b)->bool
		{
			return QString::compare(a->objectName(), b->objectName(), Qt::CaseInsensitive) < 0;
		});
	}


}
