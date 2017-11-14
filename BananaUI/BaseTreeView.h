/*******************************************************************************
Banana Qt Libraries

Copyright (c) 2016-2017 Alexandra Cherdantseva

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

#include <QTreeView>

namespace Banana
{
class AbstractObjectTreeModel;
class UndoStack;

class BaseTreeView : public QTreeView
{
	Q_OBJECT

public:
	explicit BaseTreeView(
		AbstractObjectTreeModel *model, QWidget *parent = nullptr);

	void select(QObject *item, bool expand = false);
	void select(const QObjectSet &items);
	void expandItem(QObject *item);

	QObject *getCurrentItem() const;
	inline const QObjectSet &getSelectedItems() const;
	inline const QObjectSet &getExpandedItems() const;

	bool hasItems() const;

	void cutToClipboard();
	void copyToClipboard();
	void pasteFromClipboard();
	void deleteSelectedItems();

	bool canDeleteItem(QObject *item) const;
	bool canDeleteSelectedItems() const;

	void preventReselect(bool prevent);

protected slots:
	virtual void onBeforeModelReset();
	virtual void onAfterModelReset();

private slots:
	void onDropSuccess();
	void onShouldSelect(const QItemSelection &selection);
	void onExpanded(const QModelIndex &index);
	void onExpandedItemDestroyed(QObject *item);
	void onSelectedItemDestroyed(QObject *item);
	void onCollapsed(const QModelIndex &index);
	void onSelectionChanged(
		const QItemSelection &selected, const QItemSelection &deselected);
	void onUndoStackMacroStarted();
	void onUndoStackDestroyed();

private:
	void connectUndoStack();
	void disconnectUndoStack();

protected:
	AbstractObjectTreeModel *treeModel;

	UndoStack *undoStack;
	unsigned preventReselectCounter;

	QObjectSet oldSelected;
	QObjectSet expandedItems;
	QObjectSet selectedItems;
};

const QObjectSet &BaseTreeView::getSelectedItems() const
{
	return selectedItems;
}

const QObjectSet &BaseTreeView::getExpandedItems() const
{
	return expandedItems;
}
}
