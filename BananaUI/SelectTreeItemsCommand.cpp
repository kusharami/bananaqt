/*
 * Banana Qt Libraries
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

#include "SelectTreeItemsCommand.h"

#include "BaseTreeView.h"
#include "Const.h"

#include "BananaCore/Object.h"


namespace Banana
{

	SelectTreeItemsCommand::SelectTreeItemsCommand(BaseTreeView *tree)
		: tree(tree)
		, skipRedoOnPush(true)
	{

	}

	SelectTreeItemsCommand::SelectTreeItemsCommand(BaseTreeView *tree,
												   const QObjectSet &oldSelected,
												   const QObjectSet &newSelected)
		: tree(tree)
		, skipRedoOnPush(true)
	{
		setOldSelected(oldSelected);
		setNewSelected(newSelected);
	}

	void SelectTreeItemsCommand::setOldSelected(const QObjectSet &set)
	{
		toPaths(set, oldSelected);
	}

	void SelectTreeItemsCommand::setNewSelected(const QObjectSet &set)
	{
		toPaths(set, newSelected);
	}

	void SelectTreeItemsCommand::undo()
	{
		select(oldSelected);
	}

	void SelectTreeItemsCommand::redo()
	{
		if (skipRedoOnPush)
		{
			skipRedoOnPush = false;
			return;
		}

		select(newSelected);
	}

	int SelectTreeItemsCommand::id() const
	{
		return SELECT_TREE_ITEMS_COMMAND;
	}

	bool SelectTreeItemsCommand::mergeWith(const QUndoCommand *other)
	{
		auto otherCommand = dynamic_cast<const SelectTreeItemsCommand *>(other);
		if (nullptr != otherCommand && tree == otherCommand->tree)
		{
			for (auto &path : otherCommand->newSelected)
			{
				if (findPath(path, newSelected) < 0)
					newSelected.push_back(path);
			}

			return true;
		}

		return false;
	}

	void SelectTreeItemsCommand::toPaths(const QObjectSet &source, Paths &output)
	{
		output.clear();
		for (auto object : source)
		{
			auto topAncestor = Object::getTopAncestor(object);
			if (nullptr != topAncestor)
			{
				output.push_back({ topAncestor, Object::getNamesChain(topAncestor, object) });
			}
		}
	}

	int SelectTreeItemsCommand::findPath(const Path &toFind, const Paths &paths)
	{
		int i = 0;
		for (auto &path : paths)
		{
			if (path == toFind)
				return i;
			i++;
		}

		return -1;
	}

	void SelectTreeItemsCommand::select(const Paths &toSelect)
	{
		QObjectSet toSelectSet;
		for (auto &path : toSelect)
		{
			auto object = Object::getDescendant(path.topAncestor, path.path);
			if (nullptr != object)
				toSelectSet.insert(object);
		}

		tree->select(toSelectSet);
	}

	bool SelectTreeItemsCommand::Path::operator==(const Path &other) const
	{
		return (topAncestor == other.topAncestor &&	path == other.path);
	}

}
