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

#pragma once

#include "BananaCore/ContainerTypes.h"

#include <QObject>

#include <map>

namespace Banana
{
class Object;
struct ISelectionDelegate;
class SelectionCacheManager : public QObject
{
	ISelectionDelegate *mDelegate;
	std::map<Object *, QObjectSet> mCache;
	std::multiset<QObject *> mConnectedItems;

public:
	explicit SelectionCacheManager(QObject *parent = nullptr);
	virtual ~SelectionCacheManager() override;

	inline ISelectionDelegate *selectionDelegate() const;
	void setSelectionDelegate(ISelectionDelegate *delegate);

	void retainSelection(Object *keyObject);
	void restoreSelection(Object *keyObject);

	QObjectSet getSelectedItems() const;
	void setSelectedItems(const QObjectSet &items);

	void clear();

private:
	void onKeyReloadStarted();
	void onKeyDestroyed(Object *keyObject);
	void onItemDestroyed(QObject *item);

	void connectKey(Object *keyObject);
	void disconnectKey(Object *keyObject);
	void connectItems(const QObjectSet &items);
	void disconnectItems(const QObjectSet &items);
};

ISelectionDelegate *SelectionCacheManager::selectionDelegate() const
{
	return mDelegate;
}
}
