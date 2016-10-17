/*
 * MIT License
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

#include "AbstractObjectGroup.h"

#include "IChildFilter.h"

namespace Banana
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
