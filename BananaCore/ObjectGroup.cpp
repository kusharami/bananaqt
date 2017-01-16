/*******************************************************************************
Banana Qt Libraries

Copyright (c) 2016 Alexandra Cherdantseva

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*******************************************************************************/

#include "ObjectGroup.h"

#include "Core.h"
#include "IChildFilter.h"

#include <QChildEvent>

namespace Banana
{

	ObjectGroup::ObjectGroup()
	{

	}

	void ObjectGroup::registerChildType(const QMetaObject *meta_object, Qt::CaseSensitivity sensitivity)
	{
		Q_ASSERT(nullptr != meta_object);

		auto it = child_types.find(meta_object);
		if (child_types.end() == it)
		{
			auto scope = createNameScope(meta_object, sensitivity);
			child_types[meta_object] = NameScopePtr(scope);

			reconnectChildren(scope);
		} else
		{
			it->second->setCaseSensitivity(sensitivity);
		}
	}

	void ObjectGroup::unregisterChildType(const QMetaObject *meta_object, bool children)
	{
		auto it = child_types.find(meta_object);

		if (child_types.end() != it)
		{
			childrenNeedRearrange();

			for (auto child : this->children())
			{
				if (children)
				{
					auto group = dynamic_cast<ObjectGroup *>(child);
					if (nullptr != group)
						group->unregisterChildType(meta_object, children);
				}

				if (child->metaObject() == meta_object)
					disconnectChildObject(child, it->second.get());
			}

			child_types.erase(meta_object);
		}
	}

	void ObjectGroup::unregisterAllChildTypes(bool children)
	{
		for (auto child : this->children())
		{
			if (children)
			{
				auto group = dynamic_cast<ObjectGroup *>(child);
				if (nullptr != group)
					group->unregisterAllChildTypes(children);
			}

			auto it = child_types.find(child->metaObject());

			if (child_types.end() != it)
			{
				disconnectChildObject(child, it->second.get());
			}
		}

		child_types.clear();
	}

	bool ObjectGroup::isSupportedChildType(const QMetaObject *meta_object) const
	{
		return child_types.end() != child_types.find(meta_object);
	}

	const QObjectList &ObjectGroup::getChildren()
	{
		makeChildList();

		return m_children;
	}

	void ObjectGroup::resetChildren()
	{
		childrenNeedRearrange();
	}

	UniqueNameScope *ObjectGroup::createNameScope(const QMetaObject *meta_object,
												  Qt::CaseSensitivity sensitivity) const
	{
		return new UniqueNameScope(meta_object, sensitivity);
	}

	void ObjectGroup::doAddChild(QObject *object)
	{
		auto it = child_types.find(object->metaObject());
		if (child_types.end() != it)
		{
			connectChildObject(object, it->second.get());
		}

		Object::doAddChild(object);
	}

	void ObjectGroup::doRemoveChild(QObject *object)
	{
		auto it = child_types.find(object->metaObject());
		if (child_types.end() != it)
		{
			disconnectChildObject(object, it->second.get());
		}

		Object::doRemoveChild(object);
	}

	void ObjectGroup::doConnectChildObject(QObject *object)
	{
		emit childObjectConnectionChanged(object, ConnectionState::Connected);
	}

	void ObjectGroup::doDisconnectChildObject(QObject *object)
	{
		emit childObjectConnectionChanged(object, ConnectionState::Disconnected);
	}

	void ObjectGroup::onChildObjectDestroyed(QObject *object)
	{
		childrenNeedRearrange();

		emit childObjectConnectionChanged(object, ConnectionState::Destroyed);
	}

	void ObjectGroup::childrenNeedRearrange()
	{
		m_children.clear();
	}

	void ObjectGroup::reconnectChildren(UniqueNameScope *new_scope)
	{
		childrenNeedRearrange();

		foreach (QObject *child, children())
		{
			auto object = new_scope->getObjectType()->cast(child);
			if (nullptr != object)
				connectChildObject(object, new_scope);
		}
	}

	void ObjectGroup::makeChildList()
	{
		if (m_children.empty())
		{
			foreach (QObject *child, children())
			{
				auto it = child_types.find(child->metaObject());
				if (child_types.end() != it)
				{
					auto object = dynamic_cast<Object *>(child);
					if (nullptr == object || !object->isDeleted())
						m_children.push_back(child);
				}
			}

			sortChildren(m_children);
		}
	}

	void ObjectGroup::connectChildObject(QObject *object, UniqueNameScope *scope)
	{
		childrenNeedRearrange();

		scope->connectObject(object);

		QObject::connect(object, &QObject::destroyed, this, &ObjectGroup::onChildObjectDestroyed);
		QObject::connect(object, &QObject::objectNameChanged, this, &ObjectGroup::childrenNeedRearrange);

		doConnectChildObject(object);
	}

	void ObjectGroup::disconnectChildObject(QObject *object, UniqueNameScope *scope)
	{
		childrenNeedRearrange();

		scope->disconnectObject(object);

		QObject::disconnect(object, &QObject::destroyed, this, &ObjectGroup::onChildObjectDestroyed);
		QObject::disconnect(object, &QObject::objectNameChanged, this, &ObjectGroup::childrenNeedRearrange);

		doDisconnectChildObject(object);
	}

}
