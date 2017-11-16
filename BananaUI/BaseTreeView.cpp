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

#include "BaseTreeView.h"

#include "AbstractObjectTreeModel.h"
#include "SelectTreeItemsCommand.h"

#include "BananaUI/UndoStack.h"

#include "BananaCore/AbstractObjectGroup.h"

#include <QFocusEvent>

namespace Banana
{
BaseTreeView::BaseTreeView(AbstractObjectTreeModel *model, QWidget *parent)
	: QTreeView(parent)
	, treeModel(model)
	, undoStack(nullptr)
	, preventReselectCounter(0)
{
	model->setParent(this);

	QObject::connect(
		this, &QTreeView::expanded, this, &BaseTreeView::onExpanded);
	QObject::connect(
		this, &QTreeView::collapsed, this, &BaseTreeView::onCollapsed);
	QObject::connect(treeModel, &AbstractObjectTreeModel::beforeModelReset,
		this, &BaseTreeView::onBeforeModelReset);
	QObject::connect(treeModel, &AbstractObjectTreeModel::afterModelReset, this,
		&BaseTreeView::onAfterModelReset);
	QObject::connect(treeModel, &AbstractObjectTreeModel::shouldSelect, this,
		&BaseTreeView::onShouldSelect);
	QObject::connect(treeModel, &AbstractObjectTreeModel::shouldClearSelection,
		this, &BaseTreeView::clearSelection);
	QObject::connect(treeModel, &AbstractObjectTreeModel::dropSuccess, this,
		&BaseTreeView::onDropSuccess);

	setModel(treeModel);

	QObject::connect(selectionModel(), &QItemSelectionModel::selectionChanged,
		this, &BaseTreeView::onSelectionChanged);
}

void BaseTreeView::select(QObject *item, bool expand)
{
	auto index = treeModel->findModelIndex(item);
	if (index.isValid())
	{
		auto selModel = selectionModel();
		clearSelection();
		setCurrentIndex(index);
		selModel->select(index, QItemSelectionModel::Select);
		if (expand)
			this->expand(index);

		scrollTo(index);
	}
}

void BaseTreeView::select(const QObjectSet &items)
{
	auto selModel = selectionModel();
	clearSelection();
	bool first = true;
	for (auto item : items)
	{
		auto index = treeModel->findModelIndex(item);
		if (index.isValid())
		{
			if (first)
			{
				first = false;
				setCurrentIndex(index);
			}

			selModel->select(index, QItemSelectionModel::Select);
		}
	}
}

void BaseTreeView::expandItem(QObject *item)
{
	auto index = treeModel->findModelIndex(item);
	if (index.isValid())
		expand(index);
}

QObject *BaseTreeView::getCurrentItem() const
{
	auto index = currentIndex();
	if (index.isValid())
	{
		return treeModel->getItemAt(index);
	}

	return nullptr;
}

bool BaseTreeView::hasItems() const
{
	return (nullptr != treeModel && treeModel->rowCount() > 0);
}

void BaseTreeView::cutToClipboard()
{
	treeModel->copyToClipboard(selectionModel()->selectedIndexes(), true);
}

void BaseTreeView::copyToClipboard()
{
	treeModel->copyToClipboard(selectionModel()->selectedIndexes(), false);
}

void BaseTreeView::pasteFromClipboard()
{
	treeModel->pasteFromClipboard(selectionModel()->selectedIndexes());
}

void BaseTreeView::deleteSelectedItems()
{
	treeModel->deleteItems(selectionModel()->selectedIndexes());
}

bool BaseTreeView::canDeleteItem(QObject *item) const
{
	return treeModel->canDeleteItem(item);
}

bool BaseTreeView::canDeleteSelectedItems() const
{
	return treeModel->canDeleteItems(selectionModel()->selectedIndexes());
}

void BaseTreeView::preventReselect(bool prevent)
{
	if (prevent)
		preventReselectCounter++;
	else
		preventReselectCounter--;
}

void BaseTreeView::onBeforeModelReset()
{
	auto undoStack = treeModel->getUndoStack();
	if (nullptr != undoStack && undoStack->canPushForMacro())
	{
		auto reselectCommand = new SelectTreeItemsCommand(this);
		reselectCommand->setOldSelected(selectedItems);
		undoStack->pushCommand(reselectCommand);
	}
}

void BaseTreeView::onAfterModelReset()
{
	preventReselectCounter++;

	QObjectSet items;
	items.swap(expandedItems);

	for (auto &item : items)
	{
		auto index = treeModel->findModelIndex(item);
		if (index.isValid())
		{
			expand(index);
			expandedItems.insert(item);
		}
	}

	items.clear();
	items.swap(selectedItems);

	QItemSelection selection;

	QModelIndex firstIndex;

	for (auto &item : items)
	{
		auto index = treeModel->findModelIndex(item);
		if (index.isValid())
		{
			if (!firstIndex.isValid())
				firstIndex = index;
			selection.select(index, index);
			selectedItems.insert(item);
		}
	}

	if (firstIndex.isValid())
		setCurrentIndex(firstIndex);
	selectionModel()->select(selection, QItemSelectionModel::Select);

	auto undoStack = treeModel->getUndoStack();
	if (nullptr != undoStack && undoStack->canPushForMacro())
	{
		auto reselectCommand = new SelectTreeItemsCommand(this);
		reselectCommand->setNewSelected(selectedItems);
		undoStack->pushCommand(reselectCommand);
	}

	preventReselectCounter--;
}

void BaseTreeView::onDropSuccess()
{
	setFocus();
}

void BaseTreeView::onShouldSelect(const QItemSelection &selection)
{
	if (!selection.isEmpty())
	{
		auto indexes = selection.indexes();

		setCurrentIndex(indexes.at(0));

		for (auto &index : indexes)
		{
			if (index.parent().isValid())
				expand(index.parent());
		}

		selectionModel()->select(selection, QItemSelectionModel::Select);
	}
}

void BaseTreeView::onExpanded(const QModelIndex &index)
{
	auto item = treeModel->getItemAt(index);
	expandedItems.insert(item);
	QObject::connect(item, &QObject::destroyed, this,
		&BaseTreeView::onExpandedItemDestroyed);
}

void BaseTreeView::onExpandedItemDestroyed(QObject *item)
{
	expandedItems.erase(item);
}

void BaseTreeView::onSelectedItemDestroyed(QObject *item)
{
	selectedItems.erase(item);
}

void BaseTreeView::onCollapsed(const QModelIndex &index)
{
	auto item = treeModel->getItemAt(index);
	expandedItems.erase(item);
	QObject::disconnect(item, &QObject::destroyed, this,
		&BaseTreeView::onExpandedItemDestroyed);
}

void BaseTreeView::onSelectionChanged(
	const QItemSelection &selected, const QItemSelection &deselected)
{
	Q_ASSERT(nullptr != treeModel);
	auto undoStack = treeModel->getUndoStack();
	bool canPushCommand =
		(nullptr != undoStack && undoStack->canPushForMacro());
	QObjectSet oldSelected;
	if (canPushCommand)
		oldSelected = selectedItems;

	preventReselectCounter++;

	auto indexes = selected.indexes();

	for (auto it = indexes.begin(); it != indexes.end(); ++it)
	{
		auto item = treeModel->getItemAt(*it);
		selectedItems.insert(item);
		QObject::connect(item, &QObject::destroyed, this,
			&BaseTreeView::onSelectedItemDestroyed);
	}

	indexes = deselected.indexes();

	for (auto it = indexes.begin(); it != indexes.end(); ++it)
	{
		auto item = treeModel->getItemAt(*it);
		selectedItems.erase(item);
		QObject::disconnect(item, &QObject::destroyed, this,
			&BaseTreeView::onSelectedItemDestroyed);
	}

	if (canPushCommand)
	{
		undoStack->pushCommand(
			new SelectTreeItemsCommand(this, oldSelected, selectedItems));
	} else if (nullptr != undoStack)
	{
		if (undoStack != this->undoStack)
		{
			std::swap(this->oldSelected, oldSelected);
			disconnectUndoStack();
			this->undoStack = undoStack;
			connectUndoStack();
		}
	}

	preventReselectCounter--;
}

void BaseTreeView::onUndoStackMacroStarted()
{
	if (undoStack == treeModel->getUndoStack())
	{
		Q_ASSERT(nullptr != undoStack);

		auto reselectCommand = new SelectTreeItemsCommand(this,
			oldSelected.empty() ? selectedItems : oldSelected, selectedItems);
		undoStack->pushCommand(reselectCommand);
		disconnectUndoStack();
		undoStack = nullptr;
	}
}

void BaseTreeView::onUndoStackDestroyed()
{
	undoStack = nullptr;
}

void BaseTreeView::connectUndoStack()
{
	if (nullptr != undoStack)
	{
		auto qundoStack = undoStack->qundoStack();
		QObject::connect(qundoStack, &QObject::destroyed, this,
			&BaseTreeView::onUndoStackDestroyed);
		QObject::connect(qundoStack, &UndoStack::macroStarted, this,
			&BaseTreeView::onUndoStackMacroStarted);
	}
}

void BaseTreeView::disconnectUndoStack()
{
	if (nullptr != undoStack)
	{
		auto qundoStack = undoStack->qundoStack();
		QObject::disconnect(qundoStack, &QObject::destroyed, this,
			&BaseTreeView::onUndoStackDestroyed);
		QObject::disconnect(qundoStack, &UndoStack::macroStarted, this,
			&BaseTreeView::onUndoStackMacroStarted);
	}
}
}
