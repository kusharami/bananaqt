/*******************************************************************************
Banana Qt Libraries

Copyright (c) 2017 Alexandra Cherdantseva

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

#include "SelectionCacheManager.h"

#include "ISelectionDelegate.h"

#include "BananaCore/Object.h"

namespace Banana
{
SelectionCacheManager::SelectionCacheManager(QObject *parent)
	: QObject(parent)
	, mDelegate(nullptr)
{
}

SelectionCacheManager::~SelectionCacheManager()
{
	clear();
}

void SelectionCacheManager::setSelectionDelegate(ISelectionDelegate *delegate)
{
	if (delegate != mDelegate)
	{
		clear();
		mDelegate = delegate;
	}
}

void SelectionCacheManager::retainSelection(Object *keyObject)
{
	auto &items = getSelectedItems();
	auto it = mCache.find(keyObject);
	if (it != mCache.end())
	{
		if (items == it->second)
			return;

		disconnectItems(it->second);
		if (items.empty())
		{
			disconnectKey(keyObject);
			mCache.erase(it);
			return;
		}
	} else
	{
		if (items.empty())
			return;

		connectKey(keyObject);
	}

	mCache[keyObject] = items;
	connectItems(items);
}

void SelectionCacheManager::restoreSelection(Object *keyObject)
{
	auto it = mCache.find(keyObject);

	setSelectedItems((it != mCache.end()) ? it->second : QObjectSet());
}

const QObjectSet &SelectionCacheManager::getSelectedItems() const
{
	Q_ASSERT(nullptr != mDelegate);
	return mDelegate->getSelectedItems();
}

void SelectionCacheManager::setSelectedItems(const QObjectSet &items)
{
	Q_ASSERT(nullptr != mDelegate);
	mDelegate->setSelectedItems(items);
}

void SelectionCacheManager::clear()
{
	for (auto &it : mCache)
	{
		disconnectKey(it.first);
		disconnectItems(it.second);
	}

	mCache.clear();
	Q_ASSERT(mConnectedItems.empty());
}

void SelectionCacheManager::onKeyReloadStarted()
{
	Q_ASSERT(nullptr != dynamic_cast<Object *>(sender()));
	auto object = static_cast<Object *>(sender());

	disconnectKey(object);
	onKeyDestroyed(object);
}

void SelectionCacheManager::onKeyDestroyed(Object *keyObject)
{
	auto it = mCache.find(keyObject);
	Q_ASSERT(it != mCache.end());

	disconnectItems(it->second);

	mCache.erase(it);
}

void SelectionCacheManager::onItemDestroyed(QObject *item)
{
	for (auto &it : mCache)
	{
		it.second.erase(item);
	}

	mConnectedItems.erase(item);
}

void SelectionCacheManager::connectKey(Object *keyObject)
{
	Q_ASSERT(nullptr != keyObject);

	QObject::connect(keyObject, &Object::reloadStarted, this,
		&SelectionCacheManager::onKeyReloadStarted);
	QObject::connect(keyObject, &Object::beforeDestroy, this,
		&SelectionCacheManager::onKeyDestroyed);
}

void SelectionCacheManager::disconnectKey(Object *keyObject)
{
	Q_ASSERT(nullptr != keyObject);

	QObject::disconnect(keyObject, &Object::reloadStarted, this,
		&SelectionCacheManager::onKeyReloadStarted);
	QObject::disconnect(keyObject, &Object::beforeDestroy, this,
		&SelectionCacheManager::onKeyDestroyed);
}

void SelectionCacheManager::connectItems(const QObjectSet &items)
{
	for (auto item : items)
	{
		if (mConnectedItems.count(item) == 0)
		{
			QObject::connect(item, &QObject::destroyed, this,
				&SelectionCacheManager::onItemDestroyed);
		}

		mConnectedItems.insert(item);
	}
}

void SelectionCacheManager::disconnectItems(const QObjectSet &items)
{
	for (auto item : items)
	{
		auto it = mConnectedItems.find(item);
		Q_ASSERT(it != mConnectedItems.end());

		mConnectedItems.erase(it);

		if (mConnectedItems.count(item) == 0)
		{
			QObject::disconnect(item, &QObject::destroyed, this,
				&SelectionCacheManager::onItemDestroyed);
		}
	}
}
}
